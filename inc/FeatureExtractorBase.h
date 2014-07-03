/******************************************************************************\

Microsoft Research Asia
Copyright (c) 2007 Microsoft Corporation

Module Name:
  Image Sdk

Abstract:
  This header file provide the base class for extracting image feature.

Notes:

History:
  Created on 04/18/2007 by linjuny@microsoft.com
\******************************************************************************/
#pragma once

#include "VxComm.h"
#include "IImageFeatureExtractor.h"

namespace ImageAnalysisHelper
{
    using namespace ImageAnalysis;
	///call pFE->Extract(_feature, img);
	///used for the implementation of IImageFeatureExtractor::CDoubleFeature Extract(const IImage* img, const int id = UNDEFINED_FEATURE_ID) const
	///users NEVER call this function unless you implement your own feature extractor
	DLL_IN_EXPORT CDoubleFeature ExtractImageFeature(const IImageFeatureExtractor* pFE, const IImage& img, const int id);
}
namespace ImageAnalysisHelper
{
	class CFeatureExtractorHelperUsingFECom;
}

namespace ImageAnalysis
{
    ///base class for all the feature extractors which can only deal with RGB24 image
	class DLL_IN_EXPORT CFeatureExtractorForRgbImageBase: public IImageFeatureExtractor
	{
	public:
		///more type-safe interface functions for feature extractors which can only accept RGB image
		virtual CDoubleFeature Extract(const IRgbImage& img, const int id = UNDEFINED_FEATURE_ID) const = 0;
		///more type-safe interface functions for feature extractors which can only accept RGB image
		///the feature should be assigned the id
		virtual void Extract(CDoubleFeature& feature, const IRgbImage& img) const = 0;
	private:
		///the IImageFeatureExtractor interface functions can only be called when you explicitly cast
		///this to the interface. these two functions are implemented here as a template method which firstly
		///check whether img is of type IRgbImage, if so redirect the call to Extract(const IRgbImage* img, ...)
		///else throw an exception of feature_extractor_exception
		virtual CDoubleFeature Extract(const IImage& img, const int id = UNDEFINED_FEATURE_ID) const;
		///the feature should be assigned the id
		virtual void Extract(CDoubleFeature& feature, const IImage& img) const;
		
	};

	///some of image feature extractors are implemented in a com dll previously, without the source
	///this class provides a base class for such feature extractors, NEVER used in your customized 
	///feature extractor
	class DLL_IN_EXPORT CFeatureExtractorBaseUsingFECom: public CFeatureExtractorForRgbImageBase
	{
	public:
		///can throw a exception: ImageAnalysis::feature_extractor_exception
		CFeatureExtractorBaseUsingFECom();
		~CFeatureExtractorBaseUsingFECom();

		///for the feature extractors using the previous com dll, we provide two additional functions
		///which employ float feature as parameters. Because the features in the feature extractor
		///of the com dll are all of type float, in order to get the double feature, we carried one copy. 
		///If the performance is your concern, use the following methods for your feature extraction
		///the parameter feature should be assigned the id
		virtual void Extract(CFloatFeature& feature, const IRgbImage& img) const = 0;
	protected:
		mutable ImageAnalysisHelper::CFeatureExtractorHelperUsingFECom* m_FECom;
	private:
		CFeatureExtractorBaseUsingFECom(const CFeatureExtractorBaseUsingFECom&);
		CFeatureExtractorBaseUsingFECom& operator=(const CFeatureExtractorBaseUsingFECom&);
	};
}