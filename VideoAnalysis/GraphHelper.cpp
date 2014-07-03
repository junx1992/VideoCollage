#include "StdAfx.h"
#include "GraphHelper.h"

namespace VideoAnalysisHelper
{
    CFilterGraphHelper::CFilterGraphHelper(void)
    {
    }

    CFilterGraphHelper::~CFilterGraphHelper(void)
    {
        Uninitialize();
    }

    HRESULT CFilterGraphHelper::SetFilterGraph(IGraphBuilder *pGraph)
    {
        Uninitialize();
		if (pGraph)
		{
			m_pGraph = pGraph;
			m_pEvent = pGraph;
			m_pControl = pGraph;
			m_pSeeking = pGraph;
		}
        return S_OK;
    }

	HRESULT CFilterGraphHelper::Stop()
    {
        HRESULT hr=S_OK;
		if (!m_pControl)
		{
			return E_UNEXPECTED;
		}
        return m_pControl->Stop();
    }

	HRESULT CFilterGraphHelper::Pause()
    {
        if (!m_pControl)
		{
			return E_UNEXPECTED;
		}
		else
		{
			return m_pControl->Pause();
		}
    }

	HRESULT CFilterGraphHelper::Run()
    {
        if (m_pControl)
		{
			return m_pControl->Run();
		}
		else
		{
			return E_UNEXPECTED;
		}
    }

    HRESULT CFilterGraphHelper::HandleEventByDefault(IGraphCallBack* pCB, int msCallbackTimeout)
    {
        HRESULT hr = S_OK;
		LONG evCode = 0, param1 = 0, param2 = 0;
		// Verify that the IMediaEventEx interface is valid
		if (!m_pEvent)
			return E_FAIL;
		// Process events
		bool bDone = false;
		HANDLE hEvent;
		hr = m_pEvent->GetEventHandle((OAEVENT*) &hEvent);
        _ASSERTE(SUCCEEDED(hr));
        FILTER_STATE state = State_Stopped;

        DWORD LastCallBackTime = GetTickCount();
		while(true)
		{
            if(FAILED(hr = m_pControl->GetState(100, (OAFilterState*)&state)) )
            {
                break;
            }
            else if(bDone || (state!=State_Running) )
                break;
           /* if(bDone)
                break;*/
			DWORD dwRet = WaitForSingleObject(hEvent, msCallbackTimeout);
			if ( WAIT_TIMEOUT == dwRet )
			{
				// Call the specified callback function
				if (pCB)
				{
                    pCB->Do();
                    LastCallBackTime = GetTickCount();
				}
			}
			else 
            {
                if ( WAIT_OBJECT_0 == dwRet )
			    {
				    while ((hr = m_pEvent->GetEvent(&evCode, (LONG_PTR *)&param1, 
									      (LONG_PTR *)&param2, 0)) == S_OK)
				    {
					    switch (evCode) 
		                {
			                case EC_ERRORABORT: 
			                case EC_USERABORT:
				                this->Stop();
                                bDone = true;
				                break;
			                case EC_COMPLETE:
                                bDone = true;
				                break;
		                }
					    m_pEvent->FreeEventParams(evCode, param1, param2);
				    }
			    }

                if(pCB != NULL && (GetTickCount() - LastCallBackTime) >= (DWORD)msCallbackTimeout)
                {   
                    pCB->Do();
                    LastCallBackTime = GetTickCount();
                }
            }

		}

		return hr;
    }
    
    HRESULT CFilterGraphHelper::WaitForCompletion(long msTimeout, long *pEvecode)
    {
        if(NULL != m_pEvent)
            return m_pEvent->WaitForCompletion( msTimeout, pEvecode );
        else
            return E_FAIL;
    }

	HRESULT CFilterGraphHelper::GetStopPosition(LONGLONG* streamLength) const
    {
        if(NULL != m_pSeeking)
            return m_pSeeking->GetStopPosition( streamLength );
        else
        {
            *streamLength = 0;
            return E_FAIL;
        }
    }
    
    HRESULT CFilterGraphHelper::GetCurrentPosition(LONGLONG *pCurrent) const
    {
        if(NULL != m_pSeeking)
            return m_pSeeking->GetCurrentPosition(pCurrent );
        else
        {
            *pCurrent = 0;
            return E_FAIL;
        }
    }

	HRESULT CFilterGraphHelper::GetDuration(LONGLONG *pDuration) const
    {
        if(NULL != m_pSeeking)
            return m_pSeeking->GetDuration(pDuration);
        else
        {
            *pDuration = 0;
            return E_FAIL;
        }
    }

    HRESULT CFilterGraphHelper::GetState(FILTER_STATE* state)
    {
        HRESULT hr = S_OK;
        if(NULL != m_pControl)
            return m_pControl->GetState(100, (OAFilterState*)state);
        else
        {
            *state = State_Stopped;
            return E_FAIL;
        }
    }

    HRESULT CFilterGraphHelper::SetSyncSource(IReferenceClock *pClock)
    {
        CComQIPtr<IMediaFilter> pMediaFilter = m_pGraph;
		if( pMediaFilter )
		{
			return pMediaFilter->SetSyncSource(pClock);
		}
        return E_FAIL;
    }

    void CFilterGraphHelper::Uninitialize()
    {
        if (m_pGraph)
		{           
			Stop();
			CComQIPtr<IVideoWindow> pVideo(m_pGraph);
			if (pVideo)
			{
				pVideo->put_Visible(OAFALSE);
				pVideo->put_Owner(NULL);
			}
		}
        m_pGraph.Release();
        m_pEvent.Release();
        m_pControl.Release();
        m_pSeeking.Release();
    }
}