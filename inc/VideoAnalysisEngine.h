/******************************************************************************\

Microsoft Research Asia
Copyright (c) 2007 Microsoft Corporation

Module Name:
  Video Analysis Sdk

Abstract:
  This header file provide the class for video analysis engine

Notes:

History:
  Created on 05/09/2007 by linjuny@microsoft.com
\******************************************************************************/
#pragma once
#pragma warning(disable: 4995)

#include "VxComm.h"
#include "VideoParseInterface.h"

namespace VideoAnalysisImp
{
    class CVideoAnalysisEngineImp;
}

namespace VideoAnalysis
{
    class CVideoAnalysisEngine;
    typedef void (*VideoAnalysisEngineCallBack)(CVideoAnalysisEngine* const);

    class DLL_IN_EXPORT CVideoAnalysisEngine
    {
    public:
        CVideoAnalysisEngine(void);
        ///if the file can't be found, a video_analysis_engine_exception exception will be thrown.
        CVideoAnalysisEngine(const wchar_t* videoFileName);
        ~CVideoAnalysisEngine(void);
    private:
        ///prohibit the copy of this object
        CVideoAnalysisEngine(const CVideoAnalysisEngine&);
        CVideoAnalysisEngine& operator=(const CVideoAnalysisEngine&);

    public:
        ///initialize with a video file name.
        ///if something is wrong, a video_analysis_engine_exception exception will be thrown.
        void Initialize(const wchar_t* videoFileName);
        ///get the progress, return the percentage
        double GetProgress() const;
        ///if something is wrong, a video_analysis_engine_exception exception will be thrown.
        ///the function will not return till the video analysis complete or you call stop.
        ///the function is implemented just as:
        ///   RunNonBlocked();
        ///   return HandleEvent(callback, msCallbackTimeout);
        void Run(VideoAnalysisEngineCallBack callback=NULL, const int msCallbackTimeout=1000); 
        ///run and return immediately
        ///after this, you can call HandleEvent for event handler
        void RunUnBlocked();
        ///handle the filter graph event
        void HandleEvent(VideoAnalysisEngineCallBack callback=NULL, const int msCallbackTimeout=1000);
        ///Video Analysis Engine operatoion
        ///stop the video analysis engine
        HRESULT Stop();

    public:
        //IVideoParseController
        virtual HRESULT AddReceiver(IVideoParseReceiver* const);
        virtual HRESULT RemoveReceiver(const IVideoParseReceiver* const);

    public:
        //IVideoParseProgressReport
        virtual int GetCurrentFrameNo() const;
        virtual __int64 GetCurrentStreamTime() const;
    public:
        ///get the video information
        ///get the frame width
        int FrameWidth()const;
        ///get the frame height
        int FrameHeight()const;
        ///get the frame number
        int FrameNum()const;

    private:
        VideoAnalysisImp::CVideoAnalysisEngineImp* m_pImp;
    };
}