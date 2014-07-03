#pragma once
#include "FeatureExtractorBase.h"
#include <IImageFeatureExtractor.h>
#include <ColorHistogramFeatureExtractor.h>
#include "ColorHistogramFeatureExtractor.h"
#include "RgbImage.h"

namespace ImageAnalysis
{
	using namespace std;
	class CEntropyFeatureExtractor : public CFeatureExtractorForRgbImageBase
	{
	public:
		CEntropyFeatureExtractor();
		~CEntropyFeatureExtractor();
		virtual CDoubleFeature Extract(const IRgbImage &img, const int id=UNDEFINED_FEATURE_ID) const;
		virtual double Extract(const CIntFeature &hist) const;
		//double Extract(const IRgbImage &img, CRgbImage &outimage);
		virtual void Extract(CDoubleFeature &feature, const IRgbImage &img) const;

		virtual const int  FeatureDim() const;
	};
}