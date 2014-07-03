/******************************************************************************\

Microsoft Research Asia
Copyright (c) 2007 Microsoft Corporation

Module Name:
  Image Sdk

Abstract:
  This header file provide the class for extracting texture related feature from image.
  Including:
		1. COT (co-occurence texture)
		2. Wavelet texture

Notes:

History:
  Created on 04/20/2007 by linjuny@microsoft.com
\******************************************************************************/
#pragma once

#include "VxComm.h"
#include "IImageFeatureExtractor.h"
#include "FeatureExtractorBase.h"


namespace ImageAnalysis
{
	///COT (co-occurence texture)
	class DLL_IN_EXPORT CCOTFeatureExtractor: public CFeatureExtractorForRgbImageBase
	{
	public:
		static const int FEATURE_DIM = 16;
	public:
		virtual CDoubleFeature Extract(const IRgbImage& img, const int id = UNDEFINED_FEATURE_ID) const;
		virtual void Extract(CDoubleFeature& feature, const IRgbImage& img) const;
		virtual const int FeatureDim() const;
	};	

    ///wavelet texture
	class DLL_IN_EXPORT CWaveletTextureFeatureExtractor: public CFeatureExtractorBaseUsingFECom
	{
	public:
		static const int FEATURE_DIM = 128;
	public:
		virtual CDoubleFeature Extract(const IRgbImage& img, const int id = UNDEFINED_FEATURE_ID) const;
		virtual void Extract(CDoubleFeature& feature, const IRgbImage& img) const;
		virtual const int FeatureDim() const;
	public:
		virtual void Extract(CFloatFeature& feature, const IRgbImage& img) const;
	};	
}
