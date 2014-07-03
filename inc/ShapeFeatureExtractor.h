/******************************************************************************\

Microsoft Research Asia
Copyright (c) 2007 Microsoft Corporation

Module Name:
  Image Sdk

Abstract:
  This header file provide the class for extracting shape feature from image.

Notes:

History:
  Created on 07/26/2007 by linjuny@microsoft.com
\******************************************************************************/
#pragma once

#include "VxComm.h"
#include "IImageFeatureExtractor.h"

namespace ImageAnalysis
{
	///shape feature extractor for image region.
	///reference: Yixin Chen, James Z. Wang. Image Categorization by Learning and Reasoning with Regions. Journal of Machine Learning Research 5 (2004) 913¨C939.
	class DLL_IN_EXPORT CShapeFeatureExtractor: public IImageRegionFeatureExtractor
	{
	public:
		static const int FEATURE_DIM = 3;
		virtual const int FeatureDim() const;
		virtual CDoubleFeature ExtractRegionFeature
							   (const IImage& img, const IGrayImage& regionMap,unsigned int numOfRegion, const int id = UNDEFINED_FEATURE_ID) const;
	};
}