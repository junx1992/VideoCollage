#include "Stdafx.h"
#include "VideoSegments.h"
#include "OpenCVHelper.h"
#include "AffineOffineSubShotDetector.h"
#include "AffineGlobalMotionEstimation.h"
#include <cmath>
#include <map>
#include <cassert>



namespace VideoAnalysisHelper
{
     using namespace VideoAnalysis;
     using namespace VxCore;
	 // Sub shot Affine Motion
	 //*******************************************************************************************************
	 //	Affine motion definition
	 //*******************************************************************************************************
   	 enum AFFINE_Motion
	 {
		AFFINE_MOTION_STATIC = 0,
		AFFINE_MOTION_PAN,
		AFFINE_MOTION_TILT,
		AFFINE_MOTION_ZOOM,
		AFFINE_MOTION_ROT,
		AFFINE_MOTION_OBJECT
	 };

   	//*******************************************************************************************************
	//	Frame motion info structure
	//*******************************************************************************************************
	struct FrameMotionInfo 
	{
		AFFINE_Motion value;
		bool bIsLabeled;

		// Motion Parameters
		float pan;
		float tit;
		float zom;
		float rot;
		float hyp;	

		float a1, a2, a3, a4, a5, a6;

		FrameMotionInfo(void) 
		{
			bIsLabeled = false;
			value = AFFINE_MOTION_STATIC;
			pan = tit = zom = rot = hyp = 
			a1 = a2 = a3 = a4 = a5 = a6 = 0.0;
	   }		
	} ;

    ///typedef
    typedef VxCore::CIntFeature		CAffineMotion;
    typedef std::map<unsigned int,  CAffineMotion, std::less<unsigned int> >	CAffineSubShotMotionSequence;
    typedef std::pair<unsigned int, CAffineMotion> SubshotMotionSeqPair;
                  
    ///the class for offine sub shot detect implemention
    class CAffineOffineSubshotDetectorImp
    {
    public:
		CAffineOffineSubshotDetectorImp(const CVideoSegmentList& ShotList, const IDataSet& GMESeqSet);
		~CAffineOffineSubshotDetectorImp();

        //do the sub shot detect offline, it will return a sub shot list, 
        //!!! the returned list may be empty, if there is any error 
        const CVideoSegmentList& DetectSegmentOffline();
        const CVideoSegmentList& GetSubshots() const;
   private:
		//disable copy and assign construct function
		CAffineOffineSubshotDetectorImp(const CAffineOffineSubshotDetectorImp &rhs);
		CAffineOffineSubshotDetectorImp& operator=(const CAffineOffineSubshotDetectorImp &rhs);

		HRESULT ReadMotionData(void);
		void Smooth1D(float pArray[], unsigned int n, int iSmooth);
		void SubshotDetectByThresholdBased(void);

    private:
        //the information of shot and GME
		const CVideoSegmentList& m_ShotList;
		const IDataSet& m_GMEFeatureDataSet;
        
        //frameMotionInfo array will be used in sub shot detection process
		FrameMotionInfo *m_FrmInfoArray;
        //counting the number of sub shot
		unsigned int m_SubshotIndex;
        //the sub shot motion contianer
		CAffineSubShotMotionSequence m_SubshotMotionSeqSet;
	    //the sub shot contianer
		CVideoSegmentList m_SubshotList;   
     };
}

namespace VideoAnalysis
{
      using namespace VideoAnalysisHelper;

      //the offline sub shot detector need information of shots and GME
      CAffineOffineSubshotDetector::CAffineOffineSubshotDetector(const CVideoSegmentList& ShotList, const IDataSet & GMESeqSet)
      {
           m_pImp = new CAffineOffineSubshotDetectorImp(ShotList, GMESeqSet);
      }
             
      CAffineOffineSubshotDetector::~CAffineOffineSubshotDetector()
      {
           delete  m_pImp;
      }
 
      //do the sub shot detect offline
      const CVideoSegmentList& CAffineOffineSubshotDetector::DetectSegmentOffline()
      {
           assert(m_pImp != NULL);
           return m_pImp->DetectSegmentOffline();
      }

      const CVideoSegmentList& CAffineOffineSubshotDetector::GetSubshots() const
      {
           assert(m_pImp != NULL);
           return m_pImp->GetSubshots();
      }
}

namespace VideoAnalysisHelper
{
	CAffineOffineSubshotDetectorImp::CAffineOffineSubshotDetectorImp(
		const CVideoSegmentList& ShotList, 
		const IDataSet& GMESeqSet)
		: m_ShotList(ShotList), m_GMEFeatureDataSet(GMESeqSet),
		m_FrmInfoArray(NULL), m_SubshotIndex(-1)
	{
		m_SubshotList.Clear();
	}

	CAffineOffineSubshotDetectorImp::~CAffineOffineSubshotDetectorImp()
	{
		SAFE_DELETE_ARRAY(m_FrmInfoArray);
	}

	const CVideoSegmentList& CAffineOffineSubshotDetectorImp::DetectSegmentOffline()
	{
	    //read motion info from file
		if( ReadMotionData() == S_OK )
            //sub shot detect according to threshhold
		    SubshotDetectByThresholdBased();

        return m_SubshotList;
	}

    const CVideoSegmentList& CAffineOffineSubshotDetectorImp::GetSubshots() const
    {
        return m_SubshotList;
    }
	HRESULT CAffineOffineSubshotDetectorImp::ReadMotionData(void)
	{
		unsigned int shotNum = static_cast<unsigned int>(m_ShotList.Size());
        //for some video the shots list from shot detector is empty, so we must check the number of shots
        if( shotNum == 0 )
            return S_FALSE;
		unsigned int lStartNo = m_ShotList[0].BeginFrameId();
		unsigned int lEndNo = m_ShotList[shotNum-1].EndFrameId();
		//lFrmNum = lEndNo - lStartNo + 1;		// Frame Number that shot cover
        // modified by linjuny@microsoft.com, because sometimes lStartNo is not equal to zero
		unsigned int lFrmNum =  lEndNo + 1;	//lEndNo;		

		unsigned int lIdx=0 ;
			
		/************************************************************************/
		/* Fram Index:		1, 2, 3, 4, 5, ... , lFrmNum
		/* Motion Index:	0, 1, 2, 3, 4, ... , lFrmNum-1
		/************************************************************************/
		float *a1, *a2, *a3, *a4, *a5, *a6, *Je;
		a1 = a2 = a3 = a4 = a5 = a6 = Je = NULL;

		SAFE_DELETE_ARRAY(m_FrmInfoArray);
		m_FrmInfoArray = new FrameMotionInfo[lFrmNum];

		a1 = new float[lFrmNum];
		a2 = new float[lFrmNum];
		a3 = new float[lFrmNum];
		a4 = new float[lFrmNum];
		a5 = new float[lFrmNum];
		a6 = new float[lFrmNum];
		Je = new float[lFrmNum];

		if ( m_FrmInfoArray==NULL || a1==NULL || a2==NULL || a3==NULL || 
			a4==NULL || a5==NULL || a6==NULL || Je==NULL )
		{
			SAFE_DELETE_ARRAY(a1);
			SAFE_DELETE_ARRAY(a2);
			SAFE_DELETE_ARRAY(a3);
			SAFE_DELETE_ARRAY(a4);
			SAFE_DELETE_ARRAY(a5);
			SAFE_DELETE_ARRAY(a6);
			SAFE_DELETE_ARRAY(Je);

			return E_OUTOFMEMORY;
		}


		/******************************************************************\
		f-crdeng according to discussion with MeiTao & Linjun Yang:

		We will intersect motion data if step equal to 3. The paras value between
		frame 1 and frame 2 will hold in id 2. Assume we have 13 frame

			The global motion data in feature sequence are 2, 5, 8, 11 
			
			frame id	gme	value
			1,2				2		
			3,4,5			5
			6,7,8			8
			9,10,11,12,13	11
		\******************************************************************/
		lIdx=0;
  		int size = m_GMEFeatureDataSet.GetSampleNum();
		for( int i = 0; i < size; ++i )
		{
             const CAffineGlobalMotion &gmeFeature = m_GMEFeatureDataSet.GetSample(i);
             unsigned int frameId = gmeFeature.Id();

			 for( ; lIdx < frameId; lIdx++ )
			 {
				     if( frameId >= lFrmNum )
					     break;
				     a1[lIdx] = (float)gmeFeature[0];
				     a2[lIdx] = (float)gmeFeature[1];
				     a3[lIdx] = (float)gmeFeature[2];
				     a4[lIdx] = (float)gmeFeature[3];
				     a5[lIdx] = (float)gmeFeature[4];
				     a6[lIdx] = (float)gmeFeature[5];
				     Je[lIdx] =  (float)gmeFeature[6];
			}
		}
		for( ; lIdx<lFrmNum; lIdx++)
		{
			a1[lIdx] = a1[lIdx-1];
			a2[lIdx] = a2[lIdx-1];
			a3[lIdx] = a3[lIdx-1];
			a4[lIdx] = a4[lIdx-1];
			a5[lIdx] = a5[lIdx-1];
			a6[lIdx] = a6[lIdx-1];
			Je[lIdx] = Je[lIdx-1];
		}

		//Smooth motion

		// MEDIAN SMOOTH: 0
		Smooth1D(a1, lFrmNum, 0);
		Smooth1D(a2, lFrmNum, 0);
		Smooth1D(a3, lFrmNum, 0);
		Smooth1D(a4, lFrmNum, 0);
		Smooth1D(a5, lFrmNum, 0);
		Smooth1D(a6, lFrmNum, 0);

		// Gaussian SMOOTH: 1
		Smooth1D(a1, lFrmNum, 1);
		Smooth1D(a2, lFrmNum, 1);
		Smooth1D(a3, lFrmNum, 1);
		Smooth1D(a4, lFrmNum, 1);
		Smooth1D(a5, lFrmNum, 1);
		Smooth1D(a6, lFrmNum, 1);	


		// tranfer affine motion to camera motion, because camera motion is more meaningful
		for(lIdx=0; lIdx<lFrmNum; lIdx++)
		{
			m_FrmInfoArray[lIdx].a1 = a1[lIdx];
			m_FrmInfoArray[lIdx].a2 = a2[lIdx];
			m_FrmInfoArray[lIdx].a3 = a3[lIdx];
			m_FrmInfoArray[lIdx].a4 = a4[lIdx];
			m_FrmInfoArray[lIdx].a5 = a5[lIdx];
			m_FrmInfoArray[lIdx].a6 = a6[lIdx];

			m_FrmInfoArray[lIdx].pan = a1[lIdx];
			m_FrmInfoArray[lIdx].tit = a4[lIdx];
			m_FrmInfoArray[lIdx].zom = (a2[lIdx] + a6[lIdx]) / 2;
			m_FrmInfoArray[lIdx].rot = (a5[lIdx] - a3[lIdx]) / 2;
			m_FrmInfoArray[lIdx].hyp = static_cast<float>(fabs(a2[lIdx] - a6[lIdx]) / 2.0 + fabs(a5[lIdx] + a3[lIdx]) / 2.0);				
		}

		SAFE_DELETE_ARRAY(a1);
		SAFE_DELETE_ARRAY(a2);
		SAFE_DELETE_ARRAY(a3);
		SAFE_DELETE_ARRAY(a4);
		SAFE_DELETE_ARRAY(a5);
		SAFE_DELETE_ARRAY(a6);
		SAFE_DELETE_ARRAY(Je);
		
		return S_OK;
	}



	/************************************************************************\
	Function: smooth 1D array
	Arguments:
		[IN]	pArray:		original float array pointer
		[IN]	lLength:	length of pArray
		[IN]	Smooth:		smooth method = {GAUSSIAN_FILTER, MEDIAN_FILTER}
		[OUT]	strMFileName file:	m file for MATLAB plot
		[OUT]	pArray:		resultent float array pointer
	\************************************************************************/
	void CAffineOffineSubshotDetectorImp::Smooth1D(float pArray[], unsigned int n, int iSmooth)
	{
		const int iMedWinWidth = 11;	//SMOOTH_MOTION_MEDIAN_WINDOW ;
		const int iGusWinWidth = 25;	//SMOOTH_MOTION_GAUSSIAN_WINDOW ;
		
		long lLength = n;
		CvMat* pMat = COpenCVHelper::cvCreateMat(1, lLength, CV_32F);
		long lIdx=0 ;
		int i=0, j=0, k=0 ;
		
		for( lIdx = 0; lIdx < lLength; ++lIdx )	
		       COpenCVHelper::cvSetReal1D(pMat, lIdx, pArray[lIdx]);
		
		if( iSmooth == 1 ) 
		{   //gaussian filter
            COpenCVHelper::cvSmooth(pMat, pMat, CV_GAUSSIAN, iGusWinWidth, 1);
			for(lIdx=0; lIdx<lLength; lIdx++)	
			{
				pArray[lIdx] = static_cast<float>(COpenCVHelper::cvGetReal1D(pMat, lIdx));
			}
		}  else {
            //median filter
			float* sort = new float[iMedWinWidth] ;
			for (i=0; i<iMedWinWidth; i++)
			{
				sort[i] = 0.0f;
			}

			for( i = iMedWinWidth/2; i < lLength-iMedWinWidth/2; ++i )
			{
				// Here in old version is:
				// k<iMedWinWidth, j<=i+iMedWinWidth/2
				//
				// This expression has side effect, so it has been modified as below
				// If it is a comma, the first condition will be ignored
				for (k=0, j=i-iMedWinWidth/2; k<iMedWinWidth && j<=i+iMedWinWidth/2; j++, k++)
				{
					sort[k] = static_cast<float>(COpenCVHelper::cvGetReal1D(pMat, j)); 
				}
			
				// sort array
				for (j=0; j<iMedWinWidth; j++)
				{
					for (k=0; k<iMedWinWidth-j-1; k++)
					{
						if (sort[k]>sort[k+1])	
						{
							float t = sort[k];
							sort[k] = sort[k+1];
							sort[k+1] = t;
						}
					}
				}
					
				// get the median data of sort array
				pArray[i] = sort[iMedWinWidth/2] ;		
			}
			SAFE_DELETE_ARRAY(sort);
		}

		if ( pMat != NULL )
		{
            COpenCVHelper::cvReleaseMat(&pMat) ;
			pMat = NULL;
		}
	}




	void CAffineOffineSubshotDetectorImp::SubshotDetectByThresholdBased(void)
	{
		// Tlin, Threshold for (div, rot, hyp)
		const float THRESH_CAMMOTION_LINEAR = 0.016f; // 0.012 // 0.018, 25 fps; 0.015, 30 fps
		const float THRESH_CAMMOTION_TRANSLATION = 1.15f; // 1.00 // 1.20, 25 fps; 1.00, 30 fps
		const int THRESH_CAMMOTION_TEMPORAL = 50;	// FPS

		long lFrmNum, lStartNo, lEndNo;
		long lStep = 1;

		unsigned int shotNum = static_cast<unsigned int>(m_ShotList.Size());
		lStartNo = m_ShotList[0].BeginFrameId();
		lEndNo = m_ShotList[shotNum-1].EndFrameId();
		lFrmNum = lEndNo - lStartNo + 1;

		long lIdx = 0 ;
		int i=0, j=0 ;
		
		// 1. frame-level operation
		for(lIdx=0; lIdx<lFrmNum; lIdx++)
		{
			if(fabs(m_FrmInfoArray[lIdx].pan)<THRESH_CAMMOTION_TRANSLATION) m_FrmInfoArray[lIdx].pan=0.0f;
			if(fabs(m_FrmInfoArray[lIdx].tit)<THRESH_CAMMOTION_TRANSLATION) m_FrmInfoArray[lIdx].tit=0.0f ;
			if(fabs(m_FrmInfoArray[lIdx].rot)<THRESH_CAMMOTION_LINEAR) m_FrmInfoArray[lIdx].rot=0.0f;
			if(fabs(m_FrmInfoArray[lIdx].hyp)<THRESH_CAMMOTION_LINEAR) m_FrmInfoArray[lIdx].hyp=0.0f;
			
			if((m_FrmInfoArray[lIdx].a2*m_FrmInfoArray[lIdx].a6)<0.0f) m_FrmInfoArray[lIdx].zom=0.0f ;
		
			if(fabs(m_FrmInfoArray[lIdx].zom)<THRESH_CAMMOTION_LINEAR) m_FrmInfoArray[lIdx].zom=0.0f ;		
		}
		
		// 2. segment-level operation
		// sequetial order: zoom->rotate->pan->tilt->object->static
		int iCount=0, iTemporalThresh=int( float(THRESH_CAMMOTION_TEMPORAL)*(0.85) ) ;

		// (a) zom
		for(lIdx=0; lIdx<lFrmNum; lIdx++)
		{
			iCount = 0 ;
			if( !(m_FrmInfoArray[lIdx].bIsLabeled) )
			{
				for(i=lIdx-THRESH_CAMMOTION_TEMPORAL/2; i<lIdx+THRESH_CAMMOTION_TEMPORAL/2; i++)
				{
					if(i<0)	j=-i ;
					else if(i>=lFrmNum) j=(lFrmNum-1)*2-i ;
					else j=i ;

					if(fabs(m_FrmInfoArray[j].zom)>=0.00001f) iCount++ ;
				}
			
				if(iCount>=iTemporalThresh) 
				{
					m_FrmInfoArray[lIdx].bIsLabeled = true;
					m_FrmInfoArray[lIdx].value = AFFINE_MOTION_ZOOM;
				}
			}		
		}

		// (b) rot
		for(lIdx=0; lIdx<lFrmNum; lIdx++)
		{
			iCount = 0 ;
			if( !(m_FrmInfoArray[lIdx].bIsLabeled) )
			{
				for(i=lIdx-THRESH_CAMMOTION_TEMPORAL/2; i<lIdx+THRESH_CAMMOTION_TEMPORAL/2; i++)
				{
					if(i<0)	j=-i ;
					else if(i>=lFrmNum) j=(lFrmNum-1)*2-i ;
					else j=i ;

					if(fabs(m_FrmInfoArray[j].rot)>=0.00001f) iCount++ ;
				}
			
				if(iCount>=iTemporalThresh) 
				{
					m_FrmInfoArray[lIdx].bIsLabeled = true;
					m_FrmInfoArray[lIdx].value = AFFINE_MOTION_ROT;
				}
			}		
		}
		
		// (c) pan
		for(lIdx=0; lIdx<lFrmNum; lIdx++)
		{
			iCount = 0 ;
			if( !(m_FrmInfoArray[lIdx].bIsLabeled) )
			{
				for(i=lIdx-THRESH_CAMMOTION_TEMPORAL/2; i<lIdx+THRESH_CAMMOTION_TEMPORAL/2; i++)
				{
					if(i<0)	j=-i ;
					else if(i>=lFrmNum) j=(lFrmNum-1)*2-i ;
					else j=i ;	

					if(fabs(m_FrmInfoArray[j].pan)>=0.00001f) iCount++ ;
				}
			
				if(iCount>=iTemporalThresh) 
				{
					m_FrmInfoArray[lIdx].bIsLabeled = true;
					m_FrmInfoArray[lIdx].value = AFFINE_MOTION_PAN;
				}
			}		
		}

		// (d) tit
		for(lIdx=0; lIdx<lFrmNum; lIdx++)
		{
			iCount = 0 ;
			if( !(m_FrmInfoArray[lIdx].bIsLabeled) )
			{
				for(i=lIdx-THRESH_CAMMOTION_TEMPORAL/2; i<lIdx+THRESH_CAMMOTION_TEMPORAL/2; i++)
				{
					if(i<0)	j=-i ;
					else if(i>=lFrmNum) j=(lFrmNum-1)*2-i ;
					else j=i ;

					if(fabs(m_FrmInfoArray[j].tit)>=0.00001f) iCount++ ;
				}
			
				if(iCount>=iTemporalThresh) 
				{
					m_FrmInfoArray[lIdx].bIsLabeled = true;
					m_FrmInfoArray[lIdx].value = AFFINE_MOTION_TILT;
				}
			}		
		}

		// (e) OBJ
		for(lIdx=0; lIdx<lFrmNum; lIdx++)
		{
			iCount = 0 ;
			if( !(m_FrmInfoArray[lIdx].bIsLabeled) )
			{
				for(i=lIdx-THRESH_CAMMOTION_TEMPORAL/2; i<lIdx+THRESH_CAMMOTION_TEMPORAL/2; i++)
				{
					if(i<0)	j=-i ;
					else if(i>=lFrmNum) j=(lFrmNum-1)*2-i ;
					else j=i ;

					if(fabs(m_FrmInfoArray[j].hyp)>=0.00001f) iCount++ ;
				}
			
				if(iCount>=iTemporalThresh) 
				{
					m_FrmInfoArray[lIdx].bIsLabeled = true;
					m_FrmInfoArray[lIdx].value = AFFINE_MOTION_OBJECT;
				}
			}		
		}

		// (f) STA
		// remaining is static motion
		
		/************************************************************************\
		As a result, m_pFrameInfo[i].CamMotion is filtered by threshold, and
		m_pFrameInfo[i].Label is also labeled.
		\************************************************************************/

		const int SUBSHOT_MERGE_WINDOW = 50;
		// 3. sub-shot merging, eliminate less than half a second outlier label
		int iMergeWidow = SUBSHOT_MERGE_WINDOW ;			// 25 is able to exclude less than half a second outlier
		int iSTA=0, iPAN=0, iTIT=0, iZOM=0, iROT=0, iOBJ=0 ;
		AFFINE_Motion CurFrmLab = AFFINE_MOTION_STATIC;
		for(lIdx=0; lIdx<lFrmNum; lIdx++)
		{
			iSTA=0, iPAN=0, iTIT=0, iZOM=0, iROT=0, iOBJ=0 ;
			int iMax1=0, iMax2=0, iMax3=0, iMax4=0, iMax=0 ;
			CurFrmLab = AFFINE_MOTION_STATIC;
			for(i=lIdx-iMergeWidow/2; i<=lIdx+iMergeWidow/2; i++)
			{
				if(i<0) 
					j=-i;
				else if(i>=lFrmNum) 
					j=(lFrmNum-1)*2-i;
				else 
					j=i;

				switch( m_FrmInfoArray[j].value )
				{
				case AFFINE_MOTION_STATIC:
					iSTA++;
					break;
				case AFFINE_MOTION_PAN:
					iPAN++;
					break;
				case AFFINE_MOTION_TILT:
					iTIT++;
					break;
				case AFFINE_MOTION_ZOOM:
					iZOM++;
					break;
				case AFFINE_MOTION_ROT:
					iROT++;
					break;
				case AFFINE_MOTION_OBJECT:
					iOBJ++;
					break;
				default:
					break;
				}
			}

			iMax1 = MAX(iSTA, iPAN);
			iMax2 = MAX(iTIT, iZOM);
			iMax3 = MAX(iROT, iOBJ);
			iMax4 = MAX(iMax1, iMax2);
			iMax = MAX(iMax4, iMax3);

			if(iMax==iZOM) CurFrmLab = AFFINE_MOTION_ZOOM ;
			else if(iMax==iROT) CurFrmLab = AFFINE_MOTION_ROT ;
			else if(iMax==iPAN) CurFrmLab = AFFINE_MOTION_PAN ;
			else if(iMax==iTIT) CurFrmLab = AFFINE_MOTION_TILT ;
			else if(iMax==iOBJ) CurFrmLab = AFFINE_MOTION_OBJECT ;
			else CurFrmLab = AFFINE_MOTION_STATIC;
				
			m_FrmInfoArray[lIdx].value = CurFrmLab ;
		}

		const int SUBSHOT_DURATION_THRESH = 15;
		AFFINE_Motion FrmLabel = AFFINE_MOTION_STATIC;

        CSubshot subShot;
		float pan, hyp, rot, tit, zom;
		long lIntraSubShotCount=0, lInterSubShotCount=0;
		
		long lShotNum = static_cast<long>(m_ShotList.Size());
		long lStartLabel=0, lEndLabel=0;
		for(lIdx=0; lIdx<lShotNum; lIdx++)
		{
			lStartNo = m_ShotList[lIdx].BeginFrameId();
			lEndNo = m_ShotList[lIdx].EndFrameId();
			
			lStartLabel = lStartNo ;
			lEndLabel = lStartLabel ;
			FrmLabel = m_FrmInfoArray[lStartNo].value;
			pan = hyp = rot = tit = zom = 0.0f ;
			
			lIntraSubShotCount = 0;
			lInterSubShotCount = 0;
			
			for(i=lStartNo+1; i<=lEndNo; i++)
			{
				if(FrmLabel == m_FrmInfoArray[i-1].value)
				{
					lEndLabel++;
					lIntraSubShotCount++;
					
					pan += m_FrmInfoArray[i-1].pan ;
					hyp += m_FrmInfoArray[i-1].hyp ;
					rot += m_FrmInfoArray[i-1].rot ;
					tit += m_FrmInfoArray[i-1].tit ;
					zom += m_FrmInfoArray[i-1].zom ;
				}
				else	// reach sub-shot boundary in a shot
				{
					if(lIntraSubShotCount>=SUBSHOT_DURATION_THRESH)
					{
						//subShot.SetId( lInterSubShotCount );
						subShot.Id( ++m_SubshotIndex );
						subShot.BeginFrameId( lStartLabel );
						if ( lEndLabel == lFrmNum-1 )
						{ // Add last frame into result
							lEndLabel = lFrmNum;
						}
                        if( lEndLabel == lFrmNum )
						    subShot.EndFrameId( lEndNo );
                        else
						    subShot.EndFrameId( lEndLabel );
						subShot.ShotId( m_ShotList[lIdx].Id() );
						
						//Motion Feature
						CAffineMotion subShotMotion(subShot.Id(), 1);
						subShotMotion[0] = FrmLabel;
                        m_SubshotMotionSeqSet.insert( SubshotMotionSeqPair(subShotMotion.Id(), subShotMotion) );

                        //m_ShotList[lIdx].AddChild(subShot);
                        //add a new sub shot into the sub shot list
						m_SubshotList.Add(subShot);

						lInterSubShotCount++;
						lIntraSubShotCount = 0;
						lStartLabel = lEndLabel+1 ;
						lEndLabel = lStartLabel ;
						FrmLabel = m_FrmInfoArray[i-1].value;

						pan = (lIntraSubShotCount==0 ? 0.0f : pan / lIntraSubShotCount) ;
						hyp = (lIntraSubShotCount==0 ? 0.0f : hyp / lIntraSubShotCount) ;
						rot = (lIntraSubShotCount==0 ? 0.0f : rot / lIntraSubShotCount) ;
						tit = (lIntraSubShotCount==0 ? 0.0f : tit / lIntraSubShotCount) ;
						zom = (lIntraSubShotCount==0 ? 0.0f : zom / lIntraSubShotCount) ;
						
						// push stCamMotion to a data structure ?
						pan = hyp = rot = tit = zom = 0.0f;
					
					}
					else // (lIntraSubShotCount<SUBSHOT_DURATION_THRESH)
					{
						lIntraSubShotCount = 0 ;
						lStartLabel = lEndLabel+1 ;
						lEndLabel = lStartLabel ;
						FrmLabel = m_FrmInfoArray[i-1].value;
						pan = hyp = rot = tit = zom = 0.0f;
					}
				}
			}

			if(lStartLabel!=lEndLabel)
			{
				//if(lIntraSubShotCount>=SUBSHOT_DURATION_THRESH)
				{
					subShot.Id( ++m_SubshotIndex );
					subShot.BeginFrameId( lStartLabel );
					if ( lEndLabel == lFrmNum-1 )
					{ // Add last frame into result
						lEndLabel = lFrmNum;
					}

                    if( lEndLabel == lFrmNum )
					    subShot.EndFrameId( lEndNo );
                    else
					    subShot.EndFrameId( lEndLabel );
                    subShot.ShotId( m_ShotList[lIdx].Id() );

					//Motion Feature
					CAffineMotion subShotMotion(subShot.Id(), 1);
					subShotMotion[0] = FrmLabel;
                    m_SubshotMotionSeqSet.insert( SubshotMotionSeqPair(subShotMotion.Id(), subShotMotion) );
					
					//m_ShotList[lIdx].AddChild(subShot);
                    //add a new sub shot into the sub shot list
					m_SubshotList.Add(subShot);

					pan = (lIntraSubShotCount==0 ? 0.0f : pan / lIntraSubShotCount) ;
					hyp = (lIntraSubShotCount==0 ? 0.0f : hyp / lIntraSubShotCount) ;
					rot = (lIntraSubShotCount==0 ? 0.0f : rot / lIntraSubShotCount) ;
					tit = (lIntraSubShotCount==0 ? 0.0f : tit / lIntraSubShotCount) ;
					zom = (lIntraSubShotCount==0 ? 0.0f : zom / lIntraSubShotCount) ;
					
					// push stCamMotion to a data structure ?
					pan = hyp = rot = tit = zom = 0.0f;
				}
			}
		}
	}
}

