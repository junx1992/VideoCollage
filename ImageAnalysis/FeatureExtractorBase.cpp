#include "StdAfx.h"
#include <cassert>
#include "FeatureExtractorBase.h"
#include "FeatureExtractorHelperUsingFECom.h"
using namespace std;

namespace ImageAnalysisHelper
{
	CDoubleFeature ExtractImageFeature(const IImageFeatureExtractor* pFE, const IImage& img, const int id)
	{
		ATLASSERT(pFE != NULL);
		CDoubleFeature _feature(id, pFE->FeatureDim());
		pFE->Extract(_feature, img);
		return _feature;
	}
}

namespace ImageAnalysis
{

	CDoubleFeature CFeatureExtractorForRgbImageBase::Extract(const IImage& img, const int id) const
	{
		const IRgbImage* pRgbImg = dynamic_cast<const IRgbImage*>(&img);
		
        assert(NULL != pRgbImg);
        if(NULL == pRgbImg)
			throw feature_extractor_exception("the img parameter should implement IRgbImage interface");

        return Extract(*pRgbImg, id);
	}

	void CFeatureExtractorForRgbImageBase::Extract(CDoubleFeature& feature, const IImage& img) const
	{
        feature.Resize(this->FeatureDim());
		const IRgbImage* pRgbImg = dynamic_cast<const IRgbImage*>(&img);
        
		assert(NULL != pRgbImg);
        if(NULL == pRgbImg)
           throw feature_extractor_exception("the img parameter should implement IRgbImage interface");

        return Extract(feature, *pRgbImg);
	}
	
	CFeatureExtractorBaseUsingFECom::CFeatureExtractorBaseUsingFECom()
	{
		m_FECom = new ImageAnalysisHelper::CFeatureExtractorHelperUsingFECom();
	}

	CFeatureExtractorBaseUsingFECom::~CFeatureExtractorBaseUsingFECom()
	{
		delete m_FECom;
	}
}