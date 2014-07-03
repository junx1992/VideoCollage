/******************************************************************************\

Microsoft Research Asia
Copyright (c) 2007 Microsoft Corporation

Module Name:
  Image Sdk

Abstract:
  This header file provide the helper functions for image analysis.

Notes:

History:
  Created on 04/17/2007 by linjuny@microsoft.com
\******************************************************************************/
#pragma once
#include "VxComm.h"

namespace ImageAnalysis
{
    ///get tge version information of our sdk
	DLL_IN_EXPORT void GetImageAnalysisSdkVersion(int& mainVersionId, int& minorVersionId);
}
