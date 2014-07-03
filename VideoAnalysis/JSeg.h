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
#include "IImageSegmentation.h"

namespace ImageAnalysis
{
    ///JSeg image segmentation
	class DLL_IN_EXPORT CJSeg: public IImageSegmentation
	{
	public:
        //// \brief Constrctor
		///@param numberOfScale number of scale. -1 means automatic.
		///@param quanThresh  color quantization threshold, 0-600, -1 default automatic.
		///@param mergeThresh region merge threshold, 0-1.0, default 0.4
		CJSeg(int numberOfScale=-1, float quanThresh=-1, float mergeThresh=0.4);
		virtual unsigned int Segment(const IImage& img, IGrayImage** ppRegionMap) const;
	private:
		int m_numberOfScale;
		float m_quanThresh;
		float m_mergeThresh;
	};
}