#include "StdAfx.h"
#include "TextureFeatureExtractor.h"
#include "FeatureExtractorHelper\FeatureExtractorHelper.h"
#include <atlbase.h>
#include "VxFeatureHelper.h"
#include "FeatureExtractorHelperUsingFECom.h"

namespace ImageAnalysis
{
#pragma region  CWaveletTextureFeatureExtractor
	void CWaveletTextureFeatureExtractor::Extract(CDoubleFeature& feature, const IRgbImage& img) const
	{
		ATLASSERT(feature.Size() == FEATURE_DIM);
		CFloatFeature _float_feature(UNDEFINED_FEATURE_ID, FEATURE_DIM);
		Extract(_float_feature, img);	
		VxCore::CopyFeatureData(feature, _float_feature);
	}
	CDoubleFeature CWaveletTextureFeatureExtractor::Extract(const IRgbImage& img, const int id) const
	{
		return ImageAnalysisHelper::ExtractImageFeature(this, img, id);
	}

	void CWaveletTextureFeatureExtractor::Extract(CFloatFeature& feature, const IRgbImage& img) const
	{
        feature.Resize(this->FeatureDim());
		ATLASSERT(feature.Size() == FEATURE_DIM);
		int height= img.Height();
		int width = img.Width();
		int stride = img.Stride();
		
		BITMAPINFOHEADER pbmi;
		const BYTE* pbData = img.ImageOrigin();

		pbmi.biHeight = height;
		pbmi.biWidth = width;
		pbmi.biBitCount = 24;
		pbmi.biPlanes = 1;
		pbmi.biCompression = 0;
		pbmi.biSize = sizeof(BITMAPINFOHEADER);
        
		EXCEPTION_IF_FAIL((*m_FECom)->ContainDIB(&pbmi, pbData, NULL), feature_extractor_exception, "can't call feature extractor com dll");
		EXCEPTION_IF_FAIL((*m_FECom)->Extract(FTYPE_WaveletPwtTexture, norVectorLength, feature.DataPtr()), feature_extractor_exception, "can't call feature extractor com dll");
		EXCEPTION_IF_FAIL((*m_FECom)->Extract(FTYPE_WaveletTwtTexture, norVectorLength, feature.DataPtr()+24), feature_extractor_exception, "can't call feature extractor com dll");
		EXCEPTION_IF_FAIL((*m_FECom)->DeContainDIB(), feature_extractor_exception, "can't call feature extractor com dll");

		return;
	}

	const int CWaveletTextureFeatureExtractor::FeatureDim() const
	{
		return FEATURE_DIM;
	}
#pragma endregion

#pragma region  CCOTFeatureExtractor
	CDoubleFeature CCOTFeatureExtractor::Extract(const IRgbImage& img, const int id) const
	{
		return ImageAnalysisHelper::ExtractImageFeature(this, img, id);
	}

	void CCOTFeatureExtractor::Extract(CDoubleFeature& feature, const IRgbImage& img) const
	{
        feature.Resize(this->FeatureDim());
		ATLASSERT(feature.Size() == FEATURE_DIM);
		int height= img.Height();
		int width = img.Width();
		ImageAnalysisHelper::FeatureExtractorHelper::CoocurrenceExtract(&img, feature.DataPtr());
		
		return;
	}

	const int CCOTFeatureExtractor::FeatureDim() const
	{
		return FEATURE_DIM;
	}
#pragma endregion
}