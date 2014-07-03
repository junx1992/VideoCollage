/******************************************************************************\

Microsoft Research Asia
Copyright (c) 2007 Microsoft Corporation

Module Name:
  Machine Learning Sdk

Abstract:
  This header file provide definition of all exception which will be thown in Machine Learning SDK

Notes:

History:
  Created on 07/05/2007 by linjuny@microsoft.com
\******************************************************************************/
#pragma once
#include "VxComm.h"
#include <exception>
namespace MachineLearning
{
    ///exception in the learning machine
	class DLL_IN_EXPORT learning_exception: public std::exception
	{
	public:
		learning_exception(const char* const& what);
		learning_exception();
	};
}