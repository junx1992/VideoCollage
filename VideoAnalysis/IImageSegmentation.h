/******************************************************************************\

Microsoft Research Asia
Copyright (c) 2007 Microsoft Corporation

Module Name:
  Image Sdk

Abstract:
  This header file provide the interface of Image Segmentation 

Notes:
  

History:
  Created on 07/20/2007 by linjuny@microsoft.com
\******************************************************************************/

#pragma once
#include "VxComm.h"
#include "IImage.h"

namespace ImageAnalysis
{
	///interface for image segmentation
	class DLL_IN_EXPORT IImageSegmentation
	{
	public:
		///return: number of regions.
		///regionMap: out region map. the pixel value in the region map indicates the region label of the pixel
		/// in the original image(img). The region label is one integer in the range [1 numberOfRegion].
		virtual unsigned int Segment(const IImage& img, IGrayImage** ppRegionMap) const = 0;
		virtual ~IImageSegmentation() {}
	};
}