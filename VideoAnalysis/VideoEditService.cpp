#include "Stdafx.h"
#include "VideoEditService.h"
#include "mhl.h"


#define MARK_NONE       0x00
#define MARK_FADE_IN    0x01
#define MARK_FADE_OUT   0x02
#define MARK_FADE_TRANS 0x04

#define SAFE_DELETE_MEDIATYPE(x)\
    if ( x )\
    {\
        DeleteMediaType(x);\
        x = NULL;\
    }                        

namespace VideoAnalysisHelper
{
    using namespace std; 
    using namespace VideoAnalysis;
    using namespace VxCore;
    
    class CVideoEditServiceImp
	{
	public:
		CVideoEditServiceImp();
		~CVideoEditServiceImp();
        //Init the video edit service
		HRESULT Initialize(const VideoEditServiceConfig& _config);
        //build the video dshow filter graph
        HRESULT BuildGraph(const VideoEditServiceConfig & config, const vector<Interval>& IntervalArray);
        ///save the motion thumb nail
        HRESULT Save(double TimeoutInsec, ProgressHandler handler);
        //view the motion thumb nail in a window
        HRESULT Preview(double TimeoutInsec, ProgressHandler handler);

	private:
        //run the filter graph
        HRESULT Run(double dblTimeoutInsec, ProgressHandler handler);
        //clear the com obj
		HRESULT Clear();
		//add group to timeline
		HRESULT AddGroupToTimeline(AM_MEDIA_TYPE *pmt,  IAMTimelineGroup **ppGroup);
		//add video group to timeline using AddGroupToTimeline, and set mediatype to this group
		HRESULT AddVideoGroupToTimeline(IAMTimelineGroup **ppGroup, const WCHAR *wszVideoFullPathName);
		//add Audio group to timeline using AddGroupToTimeline, and set mediatype to this group
		HRESULT AddAudioGroupToTimeline(IAMTimelineGroup **ppGroup);
		//add each 2 track to each group for transition
		HRESULT AddTrackToGroup(IAMTimelineGroup *pGroup, IAMTimelineTrack** ppTrack, LONG lPriority);
		//add video transtion to track using CLSID_Fade
		HRESULT AddVideoTransToTrack(REFERENCE_TIME llTranStart, REFERENCE_TIME llTranStop, IAMTimelineTrack *pTrack, BOOL fFromHghToLow);
		//add video effect to track
		HRESULT AddVideoEffectToTrack(
			     REFERENCE_TIME llTranStart, 
			     REFERENCE_TIME llTranStop, 
			     IAMTimelineTrack *pTrack, 
			     BOOL fFadeIn);
		//add audio effect to track
		HRESULT AddAudioEffectToTrack(
			     REFERENCE_TIME llTranStart, 
			     REFERENCE_TIME llTranStop, 
			     IAMTimelineTrack *pTrack, 
			     BOOL fFadeIn
			);
		//add source to track
		HRESULT AddSourceToTrack(
			    REFERENCE_TIME llTLStart, 
			    REFERENCE_TIME llTLStop, 
			    REFERENCE_TIME llMediaStart, 
			    REFERENCE_TIME llMediaStop, 
			    IAMTimelineTrack *pTrack,
			    const WCHAR *wszVideoFullPathName
			);
		//add video source and effect to track
		HRESULT AddVideoSourceAndEffectToTrack(
			    REFERENCE_TIME llTLStart, 
			    REFERENCE_TIME llTLStop, 
			    REFERENCE_TIME llTransDuration,
			    REFERENCE_TIME llMediaStart, 
			    REFERENCE_TIME llMediaStop, 
			    BYTE bMarkOfEffect,
			    BOOL fFromHghToLow,
			    IAMTimelineTrack *pVideoTrack,
			    const WCHAR *wszVideoFullPathName    
			);
		//add audio source and effect to track
		HRESULT AddAudioSourceAndEffectToTrack(
			    REFERENCE_TIME llTLStart, 
			    REFERENCE_TIME llTLStop, 
			    REFERENCE_TIME llTransDuration,
			    REFERENCE_TIME llMediaStart, 
			    REFERENCE_TIME llMediaStop, 
			    BYTE bMarkOfEffect,
			    BOOL fAudioExist,
			    IAMTimelineTrack *pAudioTrack,
			    const WCHAR *wszVideoFullPathName 
			);
		//create timeline front end
		HRESULT CreateTLFrontEnd(const std::vector<Interval>& IntervalArray, const VideoEditServiceConfig& config);
		//create graph builder to output
		HRESULT CreateTLBackEnd(const VideoEditServiceConfig& _config);
		//handle event, and processing report
		HRESULT HandleEvent(double dblDurationOfVideo, double dblEstimateTime	, ProgressHandler handler);

	private:
		static CLSID        m_CLSIDFadeInOut;
		static CLSID        m_CLSIDFadeTrans;

        //bool   m_IsInitialized;   
        VideoEditServiceConfig m_Config;

		IGraphBuilder       *m_pGraph;
		IMediaEventEx       *m_pMediaEventEx;
		IMediaControl       *m_pMediaControl;
		IMediaSeeking       *m_pMediaSeeking;

		IRenderEngine       *m_pRenderEngine;

		IAMTimeline			*m_pTimeline;
		IAMTimelineGroup	*m_pVideoGroup;
		IAMTimelineGroup	*m_pAudioGroup;
		IAMTimelineTrack	*m_pVideoTrackInLow;
		IAMTimelineTrack	*m_pVideoTrackInHgh;
		IAMTimelineTrack	*m_pAudioTrackInLow;
		IAMTimelineTrack	*m_pAudioTrackInHgh;
	};

    CLSID CVideoEditServiceImp::m_CLSIDFadeInOut = GUID_NULL;
    CLSID CVideoEditServiceImp::m_CLSIDFadeTrans = GUID_NULL;

    //test whether it is a wmv video,according to its extension
    inline bool IsWmvVideo(const wchar_t* videoFileExtension)
	{
		if( wcslen(videoFileExtension) != 3 )
			return false;
		//test whether the extension equals to "WMV"
        if ( towupper(videoFileExtension[0]) == L'W' && towupper(videoFileExtension[1]) == L'M' && towupper(videoFileExtension[2]) == L'V' )
		     return true;
        else
		     return false;
	}

	

    CVideoEditServiceImp::CVideoEditServiceImp():  m_pGraph(NULL),  m_pMediaControl(NULL),  m_pMediaEventEx(NULL), m_pMediaSeeking(NULL),
                                   m_pRenderEngine(NULL), m_pTimeline(NULL), m_pVideoGroup(NULL), m_pAudioGroup(NULL), 
		                           m_pVideoTrackInLow(NULL), m_pVideoTrackInHgh(NULL), 	m_pAudioTrackInLow(NULL),
		                           m_pAudioTrackInHgh(NULL)//, m_IsInitialized(false)
	{
		   CoInitialize(NULL);
          
	}
    CVideoEditServiceImp::~CVideoEditServiceImp()
	{
		  Clear();
		  CoUninitialize();
	}

	HRESULT CVideoEditServiceImp::Clear()
	{
		SAFE_RELEASE(m_pGraph);
		SAFE_RELEASE(m_pMediaEventEx);
		SAFE_RELEASE(m_pMediaControl);
		SAFE_RELEASE(m_pMediaSeeking);

        if( m_pRenderEngine ) 
            m_pRenderEngine->ScrapIt();
		SAFE_RELEASE(m_pRenderEngine);

		SAFE_RELEASE(m_pTimeline);
		SAFE_RELEASE(m_pVideoGroup);
		SAFE_RELEASE(m_pAudioGroup);
		SAFE_RELEASE(m_pVideoTrackInLow);
		SAFE_RELEASE(m_pVideoTrackInHgh);
		SAFE_RELEASE(m_pAudioTrackInLow);
		SAFE_RELEASE(m_pAudioTrackInHgh);

		return S_OK;
	}

    HRESULT CVideoEditServiceImp::Initialize(const VideoEditServiceConfig & config)
    {
	     if( config.m_OutputVideoEffect )
	     {
             HRESULT hr = S_OK;
		     hr = mhl::FindCLSIDByFriendlyName(m_CLSIDFadeInOut, CLSID_VideoEffects1Category, L"Microsoft MovieMaker Fade In Fade Out");
		     LOG_ERROR_RET("Failed to find clsid for fade in and out effect, error code = 0x%x.\n", hr);
		     hr = mhl::FindCLSIDByFriendlyName(m_CLSIDFadeTrans, CLSID_VideoEffects2Category, L"Fade");
		     LOG_ERROR_RET("Failed to find clsid for fade effect, error code = 0x%x.\n", hr);
	     }

         return S_OK;
    }

    HRESULT CVideoEditServiceImp::BuildGraph(const VideoEditServiceConfig & config, const vector<Interval>& IntervalArray)
	{
		do
		{
      		HRESULT hr = S_OK;
            
            //initialize the class if not
            if( m_CLSIDFadeInOut == GUID_NULL || m_CLSIDFadeTrans == GUID_NULL )
                hr = Initialize(config);
            //test whether it is failed
            if( hr != S_OK )
                return hr;
            //clear parameter
            Clear();
            //save the config          
            m_Config = config;

            //the duration may be 0.0, becasuse for some video the dshow api is unable to fetch its duration
            if(  m_Config.m_VideoInfo.GetDuration() == 0.0 )
            {    
                 assert( IntervalArray[IntervalArray.size()-1].endTime >  IntervalArray[0].beginTime );
                 m_Config.m_VideoInfo.SetDuration(IntervalArray[IntervalArray.size()-1].endTime- IntervalArray[0].beginTime);
                 //set the bit rate of the video file
                 m_Config.m_VideoInfo.SetBitRate(m_Config.m_VideoInfo.GetFileSize()*8/m_Config.m_VideoInfo.GetDuration());
            }

			hr = CoCreateInstance(CLSID_AMTimeline, NULL, CLSCTX_INPROC_SERVER, IID_IAMTimeline, (void**)&m_pTimeline);
			LOG_ERROR_RET("Failed to create video timeline, error code = 0x%x.\n", hr);
            hr = AddVideoGroupToTimeline(&m_pVideoGroup, m_Config.m_VideoFileInfo.m_FullPathName);
			LOG_ERROR_RET("Failed to create video timeline group, error code = 0x%x.\n", hr);
			hr = AddTrackToGroup(m_pVideoGroup, &m_pVideoTrackInLow, 0);
			LOG_ERROR_RET("Failed to add tracks into video group, error code = 0x%x.\n", hr);
			hr = AddTrackToGroup(m_pVideoGroup, &m_pVideoTrackInHgh, 1);
			LOG_ERROR_RET("Failed to add tracks into video group, error code = 0x%x.\n", hr);
            if ( m_Config.m_OutputAudio )
			{
				hr = AddAudioGroupToTimeline(&m_pAudioGroup);
				LOG_ERROR_RET("Failed to create video timeline group, error code = 0x%x.\n", hr);
				hr = AddTrackToGroup(m_pAudioGroup, &m_pAudioTrackInLow, 0);
				LOG_ERROR_RET("Failed to add tracks into audio group, error code = 0x%x.\n", hr);
				hr = AddTrackToGroup(m_pAudioGroup, &m_pAudioTrackInHgh, 1);
				LOG_ERROR_RET("Failed to add tracks into audio group, error code = 0x%x.\n", hr);
			}

			hr = CreateTLFrontEnd(IntervalArray, m_Config);
			LOG_ERROR_RET("Failed to connect timeline front end, error code = 0x%x.\n", hr);

			hr = CoCreateInstance(CLSID_RenderEngine, NULL, CLSCTX_INPROC, IID_IRenderEngine, (void **)&m_pRenderEngine);
			LOG_ERROR_RET("Failed to create render engine, error code = 0x%x.\n", hr);
			hr = m_pRenderEngine->SetTimelineObject( m_pTimeline );
			LOG_ERROR_RET("Failed to set timeline object, error code = 0x%x.\n", hr);
			hr = m_pRenderEngine->ConnectFrontEnd();
			LOG_ERROR_RET("Failed to connect front end, error code = 0x%x.\n", hr);

		}while(false);

		return S_OK;
	}
    
    ///save the motion thumb nail
    HRESULT CVideoEditServiceImp::Save(double TimeoutInsecHRESULT, ProgressHandler handler)
    {
		do
		{
      		 HRESULT hr = S_OK;

             hr = m_pRenderEngine->GetFilterGraph(&m_pGraph);
			 LOG_ERROR_RET("Failed to get filter graph, error code = 0x%x.\n", hr);
             hr = m_pGraph->QueryInterface(IID_IMediaControl, (void **)&m_pMediaControl);
			 LOG_ERROR_RET("Failed to get media control interface, error code 0x%x.\n", hr);
			 hr = m_pGraph->QueryInterface(IID_IMediaEventEx, (void **)&m_pMediaEventEx);
			 LOG_ERROR_RET("Failed to get media eventex interface, error code 0x%x.\n", hr);
			 hr = m_pGraph->QueryInterface(IID_IMediaSeeking, (void **)&m_pMediaSeeking);
			 LOG_ERROR_RET("Failed to get media seeking interface, error code 0x%x.\n", hr);

			 hr = CreateTLBackEnd(m_Config);
			 LOG_ERROR_RET("Failed to connect timeline back end, error code = 0x%x.\n", hr);

			 CComPtr<IMediaFilter> pMediaFilter = NULL;
			 hr = m_pGraph->QueryInterface(IID_IMediaFilter, (void **)&pMediaFilter);
			 LOG_ERROR_RET("Failed to get media filter, error code = 0x%x", hr);
			 hr = pMediaFilter->SetSyncSource(NULL);
			 LOG_ERROR_RET("Failed to set full speed to process video, error code = 0x%x", hr);

		}while(false);

		return Run(TimeoutInsecHRESULT, handler);
    }

    HRESULT CVideoEditServiceImp::Preview(double TimeoutInsec, ProgressHandler handler)
    {
         do
		 {
      		  HRESULT hr = S_OK;

              assert( m_pRenderEngine );
              if( m_pRenderEngine == NULL )
                  return E_FAIL;

              hr = m_pRenderEngine->RenderOutputPins();
			  LOG_ERROR("Failed to render output pins of the render engine, error code = 0x%x", hr);

              hr = m_pRenderEngine->GetFilterGraph(&m_pGraph);
			  LOG_ERROR_RET("Failed to get filter graph, error code = 0x%x.\n", hr);
			  hr = m_pGraph->QueryInterface(IID_IMediaControl, (void **)&m_pMediaControl);
			  LOG_ERROR_RET("Failed to get media control interface, error code 0x%x.\n", hr);
			  hr = m_pGraph->QueryInterface(IID_IMediaEventEx, (void **)&m_pMediaEventEx);
			  LOG_ERROR_RET("Failed to get media eventex interface, error code 0x%x.\n", hr);
			  hr = m_pGraph->QueryInterface(IID_IMediaSeeking, (void **)&m_pMediaSeeking);
			  LOG_ERROR_RET("Failed to get media seeking interface, error code 0x%x.\n", hr);

         }while(false);

         return Run(TimeoutInsec, handler);        
    }

    HRESULT CVideoEditServiceImp::Run(double dblTimeoutInsec, ProgressHandler handler)
	{
		HRESULT hr = S_OK;
		while ( S_FALSE == (hr = m_pMediaControl->Run()) );
		LOG_ERROR_RET("Failed to run graph, error code 0x%x.\n", hr);
		return HandleEvent(m_Config.m_VideoInfo.GetDuration(), dblTimeoutInsec, handler);
	}

	HRESULT CVideoEditServiceImp::AddGroupToTimeline(AM_MEDIA_TYPE *pmt, IAMTimelineGroup **ppGroup)
	{
		HRESULT hr = S_OK;

		CComPtr<IAMTimelineObj> pObj = NULL;
		hr = m_pTimeline->CreateEmptyNode( &pObj, TIMELINE_MAJOR_TYPE_GROUP);
		LOG_ERROR_RET("Failed to create timeline obj, error code = 0x%x.\n", hr);
		hr = pObj->QueryInterface(IID_IAMTimelineGroup, (void**)ppGroup);
		LOG_ERROR_RET("Failed to get timeline group interface, error code = 0x%x.\n", hr);

		hr = (*ppGroup)->SetMediaType(pmt);
		LOG_ERROR_RET("Failed to set media type, error code = 0x%x.\n", hr);
		hr = m_pTimeline->AddGroup(pObj);
		LOG_ERROR_RET("Failed to add group into timeline, error code = 0x%x.\n", hr);

		return S_OK;
	}

	HRESULT CVideoEditServiceImp::AddVideoGroupToTimeline(IAMTimelineGroup **ppGroup, const WCHAR *wszVideoFullPathName)
	{
		HRESULT hr = S_OK;

		AM_MEDIA_TYPE *pmtsrc = NULL;
		AM_MEDIA_TYPE *pmtdst = NULL;
		pmtsrc = mhl::NewMediaType();
		LOG_ERROR_MEM_RET("Failed to extract media type from source, error code = 0x%x.\n", pmtsrc);
		hr = mhl::ExtractMediaType(wszVideoFullPathName, MEDIATYPE_Video, pmtsrc);
		LOG_ERROR_RET("Failed to extract media type from source, error code = 0x%x.\n", hr);
		pmtdst = CreateMediaType(pmtsrc);
		LOG_ERROR_MEM_RET("Failed to extract media type from source, error code = 0x%x.\n", pmtdst);

		VIDEOINFOHEADER *vih = (VIDEOINFOHEADER*)pmtdst->pbFormat;
		vih->AvgTimePerFrame			= 0;
		vih->dwBitErrorRate				= 0;
		vih->dwBitRate					= 0;
		BITMAPINFOHEADER &bih = vih->bmiHeader;
		bih.biClrImportant				= 0;
		bih.biClrUsed					= 0;
		bih.biCompression				= 0;
		bih.biXPelsPerMeter				= 0;
		bih.biYPelsPerMeter				= 0;
		bih.biBitCount					= 24;
		bih.biPlanes					= 1;
		bih.biSize						= sizeof(BITMAPINFOHEADER);
		bih.biSizeImage					= DIBSIZE(bih);
		pmtdst->majortype				= MEDIATYPE_Video;
		pmtdst->subtype					= MEDIASUBTYPE_RGB24;
		pmtdst->formattype				= FORMAT_VideoInfo;
		pmtdst->cbFormat				= sizeof(VIDEOINFOHEADER);
		pmtdst->lSampleSize				= DIBSIZE(bih);
		pmtdst->bFixedSizeSamples		= TRUE;
		pmtdst->bTemporalCompression	= FALSE;

		hr = AddGroupToTimeline(pmtdst, ppGroup);
		LOG_ERROR_RET("Failed to add video group into timeline, error code = 0x%x.\n", hr);
		
		DeleteMediaType(pmtdst);
		DeleteMediaType(pmtsrc);

		return S_OK;
	}

	HRESULT CVideoEditServiceImp::AddAudioGroupToTimeline(IAMTimelineGroup **ppGroup)
	{
		HRESULT hr = S_OK;

		CMediaType mt;
		mt.SetType(&MEDIATYPE_Audio);
		hr = AddGroupToTimeline(&mt, ppGroup);
		LOG_ERROR_RET("Failed to add audio group into timeline, error code = 0x%x.\n", hr);

		return S_OK;
	}

	HRESULT CVideoEditServiceImp::AddTrackToGroup(IAMTimelineGroup *pGroup, IAMTimelineTrack** ppTrack, LONG lPriority)
	{
		HRESULT hr = S_OK;

		CComPtr<IAMTimelineComp> pComp = NULL;
		hr = pGroup->QueryInterface(IID_IAMTimelineComp, (void**)&pComp);
		LOG_ERROR_RET("Failed to get timeline composite interface, error code = 0x%x.\n", hr);
		CComPtr<IAMTimelineObj> pTrackObj = NULL;
		hr = m_pTimeline->CreateEmptyNode(&pTrackObj, TIMELINE_MAJOR_TYPE_TRACK);
		LOG_ERROR_RET("Failed to create track object, error code = 0x%x.\n", hr);
		hr = pComp->VTrackInsBefore(pTrackObj, lPriority);
		LOG_ERROR_RET("Failed to add track object into composite, error code = 0x%x.\n", hr);
		hr = pTrackObj->QueryInterface(IID_IAMTimelineTrack, (void**)ppTrack);
		LOG_ERROR_RET("Failed to get timeline track interface error code = 0x%x.\n", hr);

		return S_OK;
	}

	HRESULT CVideoEditServiceImp::AddVideoTransToTrack(
		       REFERENCE_TIME llTranStart, 
		       REFERENCE_TIME llTranStop, 
		       IAMTimelineTrack *pTrack, 
		       BOOL fFromHghToLow
		)
	{
		HRESULT hr = S_OK;

		CComPtr<IAMTimelineObj> pTranObj = NULL;
		hr = m_pTimeline->CreateEmptyNode(&pTranObj, TIMELINE_MAJOR_TYPE_TRANSITION);
		LOG_ERROR_RET("Failed to create transition object, error code = 0x%x.\n", hr);
		CComPtr<IAMTimelineTransable> pTrackTran = NULL;
		hr = pTrack->QueryInterface(IID_IAMTimelineTransable, (void**)&pTrackTran);
		LOG_ERROR_RET("Failed to get track transition interface, error code = 0x%x.\n", hr);
		hr = pTrackTran->TransAdd(pTranObj);
		LOG_ERROR_RET("Failed to add transtion into track, error code = 0x%x.\n", hr);

		hr = pTranObj->SetSubObjectGUID(m_CLSIDFadeTrans);
		LOG_ERROR_RET("Failed to set object guid, error code = 0x%x.\n", hr);
		hr = pTranObj->SetStartStop(llTranStart, llTranStop);
		LOG_ERROR_RET("Failed to set transtion start and stop time, error code = 0x%x.\n", hr);
		//	Because the transition default direction is from low track to high track,
		//  we have to swap inputs when we transit from high track to low track.
		if ( fFromHghToLow )
		{
			CComPtr<IAMTimelineTrans> pTran = NULL;
			hr = pTranObj->QueryInterface(IID_IAMTimelineTrans, (void**)&pTran);
			LOG_ERROR_RET("Failed to get transtion interface, error code = 0x%x.\n", hr);
			hr = pTran->SetSwapInputs(TRUE);
			LOG_ERROR_RET("Failed to swap inputs, error code = 0x%x.\n", hr);
		}

		return S_OK;
	}

	HRESULT CVideoEditServiceImp::AddVideoEffectToTrack(
		      REFERENCE_TIME llTranStart, 
		      REFERENCE_TIME llTranStop, 
		      IAMTimelineTrack *pTrack, 
		      BOOL fFadeIn
		)
	{
		HRESULT hr = S_OK;

		CComPtr<IAMTimelineObj> pEffectObj = NULL;
		hr = m_pTimeline->CreateEmptyNode(&pEffectObj, TIMELINE_MAJOR_TYPE_EFFECT);
		LOG_ERROR_RET("Failed to create effect object, error code = 0x%x.\n", hr);
		CComPtr<IAMTimelineEffectable> pTrackEffect = NULL;
		hr = pTrack->QueryInterface(IID_IAMTimelineEffectable, (void**)&pTrackEffect);
		LOG_ERROR_RET("Failed to get effectable interface, error code = 0x%x.\n", hr);
		hr = pTrackEffect->EffectInsBefore(pEffectObj, -1);
		LOG_ERROR_RET("Failed to add effect to track, error code = 0x%x.\n", hr);

		hr = pEffectObj->SetStartStop(llTranStart, llTranStop);
		LOG_ERROR_RET("Failed to set effect start and stop time, error code = 0x%x.\n", hr);
		hr = pEffectObj->SetSubObjectGUID(m_CLSIDFadeInOut);
		LOG_ERROR_RET("Failed to set sub object guid for effect object, error code = 0x%x.\n", hr);

		//	Fade In param, variant need to be cleared ?
		DEXTER_PARAM Param1;
		Param1.Name			= SysAllocString(L"FadeIn");
		Param1.dispID		= 0;
		Param1.nValues		= 1;

		DEXTER_VALUE Value1;
		memset(&Value1, 0, sizeof(DEXTER_VALUE) );
		VariantClear( &Value1.v );
		Value1.v.boolVal	= fFadeIn;
		Value1.v.vt			= VT_BOOL;				
		Value1.rt			= 0;
		Value1.dwInterp		= DEXTERF_JUMP;

		//	Fade Color Param
		DEXTER_PARAM Param2;
		Param2.Name			= SysAllocString(L"FadeColor");
		Param2.dispID		= 0;
		Param2.nValues		= 1;

		DEXTER_VALUE Value2;
		memset(&Value2, 0, sizeof(DEXTER_VALUE) );
		VariantClear( &Value2.v );
		Value2.v.lVal		= 0;
		Value2.v.vt			= VT_I4;
		Value2.rt			= 0;
		Value2.dwInterp		= DEXTERF_JUMP;

		do
		{
			CComPtr<IPropertySetter> pVolSetter = NULL;
			hr = CoCreateInstance(CLSID_PropertySetter, NULL, CLSCTX_INPROC_SERVER, IID_IPropertySetter, (void**)&pVolSetter);
			LOG_ERROR_BRK("Failed to create to vol setter, error code = 0x%x.\n", hr);
			hr = pVolSetter->AddProp(Param1, &Value1);
			LOG_ERROR_BRK("Failed to add prop Fade In to Vol setter, error code = 0x%x.\n", hr);
			hr = pVolSetter->AddProp(Param2, &Value2);
			LOG_ERROR_BRK("Failed to add prop Fade Color to Vol setter, error code = 0x%x.\n", hr);
			hr = pEffectObj->SetPropertySetter(pVolSetter);
			LOG_ERROR_BRK("Failed to set vol setter to effect object, error code = 0x%x.\n", hr);

		}while (false);

		SysFreeString(Param1.Name);
		SysFreeString(Param2.Name);

		return S_OK;
	}

	HRESULT CVideoEditServiceImp::AddAudioEffectToTrack(
		     REFERENCE_TIME llTranStart, 
		     REFERENCE_TIME llTranStop, 
		     IAMTimelineTrack *pTrack, 
		     BOOL fFadeIn
		)
	{
		HRESULT hr = S_OK;

		CComPtr<IAMTimelineObj> pEffectObj = NULL;
		hr = m_pTimeline->CreateEmptyNode(&pEffectObj, TIMELINE_MAJOR_TYPE_EFFECT);
		LOG_ERROR_RET("Failed to create effect object, error code = 0x%x.\n", hr);
		CComPtr<IAMTimelineEffectable> pTrackEffect = NULL;
		hr = pTrack->QueryInterface(IID_IAMTimelineEffectable, (void**)&pTrackEffect);
		LOG_ERROR_RET("Failed to get effectable interface, error code = 0x%x.\n", hr);
		hr = pTrackEffect->EffectInsBefore(pEffectObj, -1 );
		LOG_ERROR_RET("Failed to add effect to track, error code = 0x%x.\n", hr);

		hr = pEffectObj->SetStartStop(llTranStart, llTranStop);
		LOG_ERROR_RET("Failed to set effect start and stop time, error code = 0x%x.\n", hr);
		hr = pEffectObj->SetSubObjectGUID(CLSID_AudMixer);
		LOG_ERROR_RET("Failed to set sub object guid for effect object, error code = 0x%x.\n", hr);
		
		DEXTER_PARAM Param;
		Param.Name				= CComBSTR("Vol");
		Param.nValues			= 2;
		//  The 1st DEXTER_VALUE indicates the value of effect beginning, 2nd valud indicates the value of effect end
		DEXTER_VALUE AudioValue[2];
		VariantClear( &AudioValue[0].v);                    //  When effect begins, Volumn is zero
		AudioValue[0].v.dblVal	= fFadeIn ? 0.0 : 1.0;      //  Audio Volume, if fade in then from 0.0 to 1.0 (slience to loud)
		AudioValue[0].v.vt		= VT_R8;				
		AudioValue[0].rt		= 0;						//	relative to the start time of effect
		AudioValue[0].dwInterp	= DEXTERF_JUMP;
		VariantClear(&AudioValue[1].v);		                //  When effect ends, Volume to max		
		AudioValue[1].v.dblVal	= fFadeIn ? 1.0 : 0.0;      //  Audio Volume, if fade in then from 1.0 to 0.0 (slience to loud)
		AudioValue[1].v.vt		= VT_R8;
		AudioValue[1].rt		= llTranStop - llTranStart;	//	relative to the start time of effect
		AudioValue[1].dwInterp	= DEXTERF_INTERPOLATE;

		CComPtr<IPropertySetter> pVolSetter = NULL;
		hr = CoCreateInstance(CLSID_PropertySetter, NULL, CLSCTX_INPROC_SERVER, IID_IPropertySetter, (void**)&pVolSetter);
		LOG_ERROR_RET("Failed to create to vol setter, error code = 0x%x.\n", hr);
		hr = pVolSetter->AddProp(Param, AudioValue);
		LOG_ERROR_RET("Failed to add prop to Vol setter, error code = 0x%x.\n", hr);
		hr = pEffectObj->SetPropertySetter(pVolSetter);
		LOG_ERROR_RET("Failed to set vol setter to effect object, error code = 0x%x.\n", hr);

		return S_OK;
	}

	HRESULT CVideoEditServiceImp::AddSourceToTrack(
		       REFERENCE_TIME llTLStart, 
		       REFERENCE_TIME llTLStop, 
		       REFERENCE_TIME llMediaStart, 
		       REFERENCE_TIME llMediaStop, 
		       IAMTimelineTrack *pTrack,
		       const WCHAR *wszVideoFullPathName
		)
	{
		HRESULT hr = S_OK;

		CComPtr<IAMTimelineObj> pSourceObj = NULL;
		hr = m_pTimeline->CreateEmptyNode( &pSourceObj, TIMELINE_MAJOR_TYPE_SOURCE);
		LOG_ERROR_RET("Failed to create source object, error code = 0x%x.\n", hr);
		hr = pTrack->SrcAdd(pSourceObj);
		LOG_ERROR_RET("Failed to add source object into track, error code = 0x%x.\n", hr);

		CComPtr<IAMTimelineSrc> pSource = NULL;
		hr = pSourceObj->QueryInterface(IID_IAMTimelineSrc, (void**)&pSource);
		LOG_ERROR_RET("Failed to get timeline source interface, error code = 0x%x.\n", hr);
		hr = pSourceObj->SetStartStop(llTLStart, llTLStop);
		LOG_ERROR_RET("Failed to set timeline start and end, error code = 0x%x.\n", hr);
		hr = pSource->SetMediaTimes(llMediaStart, llMediaStop);
		LOG_ERROR_RET("Failed to set source media start and end, error code = 0x%x.\n", hr);
		hr = pSource->SetMediaName(CComBSTR(wszVideoFullPathName));
		LOG_ERROR_RET("Failed to set media source name, error code = 0x%x.\n", hr);

		return S_OK;
	}

	HRESULT CVideoEditServiceImp::AddVideoSourceAndEffectToTrack(
		       REFERENCE_TIME llTLStart, 
		       REFERENCE_TIME llTLStop, 
		       REFERENCE_TIME llTransDuration,
		       REFERENCE_TIME llMediaStart, 
		       REFERENCE_TIME llMediaStop, 
		       BYTE bMarkOfEffect,
		       BOOL fFromHghToLow,
		       IAMTimelineTrack *pVideoTrack,
		       const WCHAR *wszVideoFullPathName    
		)
	{
		HRESULT hr = S_OK;
		hr = AddSourceToTrack(llTLStart, llTLStop, llMediaStart, llMediaStop, pVideoTrack, wszVideoFullPathName);
		LOG_ERROR_RET("Failed to add video source to track, error code = 0x%x.\n", hr);
		if ( bMarkOfEffect & MARK_FADE_IN )
		{
			hr = AddVideoEffectToTrack(llTLStart, llTLStart + llTransDuration, pVideoTrack, TRUE);
			LOG_ERROR("Failed to add video fade in effect to track, error code = 0x%x.\n", hr);
		}
		if ( bMarkOfEffect & MARK_FADE_OUT )
		{
			hr = AddVideoEffectToTrack(llTLStop - llTransDuration, llTLStop, pVideoTrack, FALSE);
			LOG_ERROR("Failed to add video fade out effect to track, error code = 0x%x.\n", hr);
		}
		if ( bMarkOfEffect & MARK_FADE_TRANS )
		{
			hr = AddVideoTransToTrack(llTLStart, llTLStart + llTransDuration, m_pVideoTrackInHgh, fFromHghToLow);
			LOG_ERROR("Failed to add transtion on video track, error code = 0x%x.\n", hr);
		}
		return S_OK;
	}

	HRESULT CVideoEditServiceImp::AddAudioSourceAndEffectToTrack(
		   REFERENCE_TIME llTLStart, 
		   REFERENCE_TIME llTLStop, 
		   REFERENCE_TIME llTransDuration,
		   REFERENCE_TIME llMediaStart, 
		   REFERENCE_TIME llMediaStop, 
		   BYTE bMarkOfEffect,
		   BOOL fAudioExist,
		   IAMTimelineTrack *pAudioTrack,
		   const WCHAR *wszVideoFullPathName 
		)
	{
		HRESULT hr = S_OK;
		if ( fAudioExist )
		{
			hr = AddSourceToTrack(llTLStart, llTLStop, llMediaStart, llMediaStop, pAudioTrack, wszVideoFullPathName);
			LOG_ERROR_RET("Failed to add video source to track, error code = 0x%x.\n", hr);
			if ( bMarkOfEffect & MARK_FADE_IN )
			{
				hr = AddAudioEffectToTrack(llTLStart, llTLStart + llTransDuration, pAudioTrack, TRUE);
				LOG_ERROR("Failed to add transtion on audio track, error code = 0x%x.\n", hr);
			}
			if ( bMarkOfEffect & MARK_FADE_OUT )
			{
				hr = AddAudioEffectToTrack(llTLStop - llTransDuration, llTLStop, pAudioTrack, FALSE);
				LOG_ERROR("Failed to add transtion on audio track, error code = 0x%x.\n", hr);
			}
		}
		return S_OK;
	}

	HRESULT CVideoEditServiceImp::CreateTLFrontEnd(const vector<Interval>& IntervalArray, const VideoEditServiceConfig& config)
	{
		HRESULT hr = S_OK;
		size_t uiNumOfIntervals = IntervalArray.size();
		
		REFERENCE_TIME llTLStart		= 0;
		REFERENCE_TIME llTLStop			= 0;
		REFERENCE_TIME llTransDuration	= (REFERENCE_TIME)(config.m_ThumbTransDuarationInSec * UNITS); // UNITS definition in reftime.h
		for ( size_t i = 0; i < uiNumOfIntervals; ++i )
		{				
			double dblStart		= IntervalArray[i].beginTime;
			double dblStop		= IntervalArray[i].endTime;
			double dblDuration  = dblStop - dblStart;

			REFERENCE_TIME llMediaStart = (REFERENCE_TIME)(dblStart * UNITS);
			REFERENCE_TIME llMediaStop = (REFERENCE_TIME)(dblStop * UNITS);
			llTLStart = llTLStop - llTransDuration;
			if ( llTLStart < 0 ) 
			{
				llTLStart = 0;
			}
			llTLStop = llTLStart + (REFERENCE_TIME)(dblDuration * UNITS);

			//  From 2nd clip, we add a transition between 2 clips. So issues should be known:
			//	1. In DES transtion have to be set to high priority track. 
			//  2. Transtion direction is from low to high as default, so we must reverse 
			//  transition direction when changing from high track to low track.
			//  3. Since the 1st clip is put on low track, so i%2 == 0 indicates we are changing 
			//  from high track to low track.
			BOOL fFromHghToLow = (i%2 == 0);
			BYTE bVideoMarkOfEffect = MARK_NONE;
			BYTE bAudioMarkOfEffect = MARK_NONE;
		
            if( config.m_OutputVideoEffect )
			{
				//  We add a Fade In effect at the head of 1st clip, and a Fade Out effect at the tail of the last clip.
				bVideoMarkOfEffect |= ((CLSID_NULL != m_CLSIDFadeInOut) && (i == 0)) ? MARK_FADE_IN : MARK_NONE;
				bVideoMarkOfEffect |= ((CLSID_NULL != m_CLSIDFadeInOut) && (i == uiNumOfIntervals - 1)) ? MARK_FADE_OUT : MARK_NONE;
				bVideoMarkOfEffect |= ((CLSID_NULL != m_CLSIDFadeTrans) && (i != 0)) ? MARK_FADE_TRANS : MARK_NONE;
				bAudioMarkOfEffect = MARK_FADE_IN | MARK_FADE_OUT;
			}
			else
			{
				bVideoMarkOfEffect = MARK_NONE;
				bAudioMarkOfEffect = MARK_NONE;
			}
			hr = AddVideoSourceAndEffectToTrack(
				llTLStart, llTLStop, llTransDuration, llMediaStart, llMediaStop,
				bVideoMarkOfEffect, fFromHghToLow, (i%2 == 0) ? m_pVideoTrackInLow : m_pVideoTrackInHgh, 
                config.m_VideoFileInfo.m_FullPathName);
			LOG_ERROR_RET("Failed to add video source and effect to track, error code = 0x%x.\n", hr);
			//	We add a Volume Fade In effect at the head of every clip, and Volume Fade Out effect at the tail of every clip.
			hr = AddAudioSourceAndEffectToTrack(
				llTLStart, llTLStop, llTransDuration, llMediaStart, llMediaStop,
                bAudioMarkOfEffect, config.m_OutputAudio, (i%2 == 0) ? m_pAudioTrackInLow : m_pAudioTrackInHgh, 
                config.m_VideoFileInfo.m_FullPathName);
			LOG_ERROR_RET("Failed to add video source and effect to track, error code = 0x%x.\n", hr);
		}

		return S_OK;
	}

	HRESULT CVideoEditServiceImp::CreateTLBackEnd(const VideoEditServiceConfig& config)
	{
		HRESULT hr = S_OK;

		WCHAR wszMotionFullPathName[MAX_PATH] = {0};
        swprintf_s(wszMotionFullPathName, MAX_PATH, L"%s",config.m_MotionFileInfo.m_FullPathName);

		CComPtr<ICaptureGraphBuilder2> pBuilder2 = NULL;
		hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC, IID_ICaptureGraphBuilder2, (void **)&pBuilder2);
		LOG_ERROR_RET("Failed to create graph bulder2, error code 0x%x.\n", hr);
		hr = pBuilder2->SetFiltergraph(m_pGraph);
		LOG_ERROR_RET("Failed to set graph to graph bulder2, error code 0x%x.\n", hr);
		CComPtr<IBaseFilter> pMux = NULL;
		hr = pBuilder2->SetOutputFileName(&MEDIASUBTYPE_Asf, wszMotionFullPathName, &pMux, NULL);
		LOG_ERROR_RET("Failed to set output filename to builder2, error code 0x%x.\n", hr);

		CComPtr<IBaseFilter> pCompress = NULL;
		CComPtr<IWMProfile> pProfile = NULL;
        if( config.GetProfile(&pProfile) != S_OK || !config.m_OutputAudio )
		{
			hr = mhl::WMVSourceLoad(
				&pProfile, 
                config.m_VideoFileInfo.m_FullPathName,
                IsWmvVideo(config.m_VideoFileInfo.m_FileExt),
                config.m_OutputAudio,
                DWORD(config.m_VideoInfo.GetBitRate())
				);	
		}   
		LOG_ERROR_RET("Failed to get profile, error code 0x%x.\n", hr);
		if ( pProfile )
		{
			CComPtr<IConfigAsfWriter> pConfig = NULL;
			hr = pMux->QueryInterface(IID_IConfigAsfWriter, (void **)&pConfig);
			LOG_ERROR_RET("Failed to get config interface, error code 0x%x.\n", hr);
			hr = pConfig->ConfigureFilterUsingProfile( pProfile );
			LOG_ERROR_RET("Failed to set profile to writer, error code 0x%x.\n", hr);
		}


		LONG lIdxOfGroup = 0;
		LONG lNumOfGroup = 0;
		hr = m_pTimeline->GetGroupCount(&lNumOfGroup);
		LOG_ERROR_RET("Failed to get group num of timeline, error code 0x%x.\n", hr);
		for ( LONG i = 0; i < lNumOfGroup; ++i )
		{
			CComPtr<IPin> pPin = NULL;
			if ( FAILED(m_pRenderEngine->GetGroupOutputPin(i, &pPin)) )
			{
				continue;
			}

			CComPtr<IEnumMediaTypes> pEnum = NULL;
			hr = pPin->EnumMediaTypes(&pEnum);
			LOG_ERROR_RET("Failed to get enum of media type, error code 0x%x.\n", hr);

			AM_MEDIA_TYPE *pmt = NULL;
			while ( S_OK == pEnum->Next(1, &pmt, NULL) )
			{
				if ( MEDIATYPE_Video == pmt->majortype )
				{
					++lIdxOfGroup;
					hr = pBuilder2->RenderStream(NULL, &MEDIATYPE_Video, pPin, pCompress, pMux);
					LOG_ERROR_BRK("Failed to render video pin, error code 0x%x.\n", hr);
					break;
				}
				if ( MEDIATYPE_Audio == pmt->majortype )
				{
					++lIdxOfGroup;
					hr = pBuilder2->RenderStream(NULL, &MEDIATYPE_Audio, pPin, NULL, pMux);
					LOG_ERROR_BRK("Failed to connect audio pin, error code 0x%x.\n", hr);
					break;
				}
				SAFE_DELETE_MEDIATYPE(pmt);
			}
			SAFE_DELETE_MEDIATYPE(pmt);
		}
		LOG_ERROR_RET("Failed to get video pin or audio pin from timeline, we cannot render this video, error code 0x%x.\n", ( 0 == lIdxOfGroup ) ? E_FAIL : S_OK);

		return S_OK;
	}

	HRESULT CVideoEditServiceImp::HandleEvent(double dblDurationOfVideo, double dblEstimateTime, ProgressHandler handler)
	{
		HRESULT hr = S_OK;

		__int64 total = static_cast<__int64>(UNITS * dblDurationOfVideo) + 1;

		HANDLE hEvent = NULL;
		hr = m_pMediaEventEx->GetEventHandle((OAEVENT*)&hEvent);
		LOG_ERROR_RET("Failed to get event handle, error code 0x%x.\n", hr);

        __int64 preProgress = 0;
        int lingerCounter = 0;
		while (true)
		{
			DWORD dwRet = WaitForSingleObject(hEvent, 1000);
			if ( WAIT_TIMEOUT == dwRet )
			{
				//	Event handle, if output processing is over 60 mins, we will stop graph due to time out.
				//	So if you want to process a long time compress procedure, please modify the below value, 
				//	or else we only get a partial video.
				if( dblEstimateTime-- < 0.0 )	
					return m_pMediaControl->Stop();

				__int64 progress = 0;
				hr = m_pMediaSeeking->GetCurrentPosition(&progress);
                
                if( preProgress  == progress )
                {
                    lingerCounter ++;
                    if( lingerCounter > 5 )  //the limit is 10 second
                    {    //something must be wrong , the filter graph is being stopped, so we stop this filter
					    m_pMediaControl->Stop();
                        return E_FAIL;
                    }   
                } else {
                    preProgress = progress;
                    lingerCounter = 0;
                }
                //notify the progress
                if( handler )
                    handler( progress*1.0 / total );
			} else if ( WAIT_OBJECT_0 == dwRet )  {
				long lEventCode	= 0;
				long lParam1	= 0;
				long lParam2	= 0;
				while ( SUCCEEDED(m_pMediaEventEx->GetEvent(&lEventCode, &lParam1, &lParam2, 0)) )
				{
					hr = m_pMediaEventEx->FreeEventParams(lEventCode, lParam1, lParam2);
					switch (lEventCode)
					{
						case EC_USERABORT:
						case EC_ERRORABORT:
							   m_pMediaControl->Stop(); 
                               return hr;
							
						case EC_COMPLETE:
                                //notify the completion
                                if( handler )
                                    handler(1.00);
							    return hr;
						default:
							    ATLTRACE(L"The event code: %d \n", lEventCode);
							    break;
					}
				}
			} else 
			  	break;
		}	
		return hr;
	}
}

namespace VideoAnalysis
{    
    using namespace VideoAnalysisHelper;

    const double VideoEditServiceConfig::m_DurationOfTransInSec = 0.5;  //	For transition expected duration
	const double VideoEditServiceConfig::m_DurationOfClipInSec = 2.5;  //	For clip expected duration, must more than c_dblDurationOfTransInSec

    HRESULT VideoEditServiceConfig::SetProfileBySystemId(double dwSystemProfileId)
	{
		        return mhl::WMVSystemLoad(&m_WMVProfile, (DWORD)dwSystemProfileId);
	}
	
    HRESULT VideoEditServiceConfig::SetProfileByFile(const WCHAR* profileFileName)
	{
		    assert(profileFileName != NULL);
		    return mhl::WMVCustomLoad(&m_WMVProfile, profileFileName);
	}
	
    void VideoEditServiceConfig::SetMotionFileName(WCHAR* MotionFileName)
	{
            if( MotionFileName != NULL )
                 m_MotionFileInfo = ParseFileFullPathName(MotionFileName);
	}

    HRESULT VideoEditServiceConfig::GetProfile(IWMProfile** pProfile) const
	{
		    if( m_WMVProfile == NULL )
		    {
		        *pProfile = NULL;
		         return S_FALSE;
		    } else {
		         *pProfile = m_WMVProfile;
		         (*pProfile)->AddRef();
		         return S_OK;
		    }
	}

    VideoEditServiceConfig::VideoEditServiceConfig()
    {
            m_OutputAudio = false;
            m_OutputVideoEffect = false;
            m_ThumbTransDuarationInSec = 0.0;
            m_WMVProfile = NULL;   
    }

	VideoEditServiceConfig::VideoEditServiceConfig(const CVideoInfo& videoInfo, const FileInfo & fileInfo, bool bOutputAudio, bool bOutputVideoEffect)
	{
            m_OutputAudio = bOutputAudio;
            m_OutputVideoEffect = bOutputVideoEffect, 
            m_ThumbTransDuarationInSec = 0.0;
            m_WMVProfile = NULL;

            //video info
            m_VideoInfo = videoInfo;
            //file info
            m_VideoFileInfo = fileInfo;

            //motion info
            wchar_t wszMotionFileName[MAX_PATH];
            swprintf(wszMotionFileName, L"%s\\%s_Thunmnail.%s", fileInfo.m_FilePath, fileInfo.m_FileNameWithoutExt, L"wmv");
            m_MotionFileInfo = ParseFileFullPathName(wszMotionFileName);
	}

    VideoEditServiceConfig::VideoEditServiceConfig(const VideoEditServiceConfig& cpy_config)
    {
            m_OutputAudio = cpy_config.m_OutputAudio;
            m_OutputVideoEffect = cpy_config.m_OutputVideoEffect; 
            m_ThumbTransDuarationInSec = cpy_config.m_ThumbTransDuarationInSec;
            cpy_config.GetProfile(&m_WMVProfile);   

            m_MotionFileInfo = cpy_config.m_MotionFileInfo;
            m_VideoFileInfo = cpy_config.m_VideoFileInfo;
            m_VideoInfo = cpy_config.m_VideoInfo;     
    }
    
    VideoEditServiceConfig & VideoEditServiceConfig::operator = (const VideoEditServiceConfig& cpy_config)
    {
            m_OutputAudio = cpy_config.m_OutputAudio;
            m_OutputVideoEffect = cpy_config.m_OutputVideoEffect; 
            m_ThumbTransDuarationInSec = cpy_config.m_ThumbTransDuarationInSec;
            cpy_config.GetProfile(&m_WMVProfile);   

            m_MotionFileInfo = cpy_config.m_MotionFileInfo;
            m_VideoFileInfo = cpy_config.m_VideoFileInfo;
            m_VideoInfo = cpy_config.m_VideoInfo; 

            return *this;
    }

	VideoEditServiceConfig::~VideoEditServiceConfig()
	{
		    SAFE_RELEASE( m_WMVProfile )
	}

     CVideoEditService::CVideoEditService()
     {
            m_pImp = new CVideoEditServiceImp();
     }
	 CVideoEditService::~CVideoEditService()
     {
           delete m_pImp;
     }

     HRESULT CVideoEditService::BuildGraph(const VideoEditServiceConfig & config, const vector<Interval>& intervalArray)
     {
           assert( m_pImp );
           return m_pImp->BuildGraph(config, intervalArray);
     }

     HRESULT CVideoEditService::Save(double TimeoutInsec, ProgressHandler handler)
     {
           assert( m_pImp );
           return m_pImp->Save(TimeoutInsec, handler);
     }

     HRESULT CVideoEditService::Preview(double TimeoutInsec, ProgressHandler handler)
     {
           assert( m_pImp );
           return m_pImp->Preview(TimeoutInsec, handler);         
     }
}