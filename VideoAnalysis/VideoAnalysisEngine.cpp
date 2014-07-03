#include "StdAfx.h"
#include "VideoAnalysisEngine.h"
#include "VideoParseInterface.h"
#include "VideoAnalysisException.h"
#include "VideoParseFilter.h"
#include "VideoParseInterfaceForFilter.h"
using namespace VideoAnalysis;
#include <atlbase.h>
#include "GraphHelper.h"
#include <string>
#include <typeinfo>
#include "VxUtils.h"
#include "mhl_dshow.h"

#define VIDEO_ANALYSIS_ENGINE_EXCEPTION_IF_FAIL(hr, errorMessage)\
        { if(FAILED(hr)) \
            { std::string errInfo = std::string(errorMessage) + std::string(",COM Error: ") + FormatErrorMessage(hr);\
            throw video_analysis_engine_exception(errInfo.c_str()); } }

namespace VideoAnalysisHelper
{
    class CGraphCallBack:public IGraphCallBack
    {
    public:
        CGraphCallBack(VideoAnalysisEngineCallBack callback, CVideoAnalysisEngine* pVAEngine)
            :m_callback(callback),
             m_pVAEngine(pVAEngine)
        {}
        void Do()
        {
            if(m_callback)
                m_callback(m_pVAEngine);
        }
    private:
        CVideoAnalysisEngine* const m_pVAEngine;
        VideoAnalysisEngineCallBack m_callback;
    };
}

namespace VideoAnalysisImp
{
    using namespace VideoAnalysisHelper;
    using namespace VxCore;
#pragma region CVideoBasicInfoExtractor
	class CVideoBasicInfoExtractor: public IVideoParseReceiver
	{
	public:
        CVideoBasicInfoExtractor():m_FrameWidth(0), m_FrameHeight(0), m_FrameNum(0){}
		HRESULT OnNewSegment(IVideoSegment& segment)
        {
                try{
                    //what is needed is just a frame
                    const CFrame & frame = dynamic_cast<const CFrame &>(segment);
                    m_FrameWidth = frame.GetImage()->Width();
                    m_FrameHeight = frame.GetImage()->Height();
                    ++m_FrameNum;
                }catch(std::bad_cast&){
                    return S_FALSE;
                }

                return S_OK;
        }
	    HRESULT EndOfStream(){  return S_OK;   }

        int FrameWidth()const{  return m_FrameWidth; }
        int FrameHeight()const{  return m_FrameHeight; }
        int FrameNum()const{  return m_FrameNum; }

	private:
        //the video information
        int m_FrameWidth;
        int m_FrameHeight;
        int m_FrameNum;
    };
#pragma endregion
#pragma region CVideoAnalysisEngineImp
    class CVideoAnalysisEngineImp
    {
    public:
        CVideoAnalysisEngineImp(void){}
        ///if the file can't be found, a video_analysis_engine_exception exception will be thrown
        CVideoAnalysisEngineImp(const wchar_t* videoFileName)
        {
            Initialize(videoFileName);
        }
        ~CVideoAnalysisEngineImp(void) 
        {
            Uninitialize();
        }
    private:
        ///prohibit the copy of this object
        CVideoAnalysisEngineImp(const CVideoAnalysisEngineImp&);
        CVideoAnalysisEngineImp& operator=(const CVideoAnalysisEngineImp&);

    public:
        ///initialize with a video file name
        ///if something is wrong, a video_analysis_engine_exception exception will be thrown
        void Initialize(const wchar_t* videoFileName)
        {
            InitializeGraph(videoFileName);
        }
        ///get the progress, return the percentage
        double GetProgress() const
        {
            HRESULT hr = S_OK;
            LONGLONG duration;
            if(FAILED(hr = m_graph.GetDuration(&duration)))
                return 0;
            double currentTime = (double)GetCurrentStreamTime();
            return currentTime/((double)duration);
        }
        ///if something is wrong, a video_analysis_engine_exception exception will be thrown
        void Run()
        {
            HRESULT hr = S_OK;
		    while ( S_FALSE == (hr = m_graph.Run()) );
            VIDEO_ANALYSIS_ENGINE_EXCEPTION_IF_FAIL(hr, "Failed to run graph");
        }

        HRESULT Stop()
        {
            return m_graph.Stop();
        }

        void HandleEvent(IGraphCallBack* pCB, int msCallbackTimeout)
        {
            VIDEO_ANALYSIS_ENGINE_EXCEPTION_IF_FAIL(m_graph.HandleEventByDefault(pCB, msCallbackTimeout), "Failed in Handling event");
        }

    public:
        //IVideoParseController
        virtual HRESULT AddReceiver(IVideoParseReceiver* const pReceiver)
        {
            if(!m_pController)
                return E_FAIL;
            else
                return m_pController->AddReceiver(pReceiver);
        }
        virtual HRESULT RemoveReceiver(const IVideoParseReceiver* const pReceiver)
        {
            if(!m_pController)
                return E_FAIL;
            else
                return m_pController->RemoveReceiver(pReceiver);
        }

    public:
        //IVideoParseProgressReport
        virtual int GetCurrentFrameNo() const
        {
            if(!m_pProgressReport)
                return 0;
            else
                return m_pProgressReport->GetCurrentFrameNo();
        }
        virtual __int64 GetCurrentStreamTime() const
        {
            if(!m_pProgressReport)
                return 0;
            else
                return m_pProgressReport->GetCurrentStreamTime();
        }
    public:

        int FrameWidth()const{  return m_videoInfoExtractor.FrameWidth(); }
        int FrameHeight()const{  return m_videoInfoExtractor.FrameHeight(); }
        int FrameNum()const{  return m_videoInfoExtractor.FrameNum(); }

    private:
        void Uninitialize()
        {
            m_graph.Stop();
            m_pVideoParseFilter.Release();
		    m_pController.Release();
		    m_pProgressReport.Release();
        }

        void InitializeGraph(const wchar_t* videoFileName)
        {
            Uninitialize();

		    HRESULT hr = S_OK;
    		
		    CComPtr<IGraphBuilder> pGraph;
		    hr = pGraph.CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC);
            VIDEO_ANALYSIS_ENGINE_EXCEPTION_IF_FAIL(hr, "Failed to create graph builder");    		
		    //	Initialize shot detection filter, add set receiver and detector to controler
		    hr = CreateVideoParseFilter(IID_IBaseFilter, (void**)&m_pVideoParseFilter);
		    VIDEO_ANALYSIS_ENGINE_EXCEPTION_IF_FAIL(hr, "Failed to create video parse filter");
		    hr = m_pVideoParseFilter->QueryInterface(IID_IVideoParseControllerForFilter, (void **)&m_pController);
		    VIDEO_ANALYSIS_ENGINE_EXCEPTION_IF_FAIL(hr, "Failed to get video parse controller interface");
		    hr = m_pVideoParseFilter->QueryInterface(IID_IVideoParseProgressReport, (void **)&m_pProgressReport);
		    VIDEO_ANALYSIS_ENGINE_EXCEPTION_IF_FAIL(hr,"Failed to get video parse progress report interface");

		    //	Build Video Parse Graph Builder
		    CComPtr<IPin>	pVideoPin = NULL;
		    CComPtr<IPin>	pAudioPin = NULL;
		    //	Get audio output pin. Since the current algorithm needn't audio stream, so we set audio pin NULL.
		    //	Skip to connect audio pin !!!
		    hr = mhl::GetAVPinOnSourceFilter(pGraph, videoFileName, &pVideoPin, NULL);
		    VIDEO_ANALYSIS_ENGINE_EXCEPTION_IF_FAIL(hr,"Failed to get video and audio pins");
		    //	We have to add video parse filter after GetVideoAndAudioPin function call, because in this call we maybe 
		    //	nuke all of filter and remove them from graph
		    hr = pGraph->AddFilter(m_pVideoParseFilter, L"Video Parse Filter");
		    VIDEO_ANALYSIS_ENGINE_EXCEPTION_IF_FAIL(hr,"Failed to add video parse filter into graph");
		    hr = mhl::ConnectFilters(pGraph, pVideoPin, m_pVideoParseFilter);
		    VIDEO_ANALYSIS_ENGINE_EXCEPTION_IF_FAIL(hr,"Failed to connect source filter to shot detection filter");
		    hr = mhl::ConnectToNullRender(pGraph, m_pVideoParseFilter);
		    VIDEO_ANALYSIS_ENGINE_EXCEPTION_IF_FAIL(hr,"Failed to connect video parse filter to null video renderer");

		    m_graph.SetFilterGraph(pGraph);
            VIDEO_ANALYSIS_ENGINE_EXCEPTION_IF_FAIL(hr,"Failed to set filter graph");
		    //	Set full speed to process video
		    hr = m_graph.SetSyncSource(NULL);
		    VIDEO_ANALYSIS_ENGINE_EXCEPTION_IF_FAIL(hr,"Failed to set full speed to process video");

            //add the video information extractor
            hr = AddReceiver(&m_videoInfoExtractor);
		    VIDEO_ANALYSIS_ENGINE_EXCEPTION_IF_FAIL(hr,"Failed to add video information extractor");
        }
    private:
        CFilterGraphHelper m_graph;
        CComPtr<IBaseFilter> m_pVideoParseFilter;
		CComQIPtr<IVideoParseControllerForFilter, &IID_IVideoParseControllerForFilter> m_pController;
		CComQIPtr<IVideoParseProgressReport, &IID_IVideoParseProgressReport> m_pProgressReport;

        //the video information extractor
        CVideoBasicInfoExtractor m_videoInfoExtractor;
    };
#pragma endregion
}

namespace VideoAnalysis
{
    using namespace VideoAnalysisHelper;
    using namespace VideoAnalysisImp;

#pragma region CVideoAnalysisEngine
    CVideoAnalysisEngine::CVideoAnalysisEngine(void)
        :m_pImp(new CVideoAnalysisEngineImp() )
    {
        CoInitialize(NULL);
    }

    CVideoAnalysisEngine::~CVideoAnalysisEngine(void)
    {
        delete m_pImp;
        CoUninitialize();
    }

    CVideoAnalysisEngine::CVideoAnalysisEngine(const wchar_t* videoFileName)
    {
		CoInitialize(NULL);
		m_pImp = new CVideoAnalysisEngineImp(videoFileName);
	}

    void CVideoAnalysisEngine::Initialize(const wchar_t* videoFileName)
    {
        _ASSERT(m_pImp != NULL);
        return m_pImp->Initialize(videoFileName);
    }

    double CVideoAnalysisEngine::GetProgress() const
    {
        _ASSERT(m_pImp != NULL);
        return m_pImp->GetProgress();
    }

    void CVideoAnalysisEngine::Run(VideoAnalysisEngineCallBack callback, const int msCallbackTimeout)
    {
        RunUnBlocked();
        return HandleEvent(callback, msCallbackTimeout);
    }

    void CVideoAnalysisEngine::RunUnBlocked()
    {
        _ASSERT(m_pImp != NULL);
        m_pImp->Run();
    }
    
    void CVideoAnalysisEngine::HandleEvent(VideoAnalysisEngineCallBack callback, const int msCallbackTimeout)
    {
        _ASSERT(m_pImp != NULL);
        CGraphCallBack cbo(callback, this);
        return m_pImp->HandleEvent(&cbo, msCallbackTimeout);
    }

    HRESULT CVideoAnalysisEngine::Stop()
    {
        _ASSERT(m_pImp != NULL);
        return m_pImp->Stop();
    }

    HRESULT CVideoAnalysisEngine::AddReceiver(IVideoParseReceiver* const pReceiver)
    {
        _ASSERT(m_pImp != NULL);
        return m_pImp->AddReceiver(pReceiver);
    }

    HRESULT CVideoAnalysisEngine::RemoveReceiver(const IVideoParseReceiver* const pReceiver)
    {
        _ASSERT(m_pImp != NULL);
        return m_pImp->RemoveReceiver(pReceiver);
    }

    int CVideoAnalysisEngine::GetCurrentFrameNo() const
    {
        _ASSERT(m_pImp != NULL);
        return m_pImp->GetCurrentFrameNo();
    }

    __int64 CVideoAnalysisEngine::GetCurrentStreamTime() const
    {
        _ASSERT(m_pImp != NULL);
        return m_pImp->GetCurrentStreamTime();
    }

    int CVideoAnalysisEngine::FrameWidth()const
    {
        _ASSERT(m_pImp != NULL);
        return m_pImp->FrameWidth();
    }

    int CVideoAnalysisEngine::FrameHeight()const
    {
        _ASSERT(m_pImp != NULL);
        return m_pImp->FrameHeight();
    }

    int CVideoAnalysisEngine::FrameNum()const
    {
        _ASSERT(m_pImp != NULL);
        return m_pImp->FrameNum();
    }

#pragma endregion
}