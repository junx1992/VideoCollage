/******************************************************************************\

Microsoft Research Asia
Copyright (c) 2007 Microsoft Corporation

Module Name:
  Image Sdk

Abstract:
  This header file provide the class for extracting Correlogram feature from image.

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
	///correlogram feature.
	///reference: Long, F., Zhang, H.J., and Feng, D.G. (2002). Fundamental of content-based image retrieval, In: Multimedia Information Retrieval and Management (eds. D. Feng, W.C. Siu, and H.Zhang), Springer Press.
	class DLL_IN_EXPORT CCorrelogramFeatureExtractor: public CFeatureExtractorForRgbImageBase
	{
	public:
		static const int FEATURE_DIM = 144;
	public:
		virtual CDoubleFeature Extract(const IRgbImage& img, const int id = UNDEFINED_FEATURE_ID) const;
		virtual void Extract(CDoubleFeature& feature, const IRgbImage& img) const;
		virtual const int FeatureDim() const;
	};

	///36 dim correlogram
	class DLL_IN_EXPORT CCorrelogramRegionFeatureExtractor: public IImageRegionFeatureExtractor
	{
	public:
		CCorrelogramRegionFeatureExtractor(int K=5);
	public:
		static const int FEATURE_DIM = 36;
		virtual CDoubleFeature ExtractRegionFeature(const IImage& img, const IGrayImage& regionMap, unsigned int numOfRegion, const int id = UNDEFINED_FEATURE_ID) const;
		virtual const int FeatureDim() const;
	private:
		int m_K;
	};
}
