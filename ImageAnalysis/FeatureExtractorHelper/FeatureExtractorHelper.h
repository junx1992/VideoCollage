/******************************************************************************\

Microsoft Research Asia
Copyright (c) 2007 Microsoft Corporation

Module Name:
  Video Sdk

Abstract:
  This header file provide the helper functions for image analysis 

Notes:
  

History:
  Created on 04/19/2007 by linjuny@microsoft.com
\******************************************************************************/
#pragma once
#include "VxComm.h"
#include "IImage.h"
using namespace ImageAnalysis;

namespace ImageAnalysisHelper
{
	namespace FeatureExtractorHelper
	{
		int ExtractCorrelagramFeature(const IImage* pImage, unsigned int nColorNum,  double *pFeature);
		int ExtractEDHFeature(const IImage* pImage, unsigned int  nFeatureDim, unsigned int  bnum, double *pFeature);
		HRESULT FaceExtract(const IImage* pImage, const int iDim, double *pFeature);
		HRESULT CoocurrenceExtract(const IImage* pImage, double *pFeature );
	};
};