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
    ///LAB color moment, 9 (3 for 1-, 2-, 3-moment respectively) dims
	class DLL_IN_EXPORT CColorLabMoment123FeatureExtractor: public IImageRegionFeatureExtractor, public CFeatureExtractorBaseUsingFECom
	{
	public:
		static const int FEATURE_DIM = 9;
	public:
		virtual CDoubleFeature Extract(const IRgbImage& img, const int id = UNDEFINED_FEATURE_ID) const;
		virtual void Extract(CDoubleFeature& feature, const IRgbImage& img) const;
		virtual const int FeatureDim() const;

	public:
		virtual void Extract(CFloatFeature& feature, const IRgbImage& img) const;
		void Extract(float* pFeature, const IRgbImage& img) const;

	public:
		///reference: Long, F., Zhang, H.J., and Feng, D.G. (2002). Fundamental of content-based image retrieval, In: Multimedia Information Retrieval and Management (eds. D. Feng, W.C. Siu, and H.Zhang), Springer Press.
		virtual CDoubleFeature ExtractRegionFeature(const IImage& img, const IGrayImage& regionMap,unsigned int numOfRegion, const int id = UNDEFINED_FEATURE_ID) const;
	};

	///5*5 block wise Lab color moment with every block is of 9 dimensions
	class DLL_IN_EXPORT CBlockWiseColorLabMoment123FeatureExtractor: public CFeatureExtractorForRgbImageBase
	{
	public:
		static const int FEATURE_DIM = 225;
	public:
		virtual CDoubleFeature Extract(const IRgbImage& img, const int id = UNDEFINED_FEATURE_ID) const;
		virtual void Extract(CDoubleFeature& feature, const IRgbImage& img) const;
		virtual const int FeatureDim() const;

	public:
		void Extract(CFloatFeature& feature, const IRgbImage& img) const;

    private:
        CColorLabMoment123FeatureExtractor m_ColorMomentFE;
	};	
}