/******************************************************************************\

Microsoft Research Asia
Copyright (c) 2007 Microsoft Corporation

Module Name:
  Video Analysis Sdk

Abstract:
  This header file provide the implementation for the video parse filter 

Notes:
  

History:
  Created on 05/08/2007 by linjuny@microsoft.com
\******************************************************************************/
#pragma once

#pragma warning(disable: 4995)

#include "VxComm.h"
#include "VideoParseFilter.h"
#include <vector>
using std::vector;
#include "VideoParseInterfaceForFilter.h"
#include "VideoParseBase.h"

namespace VideoAnalysisHelper
{
    using namespace VideoAnalysis;
    class CVideoParseFilter : 
	    public CTransInPlaceFilter, 
	    public IVideoParseControllerForFilter,
	    public IVideoParseProgressReport
    {
    public:
        // Constructor - just calls the base class constructor
        CVideoParseFilter(TCHAR *tszName, LPUNKNOWN punk, HRESULT *phr);
	    ~CVideoParseFilter(void);

	    // Basic COM - used here to reveal our own interfaces
	    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);
    	
        // Overrides the PURE virtual Transform of CTransInPlaceFilter base class.
        // This is where the "real work" done by altering *pSample.
        // Override methods in CTransformFilter
	    HRESULT SetMediaType(PIN_DIRECTION Dir, const CMediaType *pType);
	    HRESULT CheckInputType(const CMediaType* pType);
	    HRESULT Transform(IMediaSample *pSample);
	    HRESULT StartStreaming(void);
        HRESULT StopStreaming(void);
	    HRESULT EndOfStream(void);
    public:
	    DECLARE_IUNKNOWN;
        static CUnknown *WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *phr);

    public:
        //IVideoParseController
        virtual HRESULT AddReceiver(IVideoParseReceiver* const);
        virtual HRESULT RemoveReceiver(const IVideoParseReceiver* const);

    public:
        //IVideoParseProgressReport
        virtual int GetCurrentFrameNo() const;
        virtual __int64 GetCurrentStreamTime() const;

    private:
	    int m_currentFrameNo;		//	indicate how many frame sample passed
	    __int64 m_currentStreamTime;	//	indicate the length of passed video 
        CCritSec m_Lock;					//	For filter resource critical section
        CVideoParseReceiverList m_receivers; //hold all the receiverd registered to the root
        int m_imageHeight; //frame height
        int m_imageWidth; //frame width
        int m_imageStride; //frame stride

        bool m_dbgEndOfStream;
    };
}