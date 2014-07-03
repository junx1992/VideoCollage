/******************************************************************************\

Microsoft Research Asia
Copyright (c) 2007 Microsoft Corporation

Module Name:
  VxCore Sdk

Abstract:
  This header file provide definition of all exception which will be thown in ImageAnalysisSdk

Notes:

History:
  Created on 06/25/2007 by linjuny@microsoft.com
\******************************************************************************/
#pragma once
#include "VxComm.h"
#include <exception>

namespace VxCore
{
    class DLL_IN_EXPORT std::exception;
	class DLL_IN_EXPORT feature_access_exception: public std::exception
	{
	public:
		feature_access_exception(const char* const& what);
		feature_access_exception();
	};

	class DLL_IN_EXPORT feature_extractor_exception: public std::exception
	{
	public:
		feature_extractor_exception(const char* const& what);
		feature_extractor_exception();
	};

	class DLL_IN_EXPORT persistence_exception: public std::exception
	{
	public:
		persistence_exception(const char* const& what);
		persistence_exception();
	};
}