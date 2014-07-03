/******************************************************************************\

Microsoft Research Asia
Copyright (c) 2007 Microsoft Corporation

Module Name:
  Video Analysis Sdk

Abstract:
  This header file provide the external interfaces for the video parser 

Notes:
  In order to use this header file, you must install DirectShow SDK (contained 
  in Platform SDK or Windows SDK).
  Before including this file, you should include dshow.h firstly

History:
  Created on 08/22/2007 by linjuny@microsoft.com
\******************************************************************************/
#pragma once
#include "VxComm.h"
#include <unknwn.h>
#include "VideoParseInterface.h"

namespace VideoAnalysis
{
	// {5AC2D552-0EAB-4dbb-8F64-AA5F1795DB22}
    DEFINE_GUID(IID_IVideoParseControllerForFilter, 
    0x5ac2d552, 0xeab, 0x4dbb, 0x8f, 0x64, 0xaa, 0x5f, 0x17, 0x95, 0xdb, 0x22);
    ///interface of video parser, this is only for filter
    class __declspec(novtable) IVideoParseControllerForFilter: public IUnknown
    {
    public:
        ///add a receiver
        virtual HRESULT AddReceiver(IVideoParseReceiver* const) = 0;
        ///remove a receiver.
        ///if the receiver exist in the receiver list, S_OK returned; else S_FALSE returned.
        virtual HRESULT RemoveReceiver(const IVideoParseReceiver* const) = 0;
		///the virtual destructor, it's important
		virtual ~IVideoParseControllerForFilter()=0{}
    };

	// {8359AEEE-CD09-45dc-8DB8-4FC2E889318A}
    DEFINE_GUID(IID_IVideoParseProgressReport, 
    0x8359aeee, 0xcd09, 0x45dc, 0x8d, 0xb8, 0x4f, 0xc2, 0xe8, 0x89, 0x31, 0x8a);
    ///interface for progress report
    class __declspec(novtable) IVideoParseProgressReport: public IUnknown
    {
    public:
        ///get the number of the frame currently processed
        virtual int GetCurrentFrameNo() const = 0;
        ///get the stream time of the frame currently processed. the time is in 100 ns
        virtual __int64 GetCurrentStreamTime() const = 0;
		///the virtual destructor, it's important
		virtual ~IVideoParseProgressReport()=0{}
    };
}