/******************************************************************************\

Microsoft Research Asia
Copyright (c) 2007 Microsoft Corporation

Module Name:
  Image Sdk

Abstract:
  This header file provide definition of all exception which will be thown in ImageAnalysisSdk

Notes:

History:
  Created on 04/17/2007 by linjuny@microsoft.com
\******************************************************************************/
#pragma once
#include "VxComm.h"
#include <exception>
namespace ImageAnalysis
{
	class DLL_IN_EXPORT image_process_exception: public std::exception
	{
	public:
		image_process_exception(const char* const& what);
		image_process_exception();
	};
}