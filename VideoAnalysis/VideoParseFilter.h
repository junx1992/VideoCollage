/******************************************************************************\

Microsoft Research Asia
Copyright (c) 2007 Microsoft Corporation

Module Name:
  Video Analysis Sdk

Abstract:
  This header file provide the external interfaces for the video parse filter 

Notes:
  In order to use this header file, you must install DirectShow SDK (contained 
  in Platform SDK or Windows SDK).
  Before including this file, you should include dshow.h firstly

History:
  Created on 05/08/2007 by linjuny@microsoft.com
\******************************************************************************/
#pragma once
#include "VxComm.h"
#include "VideoParseInterface.h"

namespace VideoAnalysis
{
    extern "C" DLL_IN_EXPORT HRESULT CreateVideoParseFilter(const IID& iid, void** ppv);
}