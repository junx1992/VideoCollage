/******************************************************************************\

Microsoft Research Asia
Copyright (c) 2007 Microsoft Corporation

Module Name:
  Video Analysis Sdk

Abstract:
  This header file provide the base classes and helper classes for video parser 

Notes:
  

History:
  Created on 05/22/2007 by linjuny@microsoft.com
\******************************************************************************/
#pragma once
#include "VxComm.h"
#include "VideoParseInterface.h"

namespace VideoAnalysisHelper
{
    class CVideoParseReceiverListImp;
}

namespace VideoAnalysis
{
    ///helper class to implement IVideoParseController
    class DLL_IN_EXPORT CVideoParseReceiverList: public IVideoParseReceiver
    {
    public:
        CVideoParseReceiverList();
        virtual ~CVideoParseReceiverList();
        ///adda receiver
        virtual HRESULT AddReceiver(IVideoParseReceiver* const);
        ///remove a receiver
        virtual HRESULT RemoveReceiver(const IVideoParseReceiver* const);
        ///called when a new segment is detected
        virtual HRESULT OnNewSegment(IVideoSegment& segment);
	    /*virtual HRESULT StartStreaming();
	    virtual HRESULT StopStreaming();*/
        ///called when the end of the stream is reached
	    virtual HRESULT EndOfStream();
    private:
        CVideoParseReceiverList(const CVideoParseReceiverList&);
        CVideoParseReceiverList& operator=(const CVideoParseReceiverList&);
    private:
        VideoAnalysisHelper::CVideoParseReceiverListImp* m_pImp;
    };

	/*class CVideoParseControllerBase: public IVideoParseController
	{
	public:
		virtual HRESULT AddReceiver(IVideoParseReceiver* const);
        virtual HRESULT RemoveReceiver(const IVideoParseReceiver* const);
	protected:
		virtual void FireNewSegmentOnAllReceivers(IVideoSegment& segment);
		virtual HRESULT FireEndOfStreamOnAllReceivers();
	protected:
		CVideoParseReceiverList m_receivers;
	};*/
}