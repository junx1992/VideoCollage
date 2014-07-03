#include "StdAfx.h"
#pragma warning(disable: 4819)

#include <string>
#include <streams.h>
#include "guiddef.h"
#include <initguid.h>
#include "VideoParseFilterImp.h"
#include "RgbImage.h"
#include "Log.h"

namespace VideoAnalysisHelper
{
    using namespace ImageAnalysis;
#define FILTER_NAME	L"Video Parse Filter"
// {0785D1E3-73BE-47ed-9B2F-418EA1483B76}
DEFINE_GUID(CLSID_VideoParse, 
0x785d1e3, 0x73be, 0x47ed, 0x9b, 0x2f, 0x41, 0x8e, 0xa1, 0x48, 0x3b, 0x76);

    CVideoParseFilter::CVideoParseFilter(TCHAR *tszName, LPUNKNOWN punk, HRESULT *phr)
        :CTransInPlaceFilter(tszName, punk, CLSID_VideoParse, phr),
         m_currentFrameNo(-1),  //frame no is 0 based
         m_currentStreamTime(0),
         m_imageHeight(0),
         m_imageWidth(0),
         m_imageStride(0),
         m_dbgEndOfStream(false)
    {}

    CVideoParseFilter::~CVideoParseFilter(void)
    {
        _ASSERT(m_dbgEndOfStream);
    }

    // CreateInstance
    // Provide the way for COM to create a CVideoParseFilter object
    CUnknown * WINAPI CVideoParseFilter::CreateInstance(LPUNKNOWN punk, HRESULT *phr) 
    {
        CheckPointer(phr,NULL);
        
        CVideoParseFilter *pNewObject = 
            new CVideoParseFilter(FILTER_NAME, punk, phr );

        if (pNewObject == NULL) 
	    {
            *phr = E_OUTOFMEMORY;
        }

        return pNewObject;
    } // CreateInstance

    HRESULT STDMETHODCALLTYPE CVideoParseFilter::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
    {
	    CheckPointer(ppv,NULL);
	    if ( IID_IVideoParseControllerForFilter == riid )
	    {
		    return GetInterface( static_cast<IVideoParseControllerForFilter*>(this), ppv );
	    }
	    else if ( IID_IVideoParseProgressReport == riid )
	    {
		    return GetInterface( static_cast<IVideoParseProgressReport*>(this), ppv ); 
	    }
	    else 
	    {
		    return CTransInPlaceFilter::NonDelegatingQueryInterface(riid, ppv);
	    }
    }

    HRESULT CVideoParseFilter::CheckInputType(const CMediaType* pType)
    { 
	    if( *pType->Type() != MEDIATYPE_Video )
        {
            return VFW_E_INVALIDMEDIATYPE;
        }

        if( *pType->Subtype() != MEDIASUBTYPE_RGB24 )
        {
            return VFW_E_INVALIDMEDIATYPE;
        }

	    if ( FORMAT_VideoInfo != *pType->FormatType() )	
	    {
		    return VFW_E_INVALIDMEDIATYPE;
	    }

	    return S_OK; 
    }

    //	Override SetMediaType in CTransformFilter
    //	The SetMediaType method is called when the media type is set on one of the filter's pins.
    HRESULT CVideoParseFilter::SetMediaType( PIN_DIRECTION Dir, const CMediaType * pType )
    {
	    if ( PINDIR_INPUT == Dir )
	    {
		    VIDEOINFOHEADER *pvih = (VIDEOINFOHEADER*)pType->Format();
		    m_imageHeight	= pvih->bmiHeader.biHeight;
		    m_imageWidth	= pvih->bmiHeader.biWidth;
            m_imageStride   = (pvih->bmiHeader.biWidth * (pvih->bmiHeader.biBitCount / 8) + 3) & ~3;
	    }
	    return S_OK;
    }

    HRESULT CVideoParseFilter::StartStreaming(void)
    {
	    //HRESULT hr = m_receivers.StartStreaming();
	    HRESULT hr2 = CTransInPlaceFilter::StartStreaming();
        //return FAILED(hr)?hr: hr2;
		return hr2;
    }

    //	Override StopStreaming in CTransformFilter
    //	The StopStreaming method is called when the filter switches to the stopped state.
    HRESULT CVideoParseFilter::StopStreaming(void)
    {
        //HRESULT hr = m_receivers.StopStreaming();
	    HRESULT hr2 = CTransInPlaceFilter::StopStreaming();
        //return FAILED(hr)?hr: hr2;.
		return hr2;
    }

    HRESULT	CVideoParseFilter::EndOfStream(void)
    {
        m_dbgEndOfStream = true;
        HRESULT hr = m_receivers.EndOfStream();
        HRESULT hr2 = CTransInPlaceFilter::EndOfStream();
        return FAILED(hr)?hr: hr2;
    }

    HRESULT CVideoParseFilter::Transform(IMediaSample *pSample)
    {
	    CAutoLock Lock( &m_Lock );
        ++m_currentFrameNo;
        REFERENCE_TIME start = 0, end = 0;
	    HRESULT hr = S_OK;
        RETURN_IF_FAIL(hr = pSample->GetTime(&start, &end));
        m_currentStreamTime = start;
        BYTE* pBuf = NULL;
        RETURN_IF_FAIL(hr = pSample->GetPointer((BYTE**)&pBuf));
        CFrame _frame;
        _frame.Id(m_currentFrameNo);
        _frame.BeginTime(start);
        //sometimes end time may be 0
        if(end > start)
            _frame.EndTime(end);
        else
            _frame.EndTime(start);
        CRgbImage img;
        img.Allocate(m_imageWidth, m_imageHeight, -m_imageStride, pBuf + (m_imageHeight-1)*m_imageStride);
        _frame.AttachImage(&img);
        try{
               hr = m_receivers.OnNewSegment(_frame);
        }catch( std::exception & err){
               VxCore::LogOut(err.what());
               return hr;
        }

        return hr;
    }

    HRESULT CVideoParseFilter::AddReceiver(IVideoParseReceiver* const pRec)
    {
        return m_receivers.AddReceiver(pRec);
    }
    HRESULT CVideoParseFilter::RemoveReceiver(const IVideoParseReceiver* const pRec)
    {
        return m_receivers.RemoveReceiver(pRec);
    }

    int CVideoParseFilter::GetCurrentFrameNo() const
    {
        return m_currentFrameNo;
    }

    __int64 CVideoParseFilter::GetCurrentStreamTime() const
    {
        return m_currentStreamTime;
    }
}