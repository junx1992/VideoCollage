/*****************************************************************************\

Microsoft Research Asia
Copyright (c) 2002 Microsoft Corporation

File Name:

ImageFeature.h: interface for the functions of extracting image feature

Abstract:

These functions extract image histogram & calculate the goodness

History:

Created on 06/05/2002 by Zhao Peng

\*****************************************************************************/
#include<Windows.h>

#ifndef __IMAGEFEATURE_H_INCLUDED_
#define __IMAGEFEATURE_H_INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define IMGBINNUMBER   256
#define ENTROPYWEIGHT  0.18033688
#define VARIANCEWEIGHT 0.00781250
#define BALANCE        0.5


// Get the importance of an image
//
double GetImportance(
	BYTE *pbImage,				        // Pointer to the source image data
	int   nWidth,                       // Image width
	int   nHeight,                      // Image height
	int   nStride                       // Bytes in one line image
	);


#endif //__IMAGEFEATURE_H_INCLUDED_