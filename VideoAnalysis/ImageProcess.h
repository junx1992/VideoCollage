/******************************************************************************\

Microsoft Research Asia
Copyright (c) 2007 Microsoft Corporation

Module Name:
  Image Sdk

Abstract:
  This header file provide the interfaces related to image processing 

Notes:
  

History:
  Created on 04/23/2007 by linjuny@microsoft.com
  Updated on 10/23/2013 by v-aotang@microsoft.com
\******************************************************************************/
#pragma once

#include "iimage.h"
#include "VxFeature.h"

namespace ImageAnalysis
{
	using VxCore::CFloatVector;
	///convert rgb image to gray
	DLL_IN_EXPORT HRESULT Rgb2Gray(IGrayImage* dst, const IRgbImage* const src);
	///convert rgb image to gray. If you care about the stride of this return image, please
	///allocate gray image yourself and then call Rgb2Gray(IGrayImage& dst, const IRgbImage& src)
	DLL_IN_EXPORT IGrayImage* Rgb2Gray(const IRgbImage* const src);
	///get image brightness
	DLL_IN_EXPORT BYTE Brightness(const IRgbImage* const src);
	DLL_IN_EXPORT BYTE Brightness(const IGrayImage* const src);
	DLL_IN_EXPORT BYTE Brightness(const IImage* src);
	///Rgb color space to LUV color space.
	///The LUV value of the pixel at (i,j) is [luv[(i+j*imgWidth)*3] luv[(i+j*imgWidth)*3+1] luv[(i+j*imgWidth)*3+2] ]
	DLL_IN_EXPORT void Rgb2Luv(CFloatVector& luv, const IRgbImage* const src);
	///Rgb Color to HSV color for a pixel.
	DLL_IN_EXPORT void RgbToHsv(HsvTriple& hsv, const RgbTriple& rgb);
	///quantize HSV color to a integer in the range [0 35].
	DLL_IN_EXPORT int QuantizeHsv36(const HsvTriple& hsv);


}
