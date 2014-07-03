#include "Stdafx.h"
#include <math.h>
#include "ColorEntropyFeatureExtractor.h"
#include "CommonFrameFeatureFactory.h"
#undef FEATURE_DIM
namespace ImageAnalysis
{
	const int FEATURE_DIM = 1;
	CEntropyFeatureExtractor::CEntropyFeatureExtractor()
	{
	}
	CEntropyFeatureExtractor::~CEntropyFeatureExtractor()
	{
	}
	double CEntropyFeatureExtractor::Extract(const CIntFeature &hist) const
	{
		// helper
		double sum = 0;
		double ret = 0;
		for (int i = 0; i < hist.Size(); ++i) {
			sum += hist[i];
			if (hist[i] != 0)
				ret -= hist[i] * log((double)hist[i]);
		}
		ret = log(sum) + ret / sum;
		return ret;
	}
	CDoubleFeature CEntropyFeatureExtractor::Extract(const IRgbImage &img, const int id) const
	{
		// entrance
		CDoubleFeature feature(id, FEATURE_DIM, 0);

		Extract(feature, img);
		return feature;
	}
	void CEntropyFeatureExtractor::Extract(CDoubleFeature &feature, const IRgbImage &img) const
	{
		VideoAnalysis::CFrame frame;
		CRgbImage *cimg = new CRgbImage();
		cimg->Copy(img);
		frame.AttachImage(cimg);
		CIntFeature hist = VideoAnalysis::CommonFrameFeatureFactory::GetInstance()->ExtractRGB4096Histo(frame);
		feature[0] = Extract(hist);
	}
	const int CEntropyFeatureExtractor::FeatureDim() const
	{
		return FEATURE_DIM;
	}
}