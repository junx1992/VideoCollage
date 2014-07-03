/******************************************************************************\

Microsoft Research Asia
Copyright (c) 2007 Microsoft Corporation

Module Name:
  Image Sdk

Abstract:
  This header file provide the interface for extracting features from image.

Notes:

History:
  Created on 04/17/2007 by linjuny@microsoft.com
\******************************************************************************/
#pragma once
#include "IImage.h"
#include "VxFeature.h"

namespace ImageAnalysis
{
	using namespace VxCore;
	///Interface for image feature extractor
	class DLL_IN_EXPORT IImageFeatureExtractor
	{
	public:
		///extract image feature provided a id
		///if you implement this interface, please ensure that the parameter "id" is set a default value
		///UNDEFINED_FEATURE_ID so that users can call this function without explicitly giving the id when
		///the id is not critical
		virtual CDoubleFeature Extract(const IImage& img, const int id = UNDEFINED_FEATURE_ID) const = 0;
		///the feature should be allocated memory and assigned the id
		virtual void Extract(CDoubleFeature& feature, const IImage& img) const = 0;
		///indicate the dimensionality of the feature
		virtual const int FeatureDim() const = 0;
		virtual ~IImageFeatureExtractor() = 0
		{}
	};

	///Interface for image region feature extractor
	class DLL_IN_EXPORT IImageRegionFeatureExtractor
	{
	public:
		///extract region feature.the result feature is of dimensionality FeatureDim()*numOfRegion
		virtual CDoubleFeature ExtractRegionFeature(const IImage& img, const IGrayImage& regionMap, unsigned int numOfRegion, const int id = UNDEFINED_FEATURE_ID) const = 0;
		///indicate the dimensionality of the feature for a single region.
		virtual const int FeatureDim() const = 0;
		virtual ~IImageRegionFeatureExtractor() = 0 {}
	};
}