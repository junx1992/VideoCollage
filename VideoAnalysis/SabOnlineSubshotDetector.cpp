#include "Stdafx.h"
#include "VideoSegments.h"
#include "SimpleKeyframeExtractor.h"
#include "SabOnlineSubshotDetector.h"
#include <typeinfo>
#include <cassert>


namespace VideoAnalysisHelper
{
    using namespace  VideoAnalysis;
	//sub shot detector implemetation class
	class  CSabOnlineSubshotDetectorImp: public ISubshotDetector
	{
	public:
		/**
		if the parameter is NULL, then a default keyframe extractor is used.
		*/
		CSabOnlineSubshotDetectorImp(CVideoSegmentList ShotList,
                                     IOfflineSubshotDetector* pOfflineSgtDetector,
                                     IKeyframeExtractor* pKeyframeExtractor = NULL,
                                     bool releaseImg = true);
        ~CSabOnlineSubshotDetectorImp();
    public:
        //implementation of IVideoParseReceiver
        virtual HRESULT OnNewSegment(IVideoSegment& segment);
	    virtual HRESULT EndOfStream();
    public:
        //implementation of IVideoParseController
        virtual HRESULT AddReceiver(IVideoParseReceiver* const);
        virtual HRESULT RemoveReceiver(const IVideoParseReceiver* const);
    public:
  		//get the information of all the detected subshots.
		//NOTE: the keyframe in the subshot is not available.
		virtual const CVideoSegmentList& GetSubshots() const;

        //get the shot list filled with sub shot information
        //NOTE: the key frame is still not available here
        const  CVideoSegmentList& GetCompleteShots() const;

    private:
        //do sub shot detect
        HRESULT DetectNewSubshot(CFrame & frame);
        //!!!not implemention, means don't allow copy and assignment
        CSabOnlineSubshotDetectorImp(const CSabOnlineSubshotDetectorImp&);
        CSabOnlineSubshotDetectorImp& operator=(const CSabOnlineSubshotDetectorImp&);

  private:
        IOfflineSubshotDetector* m_OfflineSgtDetector;     //the sub shot offine detector
		CVideoSegmentList  m_SubshotList;			       //sub shot list
        CVideoSegmentList  m_ShotList;                     //shot list
  		CVideoParseReceiverList m_Receivers;               //hold all the receiverd registered to shot detector
		IKeyframeExtractor* m_KeyframeExtractor;           //the key frame extractor

		unsigned int m_FrameId;                            //the frame id holder
        int m_SubshotIdx;                                  //the sub shot index
 
        bool m_bGetKeyFrameExtractorExternal;              //mark, whether the m_KeyframeExtractor is assigned externally
        bool m_IsSubshotStart;
        bool m_IsInSubshot;                                
        bool m_IsReleaseImg;                               //whether to release the rgb image		
        unsigned int m_BgnFrameId;                         //the begine frame id of current sub shot
		unsigned int m_EndFrameId;                         //the end frame id of current sub shot
	};    
}

namespace VideoAnalysis
{
     using namespace VideoAnalysisHelper;
     CSabOnlineSubshotDetector::CSabOnlineSubshotDetector(const CVideoSegmentList& ShotList,
                                                          IOfflineSubshotDetector* pOfflineSgtDetector, 
                                                          IKeyframeExtractor* pKeyframeExtractor,
                                                          bool releaseImg)
    {
            m_pImp = new CSabOnlineSubshotDetectorImp(ShotList, pOfflineSgtDetector, pKeyframeExtractor, releaseImg);
    }
    
    CSabOnlineSubshotDetector::~CSabOnlineSubshotDetector(void)
    {
            delete m_pImp;
    }

    HRESULT CSabOnlineSubshotDetector::OnNewSegment(IVideoSegment& segment)
    {
            return m_pImp->OnNewSegment(segment);
    }

    HRESULT CSabOnlineSubshotDetector::EndOfStream()
    {
            return m_pImp->EndOfStream();
    }
             		
    const CVideoSegmentList& CSabOnlineSubshotDetector::GetSubshots() const
    {
           return m_pImp->GetSubshots();
    }

    const  CVideoSegmentList& CSabOnlineSubshotDetector::GetCompleteShots() const
    {
          return m_pImp->GetCompleteShots();
    }
    HRESULT CSabOnlineSubshotDetector::AddReceiver(IVideoParseReceiver* const pRec)
    {
           return m_pImp->AddReceiver(pRec);
    }

    HRESULT CSabOnlineSubshotDetector::RemoveReceiver(const IVideoParseReceiver* const pRec)
    {
           return m_pImp->RemoveReceiver(pRec);
    }
}


namespace VideoAnalysisHelper
{
	CSabOnlineSubshotDetectorImp::CSabOnlineSubshotDetectorImp(CVideoSegmentList ShotList,
                                                               IOfflineSubshotDetector* pOfflineSgtDetector,
                                                               IKeyframeExtractor* pKeyframeExtractor,
                                                               bool releaseImg)
	  : m_FrameId(0), m_IsInSubshot(false), m_IsSubshotStart(false), m_IsReleaseImg(releaseImg),
		m_SubshotIdx(-1), m_BgnFrameId(0), m_EndFrameId(0),
        m_OfflineSgtDetector(pOfflineSgtDetector),  m_KeyframeExtractor(pKeyframeExtractor), m_ShotList(ShotList)
	{
            //Create the key frame extractor
			if( m_KeyframeExtractor == NULL )
            {
                m_KeyframeExtractor = new CSimpleKeyframeExtractor();
				m_bGetKeyFrameExtractorExternal = false;
			} else
   				m_bGetKeyFrameExtractorExternal = true;
	}

    CSabOnlineSubshotDetectorImp::~CSabOnlineSubshotDetectorImp()
    {
           if( !m_bGetKeyFrameExtractorExternal )
               delete m_KeyframeExtractor;
    }

    HRESULT CSabOnlineSubshotDetectorImp::OnNewSegment(IVideoSegment& segment)
    {
            //do offline subshot detect first
            if( m_SubshotIdx == -1 )
                m_SubshotList = m_OfflineSgtDetector->DetectSegmentOffline();
            
            if( m_SubshotList.Size() != 0 )  //the sub shot maybe empty
            {        
                HRESULT hr = S_OK;
		        try{
				        //Test wether the segment can be cast to a frame, here we just need the frames one by one
			            CFrame& newFrame = dynamic_cast<CFrame&>(segment);
					    //Shot Detect
                        hr = DetectNewSubshot(newFrame);
                        //Call new segment receivers, if there is a shot
                        if( hr == S_OK )
                        {
						    m_Receivers.OnNewSegment(m_SubshotList[m_SubshotIdx]);
                            //because store key frame is so memory cosumed, so the subshot list doesn't store the key frame
                            //the user should register a key frame saver receiver to subshot detector to save the key frame
                            CSubshot & subshot = dynamic_cast<CSubshot&>(m_SubshotList[m_SubshotIdx]);
                            //notice: wether we keep the key frame according the user's input parameter *releaseImg*
                            if( m_IsReleaseImg )
                            {
                                int keyframeSize = subshot.GetKeyframeNum();
                                for( int i = 0; i < keyframeSize; ++i )
                                      subshot.GetKeyframe(i)->ReleaseImage();
                            }
                            
                            //add the sub shot into its parent's child list, so the subshot keyframe lis is only id validate
                            m_ShotList[subshot.ShotId()].AddChild(subshot);
                    }
			    }catch(std::bad_cast &){
                    return E_FAIL;
			    }
                return hr;
            }else
                return S_FALSE;
    }

	HRESULT CSabOnlineSubshotDetectorImp::DetectNewSubshot(CFrame & frame)
	{
            m_FrameId = frame.Id();

		    if ( m_SubshotIdx < (m_SubshotList.Size()-1) && !m_IsInSubshot )
		    {
			     m_SubshotIdx++;

			     m_BgnFrameId = m_SubshotList[m_SubshotIdx].BeginFrameId();
			     m_EndFrameId = m_SubshotList[m_SubshotIdx].EndFrameId();
		   }

		   if ( m_FrameId <= m_BgnFrameId )
		   {
                // We have get the next sub shot, but frame is not arrived the bgn frame of the sub shot
		        // Example: sub shot(bgnFrameId, endFrameId): 1 (1, 10)  2(15, 20)
		        // If we arrive frame 11, we have entered the subshot 2, but we need to set the flag
			    m_IsInSubshot = true;
		   }

		   if ( m_FrameId == m_BgnFrameId )
           {
                m_SubshotList[m_SubshotIdx].BeginTime(frame.BeginTime());
                m_IsSubshotStart = true;
           }
		
           m_KeyframeExtractor->OnNewFrame(frame, m_IsSubshotStart);

		   //we get a new sub shot
		   if ( m_FrameId == m_EndFrameId )
		   {
			    m_IsInSubshot = false;
                m_IsSubshotStart = false;
			    m_SubshotList[m_SubshotIdx].EndTime(frame.EndTime());

                //extract the key frame for current sub shot
                //add the last frame
                m_KeyframeExtractor->OnNewFrame(frame, true);
                CFrameList & keyframeList = m_KeyframeExtractor->OnNewSegment(m_SubshotList[m_SubshotIdx]);
				int size = keyframeList.Size();
					
                for( int i = 0; i < size; ++i )
					  m_SubshotList[m_SubshotIdx].AddKeyframe(keyframeList[i]);

			    return S_OK;
		   }
		   
           return S_FALSE;
	}

    const CVideoSegmentList& CSabOnlineSubshotDetectorImp::GetSubshots() const
    {
            return m_SubshotList;
    }
                             
    const  CVideoSegmentList& CSabOnlineSubshotDetectorImp::GetCompleteShots() const
    {
            return m_ShotList;
    }

	HRESULT CSabOnlineSubshotDetectorImp::EndOfStream()
	{
  			//call all the receiver when it's the end of the stream
			return m_Receivers.EndOfStream();
	}
    
    HRESULT CSabOnlineSubshotDetectorImp::AddReceiver(IVideoParseReceiver* const pRec)
    {
		    return m_Receivers.AddReceiver(pRec);
	}

    HRESULT CSabOnlineSubshotDetectorImp::RemoveReceiver(const IVideoParseReceiver* const pRec)
	{
	        return m_Receivers.RemoveReceiver(pRec);
	}  
}