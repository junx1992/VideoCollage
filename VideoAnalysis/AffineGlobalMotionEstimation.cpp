#include "Stdafx.h"
#include "VxFeature.h"
#include "AffineGlobalMotionEstimation.h"
#include "OpenCVHelper.h"
#include <vector>
#include <typeinfo>
#include <opencv\cv.h>

namespace VideoAnalysisHelper
{
    using namespace VideoAnalysis;
    
	//motion estimation structure
	struct MEItem
	{
        //float m_frmDiff;       //the diff between 2 frame before GME
        float m_wFrmDiff;        //the diff between 2 frame after GME
        //in the Affine Model
        // X' = P0 + P2 * X + P3 * Y
        // Y' = P1 + P4 * X + P5 * Y
        //m_paramArray contains the 6 parameter
        float m_paramArray[6];
    };

    //the implemention class of AffineGlobalMotionEstimation
    class CAffineGlobalMotionEstimationImp
    {
    public:
        CAffineGlobalMotionEstimationImp(int step);

        HRESULT DoMotionEstimation(const unsigned char* pImgData, unsigned long imgWidth, 
                                   unsigned long imgHeight, unsigned int frameId);

        ///get the extracted feature sequence.
        const VxCore::IDataSet& GetData() const
        {
            return  m_FeatureSequenceSet;   
        }

    private:
        //does not implement, means do not allow copy and assignement
        CAffineGlobalMotionEstimationImp(const CAffineGlobalMotionEstimationImp &rhs);
        CAffineGlobalMotionEstimationImp& operator=(const CAffineGlobalMotionEstimationImp &rhs);

        void GlobalMotionEstimation(IplImage *pCurFrm, IplImage *pRefFrm, unsigned long iWidth, unsigned long iHeight);


        static void GetAffineGMEResult(unsigned int frmNum, float a1[], float a2[], 
                                       float a3[], float a4[], float a5[], float a6[]);
	
        static float CalcError(IplImage *pRef, IplImage *pCur, float *pPara);
        static IplImage* GMCompensate(IplImage *pImg, const float *pPara);
        static void AffineGME1(unsigned char *ref, unsigned char *curr, int width, int height, float *Para);
        static void ThreeTapFilter(unsigned char *Low, unsigned char *Hight, int width, int height);
        static double FiltOutliers(int *errhist, int count);
        static int DeltaMP(double *dA, int n, double *db, double *dm);
        static int ModifiedThreeStepSearch(unsigned char *ref_work, unsigned char *curr_work, int width, 
                                           int height, int *best_locationx, int *best_locationy);

    private:
        CAffineGlobalMotionSequence  m_FeatureSequenceSet;            //the feature contianer
        int m_iStep ;						                          //step for use
        unsigned long m_preFrmId;	                                  //the pre-frame id
        std::vector<MEItem> m_MEArray;                                //the global motion item for current frame
        IplImage *m_pCurImg;                                          //current image
        IplImage *m_pPreImg;                                          //pre image
	};
}

namespace VideoAnalysis
{
    //get the start address of a raw image according to its stride
    BYTE* GetOrigin(IImage* img)
	{
		 if( img->Stride() < 0 )
		     return (BYTE*)img->RowPtr(img->Height()-1);
		 else
		     return (BYTE*)img->RowPtr(0);
	}

	CAffineGlobalMotionEstimation::CAffineGlobalMotionEstimation(int step) 
	{
         if( step <= 0 )
             step = 1;
         
         m_pImp = new VideoAnalysisHelper::CAffineGlobalMotionEstimationImp(step);
	}

	CAffineGlobalMotionEstimation::~CAffineGlobalMotionEstimation(void)
	{
		 delete m_pImp;
	}

    HRESULT CAffineGlobalMotionEstimation::OnNewSegment(IVideoSegment& segment)
    {
         HRESULT hr = S_OK;
         try{
               //it must be a frame, others will be error
			   CFrame & frame = dynamic_cast<CFrame&>(segment);
               //do the motion estimation
			   hr = DoMotionEstimation(frame);
	     }catch( std::bad_cast &){
		       return E_FAIL;
		 }

		 return hr;
	}

	HRESULT CAffineGlobalMotionEstimation::DoMotionEstimation(CFrame & frame)
	{
		 //golbal motion estimation
		 unsigned int width = frame.GetImage()->Width();
		 unsigned int height = frame.GetImage()->Height();
		 const BYTE* pbData = GetOrigin(frame.GetImage());
		 unsigned int frameId = frame.Id();
            
         assert(m_pImp != NULL);
		 return m_pImp->DoMotionEstimation(pbData, width, height, frameId);
	}

	 ///get the extracted feature sequence.
	 const VxCore::IDataSet& CAffineGlobalMotionEstimation::GetData() const
     {
         assert( m_pImp != NULL );
		 return m_pImp->GetData();
     }
     
     HRESULT CAffineGlobalMotionEstimation::EndOfStream(){ return S_OK; }
}

namespace VideoAnalysisHelper
{
	CAffineGlobalMotionEstimationImp::CAffineGlobalMotionEstimationImp(int step) 
		: m_iStep(step), m_preFrmId(0), m_pCurImg(NULL), m_pPreImg(NULL)
	{
	}

	HRESULT CAffineGlobalMotionEstimationImp::DoMotionEstimation(const unsigned char* pImgData, 
		unsigned long imgWidth,	unsigned long imgHeight, unsigned int frameId)
	{
        //when m_pCurImg is empty, it is the first frame
		if( m_pCurImg == NULL )
		{
			//first frame
			m_pCurImg = COpenCVHelper::cvCreateImage(cvSize(imgWidth, imgHeight), IPL_DEPTH_8U, 3) ;
			COpenCVHelper::cvSetZero(m_pCurImg) ;
            //the picture is windows bitmap style
			m_pCurImg->origin = 1 ;
			memcpy(m_pCurImg->imageData, pImgData, 3*imgWidth*imgHeight);

			m_pPreImg = COpenCVHelper::cvCreateImage(cvSize(imgWidth, imgHeight), IPL_DEPTH_8U, 3) ;
			COpenCVHelper::cvSetZero(m_pPreImg) ;
			m_pPreImg->origin = 1 ;

			m_preFrmId = frameId;
			return S_OK;
		}

        //we just analyze the certain frame according the value of m_iStep
		if( m_iStep+m_preFrmId != frameId )
			return S_OK;

        //recode the frameid for future use
		m_preFrmId = frameId;

		//save old image
		memcpy(m_pPreImg->imageData, m_pCurImg->imageData, 3*imgWidth*imgHeight);

		//copy new image
		memcpy(m_pCurImg->imageData, pImgData, 3*imgWidth*imgHeight);

		//mv: from prev to curr
		GlobalMotionEstimation(m_pPreImg, m_pCurImg, imgWidth, imgHeight);

		MEItem& item = m_MEArray[m_MEArray.size()-1];

		//save Data into feature sequence
		CAffineGlobalMotion gmeFeature(frameId, 7);	//save global motion value on back frame:

        //if a gme value between frame 1 and 2, we think it is the 2's gme value
		gmeFeature[0] = item.m_paramArray[0];
		gmeFeature[1] = item.m_paramArray[2]-1.0f;
		gmeFeature[2] = item.m_paramArray[3];
		gmeFeature[3] = item.m_paramArray[1];
		gmeFeature[4] = item.m_paramArray[4];
		gmeFeature[5]= item.m_paramArray[5]-1.0f;
		gmeFeature[6] = item.m_wFrmDiff;

        //save the GME feature into the container
        m_FeatureSequenceSet.AddSample(gmeFeature);
		return S_OK;
	}

	/*******************************************************************************************************\
	GlobalMotionEstimation

	Function Description:
	Estimate the global motion parameters that map the currunt frame to the coordinate of reference frame.

	error = curr - (ref + mv)

	Arguments
	[IN]		pCurFrm:	the previous frame rgb image;
	[IN]		pRefFrm:	the current frame rgb image;

	Return Value:
	NONE
	\*********************************************************************************************************/
	void CAffineGlobalMotionEstimationImp::GlobalMotionEstimation(IplImage *pCurFrm, 
                                                                  IplImage *pRefFrm,
											                      unsigned long iWidth, 
                                                                  unsigned long iHeight)
	{
        //get cur and ref imgine's grayimg
		IplImage* pCurGrayImg = COpenCVHelper::cvCreateImage(cvSize(iWidth, iHeight), IPL_DEPTH_8U, 1) ;  
		COpenCVHelper::cvSetZero(pCurGrayImg); 
        //bottom-left structure, windows bitmaps style
		pCurGrayImg->origin=1 ;
		COpenCVHelper::cvCvtColor(pCurFrm, pCurGrayImg, CV_BGR2GRAY) ;	
		COpenCVHelper::cvSmooth(pCurGrayImg, pCurGrayImg, CV_GAUSSIAN, 3, 3);

		IplImage* pRefGrayImg = COpenCVHelper::cvCreateImage(cvSize(iWidth, iHeight), IPL_DEPTH_8U, 1) ;  
		COpenCVHelper::cvSetZero(pRefGrayImg); 
		pRefGrayImg->origin=1 ;	         
		COpenCVHelper::cvCvtColor(pRefFrm, pRefGrayImg, CV_BGR2GRAY) ;	
		COpenCVHelper::cvSmooth(pRefGrayImg, pRefGrayImg, CV_GAUSSIAN, 3, 3) ;
	
		unsigned char* cur = NULL;
		unsigned char* ref = NULL;
		
		cur = new unsigned char[iWidth*iHeight];
		ref = new unsigned char[iWidth*iHeight];
		
		assert(cur!=NULL && ref!=NULL);
	
		unsigned char* ppcur = (unsigned char*)(pCurGrayImg->imageData);
		unsigned char* ppref = (unsigned char*)(pRefGrayImg->imageData);
		unsigned char* pcur = cur;
		unsigned char* pref = ref;
	
		for(unsigned int row=0; 
			row<iHeight; 
			row++, ppcur+=pCurGrayImg->widthStep, ppref+=pCurGrayImg->widthStep, pcur+=iWidth, pref+=iWidth)
		{
			CopyMemory(pcur, ppcur, iWidth);
			CopyMemory(pref, ppref, iWidth);
		}

        MEItem item = {0};
		
		AffineGME1(ref, cur, iWidth, iHeight, item.m_paramArray);

        //calc the diff between the ref and cur grayimg( before and after GMC )
		//item.m_frmDiff = CalcError(pRefGrayImg, pCurGrayImg, NULL);
        CalcError(pRefGrayImg, pCurGrayImg, NULL);
		item.m_wFrmDiff = CalcError(pRefGrayImg, pCurGrayImg, item.m_paramArray);

		m_MEArray.push_back(item);

		SAFE_DELETE_ARRAY(cur);
		SAFE_DELETE_ARRAY(ref);
		
        //release the image
		if ( pCurGrayImg != NULL )
		{
			COpenCVHelper::cvReleaseImage(&pCurGrayImg);
			pCurGrayImg = NULL;
		}

		if ( pRefGrayImg != NULL )
		{
			COpenCVHelper::cvReleaseImage(&pRefGrayImg);
			pRefGrayImg = NULL;
		}
	}

	float CAffineGlobalMotionEstimationImp::CalcError(IplImage *pRef, IplImage *pCur, float *pPara)
	{
		float err=0;

		IplImage *pCom=NULL;
		int w, h;
		int i, j;
		unsigned char v1, v2;
		long count=0;
		
        //do GMC according to the parameter- pPara
		if( pPara==NULL )
			pCom = pCur;
		else {
	        pCom = GMCompensate(pCur, pPara);
			if( pCom==NULL ) 
				pCom = pCur;
		}
		
        //calc the diff
		w =	pCom->width;
		h = pCom->height;
		for( i = 0; i < w; ++i )
		{
			for( j = 0; j < h; ++j )
			{
				v2 = *COpenCVHelper::cvPtr2D(pCom, j, i, NULL);		
				v1 = *COpenCVHelper::cvPtr2D(pRef, j, i, NULL);

				err += fabs(float(v1-v2));
				count++;
			}
		}
		
		err /= count;

		if( pPara != NULL ) 
		{
			if ( pCom != NULL )
			{
				COpenCVHelper::cvReleaseImage(&pCom);
				pCom = NULL;
			}
		}

		return err;
	}

    //do the GMC
	IplImage* CAffineGlobalMotionEstimationImp::GMCompensate(IplImage *pImg, const float *pPara)
	{
		if(pImg->nChannels != 1)
		{
			wprintf(L"\nerror: in GMCompensate, src img is not gray image!\n");
			exit(0) ;
		}

		IplImage* pComImg=COpenCVHelper::cvCreateImage(COpenCVHelper::cvGetSize(pImg), IPL_DEPTH_8U, 1);	
		COpenCVHelper::cvSetZero(pComImg) ;
		int flags=CV_INTER_LINEAR + CV_WARP_FILL_OUTLIERS ;
		CvScalar fillval=cvScalarAll(0) ;
		CvMat* pMat=NULL ;
	
		/************************************************************************/
		/* Affine
		/* |X'| = | P[2], P[3], P[0] | * |X|
		/* |Y'| = | P[4], P[5], P[1] | * |Y|
		/*                               |1|
		/************************************************************************/
		
		pMat = COpenCVHelper::cvCreateMat(2, 3, CV_32FC1) ;
		
		cvmSet(pMat, 0, 0, pPara[2]) ;
		cvmSet(pMat, 0, 1, pPara[3]) ;
		cvmSet(pMat, 0, 2, pPara[0]) ;
		cvmSet(pMat, 1, 0, pPara[4]) ;
		cvmSet(pMat, 1, 1, pPara[5]) ;
		cvmSet(pMat, 1, 2, pPara[1]) ;
		
		COpenCVHelper::cvWarpAffine(pImg, pComImg, pMat, flags, fillval) ;
		

		if ( pMat != NULL )
		{
			COpenCVHelper::cvReleaseMat(&pMat);
			pMat = NULL;
		}
		return pComImg ;
	}

	/***********************************************************CommentBegin******\
	*
	* --    ThreeTapFilter
	*
	* Author : Yoshinori Suzuki : Hitachi, Ltd.
	*
	* Purpose :   3-tap filter with coefficients [1/4 1/2 1/4]，产生金字塔图像\
	*
	* Arguments in :
					Hight--输入图像
					width -- 输入图像的宽度
					height -- 输入图像的高度度
	*
	* Arguments in / out :
					Low--输出图像，宽、高分别为原图像的1/2
	*
	\***********************************************************CommentEnd********/
	void CAffineGlobalMotionEstimationImp::ThreeTapFilter(unsigned char *Low, unsigned char *Hight, int width, int height)
	{
		int i, j, hwidth = width/2, wwidth = width*2, width2 = width+2;
		unsigned char *lt, *ct, *rt, *lc, *cc, *rc,	*lb, *cb, *rb, *p;
		
		p=Low; lt=Hight; ct=Hight; rt=Hight+1; 
		lc=Hight; cc=Hight; rc=Hight+1; 
		lb=lc+width; cb=cc+width; rb=rc+width;
		*p = (*lt + (*ct<<1) + *rt + (*lc<<1) + (*cc<<2) + (*rc<<1) + *lb + (*cb<<1) + *rb + 8) >> 4;
		
		p=Low+1; lt=Hight+1; ct=Hight+2; rt=Hight+3; 
		lc=Hight+1; cc=Hight+2; rc=Hight+3; 
		lb=lc+width; cb=cc+width; rb=rc+width;

		for(i=1; i<width/2; i++, p++, lt+=2, ct+=2, rt+=2, lc+=2, cc+=2, rc+=2, lb+=2, cb+=2, rb+=2)
		{
			*p = (*lt + (*ct<<1) + *rt + (*lc<<1) + (*cc<<2) + (*rc<<1) + *lb + (*cb<<1) + *rb + 8) >> 4;
		}
		
		p=Low+hwidth; lt=Hight+width; ct=Hight+width; rt=Hight+width+1; 
		lc=lt+width; cc=ct+width; rc=rt+width; 
		lb=lc+width; cb=cc+width; rb=rc+width;
		
		for(i=1; i<height/2; i++, p+=hwidth, lt+=wwidth, ct+=wwidth, rt+=wwidth, lc+=wwidth, cc+=wwidth, rc+=wwidth, lb+=wwidth, cb+=wwidth, rb+=wwidth)
		{
			*p = (*lt + (*ct<<1) + *rt + (*lc<<1) + (*cc<<2) + (*rc<<1) + *lb + (*cb<<1) + *rb + 8) >> 4;
		}
		
		p=Low+hwidth+1; lt=Hight+1; ct=Hight+2; rt=Hight+3; 
		lc=lt+width; cc=ct+width; rc=rt+width; 
		lb=lc+width; cb=cc+width; rb=rc+width;
		
		for(j=1; j<height/2; j++, p++, lt+=width2, ct+=width2, rt+=width2, lc+=width2, cc+=width2, rc+=width2, lb+=width2, cb+=width2, rb+=width2)
		{
			for(i=1; i<hwidth; i++, p++, lt+=2, ct+=2, rt+=2, lc+=2, cc+=2, rc+=2, lb+=2, cb+=2, rb+=2)
			{
				*p = (*lt + (*ct<<1) + *rt + (*lc<<1) + (*cc<<2) + (*rc<<1) + *lb + (*cb<<1) + *rb + 8) >> 4;
			}
		}
	}

	void CAffineGlobalMotionEstimationImp::AffineGME1(unsigned char *ref, unsigned char *curr, int width, int height, float *Para)
	{
		unsigned char *ref_P[3], *curr_P[3];
		unsigned char *ref_work = NULL, *curr_work = NULL;
		int i, j, best_locationx, best_locationy;
		int curr_pel, ref_pel;
		int ref_pel1, ref_pel2, ref_pel3;
		double dm[6], db[6], dA[36], M[6];
		double dE2 = 0;
		int level = 2, ite = 0;
		int x, y, x1, y1;
		double dx1, dy1, dx, dy;
		double dt, du, dk, d1mk, dl, d1ml,dI1, de, dI1dx, dI1dy;
		double ddedm[6];
		double I1x1y1[4];
		int check = 1;
		int stop=0;
		double err, err_threshold;
		int error_histgram[256];

		for( i = 0; i < 3; ++i )
			  ref_P[i] = curr_P[i] = NULL;

		ref_P[0] = ref;
		curr_P[0] = curr;
		ref_P[1] = new unsigned char[width*height/4];
		curr_P[1] = new unsigned char[width*height/4];
		ref_P[2] = new unsigned char[width*height/16];
		curr_P[2] = new unsigned char[width*height/16];

		assert ( ref_P[1]!=NULL && curr_P[1]!=NULL && ref_P[2]!=NULL && curr_P[2]!=NULL );

        //get the 1/4 and 1/16 resolution picture
		ThreeTapFilter(ref_P[1], ref_P[0], width, height);
		ThreeTapFilter(curr_P[1], curr_P[0], width, height);
		ThreeTapFilter(ref_P[2], ref_P[1], width/2, height/2);
		ThreeTapFilter(curr_P[2], curr_P[1], width/2, height/2);

		for(i = 0; i < 6; i++) dm[i] = 0;
		for(i = 0; i < 6; i++) M[i] = 0;

		ref_work = ref_P[2];
		curr_work = curr_P[2];
		width = width/4;
		height= height/4;
		best_locationx = 0;
		best_locationy = 0;

		double vp=0;
		for( i = 0; i < 6; ++i ) 
              vp += fabs(Para[i]);

		if(vp==0)
		{
			M[0]=1.0;
			M[4]=1.0;
			ModifiedThreeStepSearch(ref_work, curr_work, width, height, &best_locationx, &best_locationy);
			M[2] = (double)best_locationx;
			M[5] = (double)best_locationy;
		} else {
			M[0] = Para[2];
			M[1] = Para[3];
			M[2] = Para[0]/4;
			M[3] = Para[4];
			M[4] = Para[5];
			M[5] = Para[1]/4;
		}

		for( level = 2; level >= 0; --level ){
	        
			ref_work = ref_P[level];
			curr_work = curr_P[level];
			//初始化排除外点的界限
			err_threshold = 255;

			for(ite = 0; ite < 32; ite++)
			{
	            dE2 = 0.;
				curr_pel = 0;
				stop = 0;
				for(j = 0; j < 36; j++) dA[j] = 0;
				for(i = 0; i < 6; i++) db[i] = 0;
				//初始化残留误差数组
				memset(error_histgram, 0, 256*sizeof(int));

				for(y=0; y<height; y++)
				{
	                dy = (double)(y); 
	                for(x=0; x<width; x++, curr_pel++)
					{
	                    dx = (double)(x); 
	                    
						dt = M[0] * dx + M[1] * dy + M[2];
	                    du = M[3] * dx + M[4] * dy + M[5];
						dx1 = dt;
						dy1 = du;
						if(dx1<0 || dx1>width || dy1<0 || dy1>height) continue;
						x1 = (int) dx1;
						y1 = (int) dy1;
						if(x1>=0 && (x1+1)<width && y1>=0 && (y1+1)<height)
						{
							
	                        ref_pel = x1 + width * y1;
							ref_pel1 = x1+1 + width * y1;
							ref_pel2 = x1 + width * (y1+1);
							ref_pel3 = x1+1 + width * (y1+1);
							dk = dx1 - x1;
							d1mk = 1. - dk;
							dl = dy1 - y1;
							d1ml = 1. -dl;
							I1x1y1[0] = ref_work[ref_pel];
							I1x1y1[1] = ref_work[ref_pel1];
							I1x1y1[2] = ref_work[ref_pel2];
							I1x1y1[3] = ref_work[ref_pel3];
							dI1 = d1mk*d1ml*I1x1y1[0] + dk*d1ml*I1x1y1[1] +
							d1mk*dl*I1x1y1[2] + dk*dl*I1x1y1[3];
							de = dI1 - curr_work[curr_pel];		//dI/dt
							//判断该点是否为外点,若是则不考虑（即抛弃此点）,
							//否则加入到误差数组中用以构造残差直方图，并加入到梯度下降的计算中
							err = fabs(de);
							if(err > err_threshold) continue;
							error_histgram[(int)(fabs(de))]++;
							stop++;

	                        dI1dx = (I1x1y1[1]-I1x1y1[0])*d1ml + (I1x1y1[3]-I1x1y1[2])*dl;
							dI1dy = (I1x1y1[2]-I1x1y1[0])*d1mk + (I1x1y1[3]-I1x1y1[1])*dk;
							ddedm[0] = dx * dI1dx;		//	dI/dM[0]
							ddedm[1] = dy * dI1dx;		//	dI/dM[1]
							ddedm[2] = dI1dx;			//	dI/dM[2]，即dI/dx
							ddedm[3] = dx * dI1dy;		//	dI/dM[3]
							ddedm[4] = dy * dI1dy;		//	dI/dM[4]
							ddedm[5] = dI1dy;			//	dI/dM[5]，即dI/dy
							db[0] += -de*ddedm[0];		//	dI/dt * dI/dM[0]
							db[1] += -de*ddedm[1];		//	dI/dt * dI/dM[1]
							db[2] += -de*ddedm[2];		//	dI/dt * dI/dM[2]
							db[3] += -de*ddedm[3];		//	dI/dt * dI/dM[3]
							db[4] += -de*ddedm[4];		//	dI/dt * dI/dM[4]
							db[5] += -de*ddedm[5];		//	dI/dt * dI/dM[5]
							for(j=0; j<6; j++)
	                            for(i=0; i<=j; i++)
									dA[j*6+i] += ddedm[j] * ddedm[i];	// dI/dM[i] * dI/dM[j]
							dE2 += de*de;

						}//*x1, y1
					}//* x
				}//* y 
			
				if(stop<3) break;
				err_threshold = FiltOutliers(error_histgram, stop);
				if(err_threshold==0) break;
				
				//梯度下降求解
				for(j=0; j<6; j++)
	                for(i=j+1; i<6; i++)
                        //使dA矩阵沿着对角线对称
	                    dA[j*6+i] = dA[i*6+j];
	            check = DeltaMP(dA, 6, db, dm);
				if(check)
				{
	                for(i=0; i<6; i++) M[i] += dm[i];
		
					if(fabs(dm[2]) < 0.001 && fabs(dm[5]) < 0.001 && fabs(dm[0]) < 0.00001 &&
						fabs(dm[1]) < 0.00001 && fabs(dm[3]) < 0.00001 && 
						fabs(dm[4]) < 0.00001 ) 
						break;
				}
				else
	                break;
			} /* iteration */

			width *= 2;
			height *= 2;
			if(level)
			{
				M[2] *= 2;
				M[5] *= 2;
			}

		} /* level */

		Para[2] = (float)M[0];
		Para[3] = (float)M[1];
		Para[0] = (float)M[2];
		Para[4] = (float)M[3];
		Para[5] = (float)M[4];
		Para[1] = (float)M[5];

		for( i = 1; i < 3; ++i )
		{
			SAFE_DELETE_ARRAY(ref_P[i]);
			SAFE_DELETE_ARRAY(curr_P[i]);
		}
	}


	/***********************************************************CommentBegin******\
	 *
	 * --    ModifiedThreeStepSearch
	 *
	 * Author : Yoshinori Suzuki : Hitachi, Ltd.
	 *
	 * Created : 05-26-98
	 *
	 * Purpose :   coarse estimate for translation component of displacement 
	 *				改进的散步搜索算法，主要是对两帧间的全局位移进行粗略估计
	 * Arguments in :
					ref_work -- 参考帧
					curr_work -- 当前帧
	 *				ref_alpha_work、curr_alpha_work -- 参考帧和当前帧的alpha分量，
														对于要处理的像素，对应位置上应设为255
					width、height、width、height--参考帧和当前帧的宽、高

					offset_x、offset_y -- 偏移量
	 * Arguments in / out :
	 *				best_locationx、best_locationy -- 估计出的全局位移分量
	 * Return values :
	 *				the estimation error
	\***********************************************************CommentEnd********/
	int CAffineGlobalMotionEstimationImp::ModifiedThreeStepSearch(unsigned char *ref_work, unsigned char *curr_work, int width, int height, int *best_locationx, int *best_locationy)
	{
		int i,j, locationx, locationy, no_of_pel=0;
		int curr_pel, ref_pel, ref_x, ref_y;
		int range_locationx=0, range_locationy=0;
		int thrs=255,total_no;
		double dE1 = 0, min_error=300;
		int hist[256],round=0, last_thrs;
		int error;

		do{
			last_thrs=thrs;

			range_locationx=*best_locationx;
			range_locationy=*best_locationy;
			for (locationy = range_locationy-8; locationy <= range_locationy+8; locationy ++)
			{
	            for (locationx = range_locationx-8; locationx <= range_locationx+8; locationx ++)
				{
	                dE1 = 0.0;
					no_of_pel = 0;
					total_no = 0;
					curr_pel = 0;
					for (j = 0; j< height ; j++)
					{
	                    for (i = 0; i< width ; i++, curr_pel++)
						{
	                        ref_x = locationx + i;
	                        ref_y = locationy + j;
	                        ref_pel = ref_y * width + ref_x;

	                        if ((ref_x >=  0) && (ref_x < width) && (ref_y >=  0) && (ref_y < height))
							{
	                            if((error=abs(curr_work[curr_pel] - ref_work[ref_pel]))<=last_thrs)
								{
	                                dE1 += error;
	                                no_of_pel ++;
								}
								total_no++;
							}//* limit of ref area
						}//* i
					}  //* j

					if (no_of_pel > 0)
						dE1 = dE1 / no_of_pel;
					else
						dE1 = min_error+1;
					if (((dE1 < min_error) && (no_of_pel*2>total_no)) || 
						((dE1 == min_error) && (no_of_pel*2>total_no) && (abs(*best_locationx) + 
																		abs(*best_locationy)) > (abs(locationx) + abs(locationy))))
					{
						min_error = dE1;
						*best_locationx = locationx;
						*best_locationy = locationy;
					}
				}//* locationx 
			}//* locationy

			for (j = 0; j< 256 ; j++) hist[j]=0;
			no_of_pel=0;
			curr_pel = 0;
			for (j = 0; j< height ; j++) 
			{
	            for (i = 0; i< width ; i++, curr_pel++) 
				{
	                ref_x = *best_locationx + i;
	                ref_y = *best_locationy + j;
					ref_pel = ref_y * width + ref_x;

					if ((ref_x >=  0) && (ref_x < width) && (ref_y >=  0) && (ref_y < height))
					{
	                    hist[abs(curr_work[curr_pel] - ref_work[ref_pel])]++;
						no_of_pel ++;
					} //* limit of ref area 
				} //* i
			} //* j

			no_of_pel=(int)(no_of_pel*0.8);
			j=0;
			for (i=0;i<256;i++)
			{
				j+=hist[i];
				if(j>no_of_pel) {thrs=i;break;}
			}	

			round++;
		}while(thrs!=last_thrs && round<5);

		return(thrs);
	}

	/***********************************************************CommentBegin******\
	 *
	 * --    DeltaMP
	 *
	 * Author : Yoshinori Suzuki : Hitachi, Ltd.
	 *
	 * Created : 07-06-98
	 *
	 * Purpose :   generating the inverse matrix 
	 *
	\***********************************************************CommentEnd********/
	int CAffineGlobalMotionEstimationImp::DeltaMP(double *dA, int n, double *db, double *dm)
	{
		int i, j, i2, n2=n*2, k, i_pivot;
		double *dAi = new double[n * n * 2];
		assert(dAi != NULL);
		double pivot, tmp;
		
        //find the max value in dA
		pivot = *dA;
		for(j = 0; j < n; j++)
		{
			for(i = 0; i < n; i++) /*if(fabs(*(dA + j * n2 + i)) > fabs(pivot))*/
			{
				if(fabs(*(dA + j * n + i)) > fabs(pivot))	pivot = *(dA + j * n + i);
			}
		}
		
		if(fabs(pivot) < 0.000000001) 
		{
			SAFE_DELETE_ARRAY(dAi);
			return(0);
		}
		pivot = 1.0 / pivot; 
		
		for(j = 0; j < n; j++)
		{
			for(i = 0, i2 = n; i < n; i++, i2++)
			{
				*(dAi + j * n2 + i) = (*(dA + j * n + i))*pivot;
				if(i == j)	*(dAi + j * n2 + i2) = pivot;
				else			*(dAi + j * n2 + i2) = 0.0;
			}
		}
		
		for(i = 0; i < n; i++) 
		{
			i_pivot = i;
			pivot = *(dAi + i * n2 + i);
			for(j = i; j < n; j++)
			{
				if(fabs(*(dAi + j * n2 + i)) > fabs(pivot))
				{
					i_pivot = j;
					pivot = *(dAi + j * n2 + i);
				}
			}
			
			if(fabs(pivot) < 0.000000001) 
			{
				SAFE_DELETE_ARRAY(dAi);
				return(0);
			}
			
			if(i_pivot!=i)
			{
				for(k = 0; k < n2; k++) 
				{
					tmp = *(dAi + i * n2 + k);
					*(dAi + i * n2 + k) = *(dAi + i_pivot * n2 + k);
					*(dAi + i_pivot * n2 + k) = tmp;
				}
			}
			
			for(j = 0; j < n; j++) 
			{
				if(j != i)
				{
					pivot = *(dAi + j * n2 + i) / *(dAi + i * n2 + i);
					for(k = i+1; k < n2; k++)
					{
						*(dAi + j * n2 + k) -= pivot*(*(dAi + i * n2 + k));
					}
				}
			}
			
		}
		
		for(j = 0; j < n; j++)
		{
			for(i = 0, i2 = n; i < n; i++, i2++)
			{
				*(dAi + j * n2 + i2) /= *(dAi + j * n2 + j);
			}
		}
		
		for(i = 0; i < n; i++) *(dm + i) = 0.0;
		
		for(j = 0; j < n; j++)
		{
			for(i = 0, i2 = n; i < n; i++, i2++)
			{
				*(dm + j) += (*(dAi + j * n2 +i2))*(*(db + i));
			}
		}
		
		SAFE_DELETE_ARRAY(dAi);
		return(1);
	}

	/*******************************************************************************************************\
	FiltOutliers

	Function Description:
		Use the Fisher linear criterion to find the threshold adaptively according to the estimated residual errors

	Arguments
		[IN]		errhist:		histogram of the stimated residual errors;
		[IN]		count:			the total size of the histogram

	Return Value:
		the adaptive threshold
	\*********************************************************************************************************/
	double CAffineGlobalMotionEstimationImp::FiltOutliers(int *errhist, int count)
	{
		int N; //特征点对的总数
		//利用fisher准则将误差分成两类，分别代表两类点：outliers  inliers
		int maxErr=0;
		int i,j,s1,e1,sum=0;
		double w0,w1,u0,u1,*p,B,B1;  //分别为第0类和第1类的概率、均值、类间方差、每钟灰的概率

		N = count;
		maxErr=256;
		p=new double[maxErr];
		assert(p!=NULL);
		for(i=0;i<maxErr;i++) p[i]=double(errhist[i])/double(N);
		s1=0;
		e1=maxErr-1;

		//从s1,e1 开始聚类
		double max=0;
		int value=0;
		for(i=s1;i<=e1;i++)
		{
			w0=u0=0;
			for(j=0;j<=i;j++)
			{
				w0+=p[j];
				u0+=j*p[j];
			}
			if(w0==0) continue;
			u0=u0/w0;
			w1=1-w0;
			if(w1==0) break;
			u1=0;
			for(j=i+1;j<maxErr;j++) u1+=j*p[j];
			u1=u1/w1;

	        B1=0;
			for(j=i+1;j<maxErr;j++) B1+=(j-u1)*(j-u1)*p[j];
			B1=B1/w1;
			B1=sqrt(B1);
			B=w0*B1*(u1-u0)*(u1-u0);
			if(B>max)
			{
				max=B;
				value=i;
			}
		}
		SAFE_DELETE_ARRAY(p);

		return double(value);
	}
}