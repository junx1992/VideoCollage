/******************************************************************************\

Microsoft Research Asia
Copyright (c) 2007 Microsoft Corporation

Module Name:
  Image Sdk

Abstract:
  This header file provide the class for extracting color histogram feature from image.
  Including:
		1. hsv 64-bin histogram
		2. hsv 192-bin histogram
		3. rgb 4096-bin histogram

Notes:

History:
  Created on 04/17/2007 by linjuny@microsoft.com
\******************************************************************************/
#pragma once

#include "VxComm.h"
#include "IImageFeatureExtractor.h"
#include "FeatureExtractorBase.h"

namespace ImageAnalysis
{
	///64-bin hsv feature
	///the constructor can throw an exception: ImageAnalysis::feature_extractor_exception
	class DLL_IN_EXPORT CHSV64HistogramFeatureExtractor: public CFeatureExtractorBaseUsingFECom
	{
	public:
		static const int FEATURE_DIM = 64;
	public:
		virtual CDoubleFeature Extract(const IRgbImage& img, const int id = UNDEFINED_FEATURE_ID) const;
		virtual void Extract(CDoubleFeature& feature, const IRgbImage& img) const;
		virtual const int FeatureDim() const;

	public:
		///the feature should be assigned the id
		virtual void Extract(CFloatFeature& feature, const IRgbImage& img) const;
	};

	///4096-bin rgb histogram
	class DLL_IN_EXPORT CRGB4096HistogramFeatureExtractor: public CFeatureExtractorForRgbImageBase
	{
	public:
		static const int FEATURE_DIM = 4096;
	public:
		virtual CDoubleFeature Extract(const IRgbImage& img, const int id = UNDEFINED_FEATURE_ID) const;
		virtual void Extract(CDoubleFeature& feature, const IRgbImage& img) const;
		virtual const int FeatureDim() const;
	public:
		///this function return the histogram without normalization, i.e. it only calculate the pixel number for
		///each RGB color.
		static CIntFeature ExtractNonNormalizeHisto(const IRgbImage& img, const int id = UNDEFINED_FEATURE_ID);
	};

    ///256-bin rgb histogram
	class DLL_IN_EXPORT CRGB256HistogramFeatureExtractor: public CFeatureExtractorForRgbImageBase
	{
	public:
		static const int FEATURE_DIM = 256;
	public:
		virtual CDoubleFeature Extract(const IRgbImage& img, const int id = UNDEFINED_FEATURE_ID) const;
		virtual void Extract(CDoubleFeature& feature, const IRgbImage& img) const;
		virtual const int FeatureDim() const;
	public:
		///this function return the histogram without normalization, i.e. it only calculate the pixel number for
		///each RGB color.
		static CIntFeature ExtractNonNormalizeHisto(const IRgbImage& img, const int id = UNDEFINED_FEATURE_ID);
	};
}