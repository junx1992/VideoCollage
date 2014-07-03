#include "StdAfx.h"
#include "atlbase.h"
#include "EdgeFeatureExtractor.h"
#include "FeatureExtractorHelper\FeatureExtractorHelper.h"
#include "RgbImage.h"

namespace ImageAnalysis
{
#pragma region  CEDHFeatureExtractor
	CDoubleFeature CEDHFeatureExtractor::Extract(const IRgbImage& img, const int id) const
	{
		return ImageAnalysisHelper::ExtractImageFeature(this, img, id);
	}

	void CEDHFeatureExtractor::Extract(CDoubleFeature& feature, const IRgbImage& img) const
	{
        feature.Resize(this->FeatureDim());
		ATLASSERT(feature.Size() == FEATURE_DIM);
		int height= img.Height();
		int width = img.Width();
		int localWidth = width / 2;
		int localHeight = height / 2;

		CRgbImage subImg;
		UINT stride = (localWidth*3 + 3) &~3;  //four bytes alignment
		BYTE* localImgData = new BYTE[stride*localHeight];
		subImg.Allocate(localWidth, localHeight, stride, localImgData);

		//1st block
		subImg.Copy(img, RECTANGLE(0, 0, localWidth, localHeight) );
		ImageAnalysisHelper::FeatureExtractorHelper::ExtractEDHFeature(&subImg, 15, 5, feature.DataPtr());

		//2nd block
		subImg.Copy(img, RECTANGLE(width-1-localWidth, 0, width-1, localHeight) );
		ImageAnalysisHelper::FeatureExtractorHelper::ExtractEDHFeature(&subImg, 15, 5, feature.DataPtr()+15);

		//3rd block
		subImg.Copy(img, RECTANGLE(0, height-1-localHeight, localWidth, height-1) );
		ImageAnalysisHelper::FeatureExtractorHelper::ExtractEDHFeature(&subImg, 15, 5, feature.DataPtr()+15*2);

		//4th block
		subImg.Copy(img, RECTANGLE(width-1-localWidth, height-1-localHeight, width-1, height-1) );
		ImageAnalysisHelper::FeatureExtractorHelper::ExtractEDHFeature(&subImg, 15, 5, feature.DataPtr()+15*3);

		//5th block
		subImg.Copy(img, RECTANGLE(width/4, height/4, width/4 + localWidth, height/4 + localHeight) );
		ImageAnalysisHelper::FeatureExtractorHelper::ExtractEDHFeature(&subImg, 15, 5, feature.DataPtr()+15*4);

		delete[] localImgData;
		
		return;
	}

	const int CEDHFeatureExtractor::FeatureDim() const
	{
		return FEATURE_DIM;
	}
#pragma endregion
}