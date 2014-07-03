/******************************************************************************\

Microsoft Research Asia
Copyright (c) 2007 Microsoft Corporation

Module Name:
  Video Analysis Sdk

Abstract:
  This header file provide the helper classes for dshow filter graph 

Notes:

History:
  Created on 05/10/2007 by linjuny@microsoft.com
\******************************************************************************/
#pragma once
#include "VxComm.h"
#include <atlbase.h>
#include <dshow.h>

namespace VideoAnalysisHelper
{
    class IGraphCallBack
    {
    public:
        virtual void Do() = 0;
    };

    class CFilterGraphHelper
    {
    public:
        CFilterGraphHelper(void);
        ~CFilterGraphHelper(void);
    public:
        CFilterGraphHelper(const CFilterGraphHelper&);
        CFilterGraphHelper& operator=(const CFilterGraphHelper&);
        HRESULT SetFilterGraph(IGraphBuilder *pGraph);
	    HRESULT Stop();
	    HRESULT Pause();
	    HRESULT Run();
        HRESULT GetState(FILTER_STATE* state);
        HRESULT HandleEventByDefault(IGraphCallBack* pCB, int msCallbackTimeout);
	    HRESULT WaitForCompletion(long msTimeout, long *pEvecode);
	    HRESULT GetStopPosition(LONGLONG* streamLength) const;
	    HRESULT GetCurrentPosition(LONGLONG *pCurrent) const;
	    HRESULT GetDuration(LONGLONG *pDuration) const;
	    HRESULT SetSyncSource(IReferenceClock *pClock);
    private:
        void Uninitialize();
    private:
        CComPtr<IGraphBuilder>    m_pGraph;
        CComQIPtr<IMediaEventEx>  m_pEvent;
        CComQIPtr<IMediaControl>  m_pControl;
        CComQIPtr<IMediaSeeking>  m_pSeeking;
    };
}