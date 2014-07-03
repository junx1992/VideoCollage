#include "StdAfx.h"
#include "FaceFeatureExtractor.h"
#include "FeatureExtractorHelper\FeatureExtractorHelper.h"
#include <atlbase.h>
#include "FeatureExtractorBase.h"

namespace ImageAnalysis
{
	CDoubleFeature CFaceFeatureExtractor::Extract(const IRgbImage& img, const int id) const
	{
		return ImageAnalysisHelper::ExtractImageFeature(this, img, id);
	}

	void CFaceFeatureExtractor::Extract(CDoubleFeature& feature, const IRgbImage& img) const
	{
        feature.Resize(this->FeatureDim());
		ATLASSERT(feature.Size() == FEATURE_DIM);
		HRESULT hr = ImageAnalysisHelper::FeatureExtractorHelper::FaceExtract(&img, FEATURE_DIM, feature.DataPtr());
		
        if(FAILED(hr))
           throw feature_extractor_exception("error in extract face feature");

    }
	const int CFaceFeatureExtractor::FeatureDim() const
	{
		return FEATURE_DIM;
	}
}