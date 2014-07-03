#include "StdAfx.h"
#include <typeinfo>
#include <iostream>
#include "DThreshShotDetector.h"
#include "VideoParseBase.h"
#include "SimpleKeyframeExtractor.h"
#include "CommonFrameFeatureFactory.h"
#include "ImageProcess.h"


namespace VideoAnalysisHelper
{
	using namespace VideoAnalysis;

	///the process state
	//enum ProcessState 
	//{
	//	PROCESS_NONE = -1,                                      ///mean nothing happens
	//	PROCESS_FLASH_CANDIDATE,                                ///mean it is a flashlight candidate
	//	PROCESS_GRADUAL_TRANSITION_UNFINISH,                    ///mean it is still in the period of gradual transition
	//	PROCESS_NORMAL                                          ///mean it is in the common state
	//};

	/////the result state after process
	//enum ResultState 
	//{
	//	RESULT_NONE = -1,
	//	RESULT_CUT,                                              ///mean it is a shot
	//	RESULT_CUT_PRE,                                          ///mean it is a shot in previous
	//	RESULT_GRADUAL_TRANSITION,                               ///mean it is still in the period of gradual transition
	//	RESULT_FLASH_CANDIDATE,                                  ///mean it may be a flashlight
	//	RESULT_FLASH,                                            ///mean it is a flashlight
	//	RESULT_NORMAL                                            ///mean it is in the common state
	//};

	///The threshold for shot detects
	//struct ShotDetectThreshold
	//{
	//	static const double	dblDiffLowThresh;                    ///0.0959
	//	static const double	dblDiffRatioThresh;                  ///3.0,	Threshold for the ratio of current histogram diff and previous histogram diff
	//	static const double	dblDiffThresh;                       ///0.22	Threshold for histogram diff
	//	static const double	dblKeyFrameThresh;                   ///0.22	Threshold for key frame extraction
	//	static const int	iAvgYDiffThresh;                     ///3       Threshold for average intensity of a frame
	//	static const int	iShotDuration;                       ///30		We assume that a shot could not be shorter than this
	//	static const int	iFlashDuration;                      ///5		We assume that a flash light could not be longer than this
	//	static const int	iGradualTransDuration;               ///7		we assume that a Gradual transition should not be less than this
	//	static const int	iSkippedFrames;                      ///5		we should skip the first frames for denoising
	//};

	double ShotDetectThreshold::dblDiffLowThresh    = 0.0959;
	double ShotDetectThreshold::dblDiffRatioThresh  = 3.0;
	double ShotDetectThreshold::dblDiffThresh       = 0.22;	
	double ShotDetectThreshold::dblKeyFrameThresh   = 0.22;	
	int ShotDetectThreshold::iAvgYDiffThresh        = 3;		
	int ShotDetectThreshold::iShotDuration          = 30;		
	int ShotDetectThreshold::iFlashDuration         = 5;		
	int ShotDetectThreshold::iGradualTransDuration  = 7;	
	int ShotDetectThreshold::iSkippedFrames         = 5;


	struct FrameFeatureContainer
	{
		unsigned char m_brightness;
		CRGB4096Histo	 m_histogram;   
	};

	class CDThreshShotDetectorImp: public IShotDetector
	{
	public:
		CDThreshShotDetectorImp(IKeyframeExtractor* pKeyframeExtractor = NULL, bool releaseImg = true);
		~CDThreshShotDetectorImp(void);
	public:
		///implementation of IVideoParseReceiver
		virtual HRESULT OnNewSegment(IVideoSegment& segment);
		virtual HRESULT EndOfStream();
	public:
		///implementation of IVideoParseController
		virtual HRESULT AddReceiver(IVideoParseReceiver* const);
		virtual HRESULT RemoveReceiver(const IVideoParseReceiver* const);

	public:
		///get the information of all the detected shots.
		///NOTE: the keyframe in the shot is not available.
		virtual const CVideoSegmentList& GetShots() const;

	protected:
		///shot detect
		HRESULT DetectNewShot(CFrame & newFrame);

		///process the frame state during the shot detect
		ResultState ProcessCurProcessState(CFrame & frame);
		double ProcessCurResultState(CFrame & frame);

		///initialize and uninitialize
		void Uninitialize();
		void Initialize(IKeyframeExtractor* pKeyframeExtractor, bool releaseImg);

		///save the frame feature for shot detect 
		void SaveFrameFeature(CFrame &frame);          ///Save the feature information for next sample
	private:
		ProcessState  m_ProcessState;                  ///Indicate the intermediate state of our shot detection process
		ResultState	  m_ResultState;			       ///Indicate the result state of our shot deteciton

		int           m_FrameId;                       ///the index of the frame
		unsigned int  m_ShotId;                        ///the shot id
		unsigned int  m_ImageSize;                     ///the image size
		bool          m_Endflag;                       ///used in EndOfStreams
		bool          m_IsReleaseImg;                  ///wether release the rgb image of every key frame

		/// for Gradual Transition
		int	  	      m_GradualTransPeriod;	           ///the period of gradual frames
		int		      m_GradualTransEndIdx;            ///the end frame index of gradual frames
		int			  m_GradualTransStartIdx;          ///the start frame index of gradual frames
		__int64		  m_GradualTransEndTime;           ///the end time of gradual frames
		__int64		  m_GradualTransStartTime;         ///the start time of gradual frames

		double        m_CurFrameHistDiff;
		double        m_PreFrameHistDiff;
		double		  m_fAnchorHistDiff;               ///Store the HisDiff of the anchor frame of the flash segment
		__int64		  m_TimeStart;                   
		__int64		  m_PreCutFrameTime;
		__int64		  m_FlashTime;	
		int			  m_ShotStart;                     ///the start frame of the shot 
		int			  m_FlashStart;                    ///Remember the start frame No. of the candidate flash
		int			  m_PreCutFrameIndex;              ///Frame number of the previous cut (the 'right' frame of the cut)
		__int64       m_LastFrameEndTime;              ///the end time of last frame

		bool m_bGetKeyFrameExtractorExternal;          ///mark wether the m_KeyframeExtractor given by the uer
		IKeyframeExtractor * m_KeyframeExtractor;      ///the key frame extractor
		CVideoParseReceiverList m_Receivers;           ///hold all the receiverd registered to shot detector
		CShot m_Shot;                                  ///the shot we just detected	
		CVideoSegmentList m_ShotList;                  ///the shot list
		FrameFeatureContainer m_FeaturePre;            ///the common feature of previous frame

		typedef CIntFeature CRGB4096Histo;
		CRGB4096Histo 	m_HistAnchor;
		CRGB4096Histo  	m_HistPreGradualTrans;
	};
}

namespace VideoAnalysis
{
	//get the intersect sum of 2 features, it has been defined in SimpleKeyframeExtractor
	unsigned long Intersect(const CIntFeature &H1, const CIntFeature &H2);

	CDThreshShotDetector::CDThreshShotDetector(IKeyframeExtractor* pKeyframeExtractor, bool releaseImg)
		:m_pImp(new CDThreshShotDetectorImp(pKeyframeExtractor,releaseImg))
	{
	}

	CDThreshShotDetector::~CDThreshShotDetector(void)
	{
		delete m_pImp;
	}

	HRESULT CDThreshShotDetector::OnNewSegment(IVideoSegment& segment)
	{
		return m_pImp->OnNewSegment(segment);
	}

	HRESULT CDThreshShotDetector::EndOfStream()
	{
		return m_pImp->EndOfStream();
	}

	HRESULT CDThreshShotDetector::AddReceiver(IVideoParseReceiver* const pRec)
	{
		return m_pImp->AddReceiver(pRec);
	}

	HRESULT CDThreshShotDetector::RemoveReceiver(const IVideoParseReceiver* const pRec)
	{
		return m_pImp->RemoveReceiver(pRec);
	}

	const CVideoSegmentList& CDThreshShotDetector::GetShots() const
	{
		return m_pImp->GetShots();
	}

}

namespace VideoAnalysisHelper
{
	CDThreshShotDetectorImp::CDThreshShotDetectorImp(IKeyframeExtractor* pKeyframeExtractor, bool releaseImg)
	{
		Initialize(pKeyframeExtractor, releaseImg);
	}
	CDThreshShotDetectorImp::~CDThreshShotDetectorImp(void)
	{
		Uninitialize();
	}

	HRESULT CDThreshShotDetectorImp::OnNewSegment(IVideoSegment& segment)
	{
		HRESULT hr = S_OK;
		try{
			//Test wether the segment can be cast to a frame, here we just need the frames one by one
			CFrame& newFrame = dynamic_cast<CFrame&>(segment);
			//Shot Detect
			hr = DetectNewShot(newFrame);

			if( hr == S_OK )
			{
				//Call new segment receivers, if there is a shot
				m_Shot.Id(++m_ShotId);
				m_Receivers.OnNewSegment(m_Shot);

				//we add the shot into the shot list
				//notice: wether we keep the key frame according the user's input parameter *releaseImg*
				if( m_IsReleaseImg )
				{
					int keyframeSize = m_Shot.GetKeyframeNum();
					for( int i = 0; i < keyframeSize; ++i )
						m_Shot.GetKeyframe(i)->ReleaseImage();
				}
				m_ShotList.Add(m_Shot);
			}

		}catch(std::bad_cast &){
			return E_FAIL;
		}

		return hr;
	}

	HRESULT CDThreshShotDetectorImp::DetectNewShot(CFrame & newFrame)
	{
		HRESULT hr = S_FALSE;

		//hold the frame index
		m_FrameId = newFrame.Id();

		//skip the first frame
		if( m_FrameId > 1 ) 
		{			                 
			// Get histogram difference between neighbour frames
			m_ImageSize = newFrame.GetImage()->Width()*newFrame.GetImage()->Height();
			m_CurFrameHistDiff = 1.0 - double(Intersect(CommonFrameFeatureFactory::GetInstance()->ExtractRGB4096Histo(newFrame), m_FeaturePre.m_histogram)) / m_ImageSize;
			//record the endtime, we need it when call EndofStream
			m_LastFrameEndTime = newFrame.EndTime();                  

			//Process by result state
			m_ResultState = ProcessCurProcessState(newFrame);
			//Get process state
			double _ret = ProcessCurResultState(newFrame);

			//save the current frame's histogram diff
			m_PreFrameHistDiff = m_CurFrameHistDiff;

			//notify the key frame extractor, there is a new frame
			if( m_ResultState == RESULT_NORMAL )
				m_KeyframeExtractor->OnNewFrame(newFrame, true);
			else
				m_KeyframeExtractor->OnNewFrame(newFrame, false);

			//Wow, wether we have found a shot cut?
			hr = (_ret > 0.0 && _ret != 2.0) ? S_OK : S_FALSE;
			if( hr == S_OK )
			{
				//extract some key frames during last shot cut
				CFrameList & keyframeList = m_KeyframeExtractor->OnNewSegment(m_Shot);
				int size = keyframeList.Size();

				//is there any key frame?
				if( size != 0 )
				{
					for( int i = 0; i < size; ++i )
						m_Shot.AddKeyframe(keyframeList[i]);
				} else
					//we can't extract any key frame from last shot cut, so we don't say it a shot cut
					hr = S_FALSE;
			} 
		}

		//Save the feature information for next sample
		SaveFrameFeature(newFrame);

		return hr;
	}

	void CDThreshShotDetectorImp::SaveFrameFeature(CFrame &frame)
	{
		m_FeaturePre.m_brightness = CommonFrameFeatureFactory::GetInstance()->ExtractBrightness(frame);
		m_FeaturePre.m_histogram = CommonFrameFeatureFactory::GetInstance()->ExtractRGB4096Histo(frame);
	}

	ResultState CDThreshShotDetectorImp::ProcessCurProcessState(CFrame & frame)
	{
		static int s_margin = 0;

		//the start and ent time of current frame
		__int64 rtBgn = frame.BeginTime();
		__int64 rtEnd = frame.EndTime();

		ResultState nResultState = RESULT_NONE;

		switch ( m_ProcessState )	
		{
		case PROCESS_NORMAL:  
			//Normal process means nothing happened previously
			if ( m_FrameId >= ShotDetectThreshold::iSkippedFrames ) 
			{			
				if ( m_CurFrameHistDiff >= ShotDetectThreshold::dblDiffThresh && m_CurFrameHistDiff > m_PreFrameHistDiff )
				{
					//Maybe it is flash light
					if ( (CommonFrameFeatureFactory::GetInstance()->ExtractBrightness(frame) - m_FeaturePre.m_brightness) >= ShotDetectThreshold::iAvgYDiffThresh ) 
					{
						nResultState = RESULT_FLASH_CANDIDATE;
					} 
					//It is a CUT
					else if ( (m_FrameId - m_PreCutFrameIndex) > ShotDetectThreshold::iShotDuration ) 
					{	
						nResultState = RESULT_CUT;
					} 
					else 
					{	
						nResultState = RESULT_NORMAL;
					}
				} 
				else if ( 
					m_CurFrameHistDiff * 2 >= ShotDetectThreshold::dblDiffThresh && 
					m_PreFrameHistDiff > 0.0 && 
					( m_CurFrameHistDiff / m_PreFrameHistDiff ) >= ShotDetectThreshold::dblDiffRatioThresh 
					)
				{
					nResultState = RESULT_FLASH_CANDIDATE;
				}
				else if (  
					( m_CurFrameHistDiff >= ShotDetectThreshold::dblDiffLowThresh + 0.01 && m_CurFrameHistDiff > m_PreFrameHistDiff * 1.1 ) ||
					( m_CurFrameHistDiff >= ShotDetectThreshold::dblDiffLowThresh - 0.01 && m_CurFrameHistDiff > m_PreFrameHistDiff * 1.7 ) 
					)
				{
					nResultState = RESULT_NORMAL;

					m_GradualTransPeriod++;
					m_GradualTransStartIdx = m_FrameId;
					m_GradualTransStartTime = rtEnd;

					//save histogram
					m_HistPreGradualTrans = m_FeaturePre.m_histogram;
					m_ProcessState = PROCESS_GRADUAL_TRANSITION_UNFINISH;
					s_margin = 2;
				} 
				else 
				{
					nResultState = RESULT_NORMAL;
				}
			} 
			else 
			{
				nResultState = RESULT_NORMAL;
			}
			break;
		case PROCESS_FLASH_CANDIDATE:    
			//Previous informaition shows that flash light MAYBE happen, but we need this process to verify
			if ( m_FrameId - m_FlashStart <= ShotDetectThreshold::iFlashDuration ) 
			{
				// If within maximal flash duration, then Get the histo. diff. between current frame and anchor frame
				double fSkippedHistDiff = 1.0 - double(Intersect(m_HistAnchor, CommonFrameFeatureFactory::GetInstance()->ExtractRGB4096Histo(frame))) / m_ImageSize;
				if ( fSkippedHistDiff < ShotDetectThreshold::dblDiffThresh ) 
				{
					nResultState = RESULT_FLASH;
				}
			} 
			else if ( m_FlashStart - m_PreCutFrameIndex > ShotDetectThreshold::iShotDuration ) 
			{
				nResultState = RESULT_CUT_PRE;			
			} 
			else 
			{
				nResultState = RESULT_NORMAL;
				m_ProcessState = PROCESS_NORMAL;
			}
			break;
		case PROCESS_GRADUAL_TRANSITION_UNFINISH:
			if (
				m_FrameId - m_ShotStart > ShotDetectThreshold::iShotDuration &&
				m_CurFrameHistDiff >= ShotDetectThreshold::dblDiffThresh &&
				m_PreFrameHistDiff > 0.0 &&
				m_CurFrameHistDiff / m_PreFrameHistDiff >= ShotDetectThreshold::dblDiffRatioThresh
				)
			{
				nResultState = RESULT_CUT;
			}
			else if ( m_CurFrameHistDiff >= 0.07 ) 
			{
				//Gradual Transition continues
				m_GradualTransPeriod++;

				if ( 
					m_GradualTransPeriod >= ShotDetectThreshold::iGradualTransDuration && 
					m_GradualTransStartIdx- m_ShotStart > ShotDetectThreshold::iShotDuration 
					)
				{
					//long enough, it may be a gradual transition
					m_GradualTransEndIdx = m_FrameId;
					m_GradualTransEndTime = rtEnd;		
					nResultState = RESULT_GRADUAL_TRANSITION;
				} 
				else 
					nResultState = RESULT_NORMAL;
			} 
			else 
			{
				s_margin--;
				//Gradual Transition terminates
				if ( s_margin == 0 )
				{
					nResultState = RESULT_NORMAL;
					m_ProcessState = PROCESS_NORMAL;
					m_GradualTransPeriod = 0;
					s_margin = 2; 
				}
			}
			break;
		}

		return nResultState;
	}

	double CDThreshShotDetectorImp::ProcessCurResultState(CFrame & frame)
	{
		//the start and ent time of current frame
		__int64 rtBgn = frame.BeginTime();
		__int64 rtEnd = frame.EndTime();

		double _ret = -1.0;	
		switch( m_ResultState )	
		{
			//it's normal, nothing happens, no cut
		case RESULT_NORMAL:		
			_ret = 0.0;			                               			
			break;

			//current frame is a shot cut boundary
		case RESULT_CUT:			
			m_ProcessState = PROCESS_NORMAL;
			m_GradualTransPeriod = 0;

			// Save this CUT
			m_PreCutFrameIndex = m_FrameId;
			m_PreCutFrameTime  = rtEnd;
			_ret = m_CurFrameHistDiff;	// Cut

			// Clear the pre Shot
			m_Shot.RemoveAllKeyFrame();
			m_Shot.RemoveAllChild();

			// Save Shot information
			m_Shot.BeginFrameId( m_ShotStart );
			m_Shot.EndFrameId( m_FrameId  );
			m_Shot.BeginTime( m_TimeStart );
			m_Shot.EndTime( rtEnd );
			m_Shot.SetShotBoundaryType(CShot::SHOT_CUT);
			//m_Shot.Id(++m_ShotId);

			m_ShotStart = m_FrameId+1;
			m_TimeStart  = rtEnd;

			break;

			// The previous frame is a shot cut boundary. There is latency here    
		case RESULT_CUT_PRE:    
			m_ProcessState = PROCESS_NORMAL;
			m_GradualTransPeriod = 0;

			// Save this CUT
			m_PreCutFrameIndex = m_FlashStart;
			m_PreCutFrameTime = m_FlashTime;
			_ret = m_fAnchorHistDiff;	// Cut

			// Clear the pre Shot
			m_Shot.RemoveAllKeyFrame();
			m_Shot.RemoveAllChild();

			// Save Shot information
			m_Shot.BeginFrameId( m_ShotStart );
			m_Shot.EndFrameId( m_FlashStart );
			m_Shot.BeginTime( m_TimeStart );
			m_Shot.EndTime( m_FlashTime );
			m_Shot.SetShotBoundaryType(CShot::SHOT_CUT);
			//m_Shot.Id(++m_ShotId);

			m_ShotStart = m_FlashStart+1;
			m_TimeStart  = m_FlashTime;

			break;

			// Current frame may be a flash candidate
		case RESULT_FLASH_CANDIDATE:
			m_ProcessState = PROCESS_FLASH_CANDIDATE;

			m_FlashStart   = m_FrameId;
			m_FlashTime	= rtEnd;

			//Save candidate histogram and difference
			m_HistAnchor = m_FeaturePre.m_histogram;
			m_fAnchorHistDiff = m_CurFrameHistDiff; 

			_ret = 0.0;
			break;

		case RESULT_FLASH:    
			m_ProcessState = PROCESS_NORMAL;

			//No cut
			_ret = 0.0;  
			break;

		case RESULT_GRADUAL_TRANSITION: 
			m_ProcessState = PROCESS_NORMAL;
			if((m_GradualTransStartIdx - m_ShotStart) > ShotDetectThreshold::iShotDuration)
			{
				m_PreCutFrameIndex = m_GradualTransEndIdx;

				//Clear the pre Shot
				m_Shot.RemoveAllKeyFrame();
				m_Shot.RemoveAllChild();

				//Save the Shot information
				m_Shot.BeginFrameId( m_ShotStart );
				m_Shot.EndFrameId( m_GradualTransStartIdx);
				m_Shot.BeginTime( m_TimeStart );
				m_Shot.EndTime( m_GradualTransStartTime );
				m_Shot.SetShotBoundaryType(CShot::SHOT_GRADUAL);
				//m_Shot.Id(++m_ShotId);

				m_ShotStart = m_GradualTransEndIdx+1;	

				//time format here 
				m_TimeStart  = m_GradualTransEndTime;

				//linjuny@microsoft.com: here the shot gradual transition is discarded, not belonging to any shots
				_ret = 1.0 - double(Intersect(m_HistPreGradualTrans, CommonFrameFeatureFactory::GetInstance()->ExtractRGB4096Histo(frame))) / m_ImageSize;
				m_GradualTransPeriod = 0;
			}
			else
				_ret = 0.0;   
			break;

		default:
			_ret = -1.0;
		}

		return _ret;
	}


	HRESULT CDThreshShotDetectorImp::EndOfStream()
	{
		if( m_Endflag || m_FrameId <= 2 ) return S_FALSE;
		m_Endflag = true;

		//Clear the pre Shot
		m_Shot.RemoveAllKeyFrame();
		m_Shot.RemoveAllChild();

		//Save the end Shot
		m_Shot.BeginFrameId(m_ShotStart);
		m_Shot.EndFrameId(m_FrameId);
		m_Shot.BeginTime(m_TimeStart);
		m_Shot.EndTime(m_LastFrameEndTime);
		m_Shot.Id(++m_ShotId);

		//the last shot does not possess shot boundary type.

		HRESULT hr = S_OK;
		CFrameList & keyframeList = m_KeyframeExtractor->OnNewSegment(m_Shot);
		int size = keyframeList.Size();
		//If we can't extract any key frame, then it's not a shot
		if( size == 0 )   
			hr = S_FALSE;
		else{
			//save the shot's key frames
			for( int i = 0; i < size; ++i )
				m_Shot.AddKeyframe(keyframeList[i]);

			//Call all the receiver when there is a shot
			m_Receivers.OnNewSegment(m_Shot);

			//we add the shot into the shot list
			//notice: wether we keep the key frame according the user's input parameter *releaseImg*
			if( m_IsReleaseImg )
			{
				int keyframeSize = m_Shot.GetKeyframeNum();
				for( int i = 0; i < keyframeSize; ++i )
					m_Shot.GetKeyframe(i)->ReleaseImage();
			}
			m_ShotList.Add(m_Shot);
		}


		//Call all the receiver when it's the end of the stream
		m_Receivers.EndOfStream();

		return hr;
	}

	const CVideoSegmentList& CDThreshShotDetectorImp::GetShots() const
	{
		return m_ShotList;
	}


	//implementation of IVideoParseController
	HRESULT CDThreshShotDetectorImp::AddReceiver(IVideoParseReceiver* const pRec)
	{
		return m_Receivers.AddReceiver(pRec);
	}
	HRESULT CDThreshShotDetectorImp::RemoveReceiver(const IVideoParseReceiver* const pRec)
	{
		return m_Receivers.RemoveReceiver(pRec);
	}  

	void  CDThreshShotDetectorImp::Initialize(IKeyframeExtractor* pKeyframeExtractor, bool releaseImg)
	{
		m_Endflag                = false;
		m_IsReleaseImg           = releaseImg;
		m_FrameId                = 0;
		m_ShotId                 = -1;
		m_ShotStart              = 0;
		m_TimeStart              = 0;
		m_LastFrameEndTime       = 0;
		m_PreCutFrameTime        = 0;
		m_FlashTime              = 0;	
		m_GradualTransPeriod     = 0;
		m_GradualTransEndIdx     = 0;
		m_GradualTransStartIdx   = 0;
		m_GradualTransEndTime    = 0;
		m_GradualTransStartTime  = 0;

		m_CurFrameHistDiff       = 0.0;
		m_PreFrameHistDiff       = 0.0;

		m_fAnchorHistDiff        = 0;
		m_FlashStart             = 0;
		m_PreCutFrameIndex       = 0;
		m_ProcessState           = PROCESS_NORMAL;
		m_ResultState            = RESULT_NORMAL;

		//Create the key frame extractor
		if( pKeyframeExtractor == NULL ) {
			SAFE_NEW_PTR(m_KeyframeExtractor, CSimpleKeyframeExtractor);
			m_bGetKeyFrameExtractorExternal = false;
		} else {
			m_KeyframeExtractor = pKeyframeExtractor;
			m_bGetKeyFrameExtractorExternal = true;
		}
	}

	void CDThreshShotDetectorImp::Uninitialize()
	{
		//release the key frame extractor( test wether we get the  extractor from user at first)
		if( !m_bGetKeyFrameExtractorExternal )
			SAFE_DELETE_PTR(m_KeyframeExtractor);
	}

}