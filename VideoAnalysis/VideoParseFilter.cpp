#include "StdAfx.h"
#include "VideoParseFilter.h"
#include <streams.h>
#include "VideoParseFilterImp.h"

namespace VideoAnalysis
{
    HRESULT CreateVideoParseFilter(const IID& iid, void** ppv)
    {
        HRESULT hr = S_OK;
        CUnknown* pUnk = VideoAnalysisHelper::CVideoParseFilter::CreateInstance(NULL, &hr);
        if(FAILED(hr))
        {
            *ppv = NULL;
            return hr;
        }
        hr = pUnk->NonDelegatingQueryInterface(iid, ppv);
        if(FAILED(hr))
        {
            *ppv = NULL;
            return hr;
        }
        //pUnk->NonDelegatingAddRef();
        return S_OK;
    }
}