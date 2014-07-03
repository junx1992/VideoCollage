/*****************************************************************************\

Microsoft Research Asia
Copyright (c) 2002 Microsoft Corporation

File Name:

ImageFeature.cpp: implementation for the functions of extracting image feature

Abstract:

These functions extract image histogram & calculate the goodness

History:

Created on 06/05/2002 by Zhao Peng

\*****************************************************************************/

#include "stdafx.h"
#include <math.h>

#include "ImageFeature.h"

BOOL g_bImgFeaInit = FALSE;
BYTE g_rgRedToY[IMGBINNUMBER];
BYTE g_rgGreenToY[IMGBINNUMBER];
BYTE g_rgBlueToY[IMGBINNUMBER];

//
// Initialize the table of image feature extraction
//
void InitImageFeature()
{
	double dR = 0.5, dG = 0.5, dB = 0.5;

	for (int i = 0; i < 256; i++)
	{
		g_rgRedToY[i] = (BYTE)dR;
		g_rgGreenToY[i] = (BYTE)dG;
		g_rgBlueToY[i] = (BYTE)dB;

		dR += 0.299 * IMGBINNUMBER / 256;
		dG += 0.587 * IMGBINNUMBER / 256;
		dB += 0.114 * IMGBINNUMBER / 256;
	}
}

// Get the importance of an image
//
double GetImportance(
	BYTE *pbImage,				        // Pointer to the source image data
	int   nWidth,                       // Image width
	int   nHeight,                      // Image height
	int   nStride                       // Bytes in one line image
	)
{
	// Initialize the table
	//
	if (!g_bImgFeaInit)
	{
		InitImageFeature();

		g_bImgFeaInit = TRUE;
	}

	// Get the gray histogram
	//
	int rgiCurHistogram[IMGBINNUMBER];
	int i, j, nGrayValue;
	BYTE *pbLine;
	double dPercent;

	memset(rgiCurHistogram, 0, sizeof(rgiCurHistogram));

#ifdef SAMPLING
	for (i = 0; i < nHeight; i += SAMPLING_RATE, pbImage += SAMPLING_RATE*nStride)
	{
		pbLine = pbImage;

		for (j = 0; j < nWidth; j += SAMPLING_RATE, pbLine += 3 * SAMPLING_RATE)
		{
			nGrayValue = g_rgRedToY[pbLine[2]] +
				g_rgGreenToY[pbLine[1]] +
				g_rgBlueToY[pbLine[0]];

			rgiCurHistogram[nGrayValue] ++;
		}
	}

	dPercent = (SAMPLING_RATE*SAMPLING_RATE) / (nWidth * nHeight);
#else
	for (i = 0; i < nHeight; i++, pbImage += nStride)
	{
		pbLine = pbImage;

		for (j = 0; j < nWidth; j++, pbLine += 3)
		{
			nGrayValue = g_rgRedToY[pbLine[2]] +
				g_rgGreenToY[pbLine[1]] +
				g_rgBlueToY[pbLine[0]];

			rgiCurHistogram[nGrayValue] ++;
		}
	}

	dPercent = 1.0 / (nWidth * nHeight);
#endif

	// Calculate the variance & entropy
	//
	double dVariance = 0;
	double dEntropy = 0;
	double dCurAvgLum = 0;
	double dTmp;

	for (i = 0; i < IMGBINNUMBER; i++)
	{
		if (rgiCurHistogram[i] > 0)
		{
			dTmp = rgiCurHistogram[i] * dPercent;

			dCurAvgLum += dTmp * i;
			dVariance += dTmp * i * i;
			dEntropy -= dTmp * log(dTmp);
		}
	}

	dVariance -= dCurAvgLum * dCurAvgLum;

	return (BALANCE * ENTROPYWEIGHT * dEntropy + (1 - BALANCE) * VARIANCEWEIGHT * sqrt(dVariance));
}