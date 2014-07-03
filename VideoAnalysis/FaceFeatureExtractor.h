/******************************************************************************\

Microsoft Research Asia
Copyright (c) 2007 Microsoft Corporation

Module Name:
  Image Sdk

Abstract:
  This header file provide the class for extracting face feature from image.

Notes:

History:
  Created on 04/18/2007 by linjuny@microsoft.com
\******************************************************************************/
#pragma once

#include "VxComm.h"
#include "IImageFeatureExtractor.h"
#include "FeatureExtractorBase.h"

namespace ImageAnalysis
{
    ///face feature extractor
    ///the face feature has 7 dimensions. They are 
    ///dim 0: count of faces in this image,
    ///dim 1: area of all the faces, 
    ///dim 3: the percentage of the size of the largest face to the image size
    ///dim 4: the bottom of the bounding rectangle containing the largest face
    ///dim 5: the top of the bounding rectangle containing the largest face
    ///dim 6: the right of the bounding rectangle containing the largest face
    ///dim 7: the left of the bounding rectangle containing the largest face
	class DLL_IN_EXPORT CFaceFeatureExtractor: public CFeatureExtractorForRgbImageBase
	{
	public:
		static const int FEATURE_DIM = 7;
	public:
		virtual CDoubleFeature Extract(const IRgbImage& img, const int id = UNDEFINED_FEATURE_ID) const;
		virtual void Extract(CDoubleFeature& feature, const IRgbImage& img) const;
		virtual const int FeatureDim() const;
	};
}