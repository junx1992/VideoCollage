#include "StdAfx.h"
#include "CorrelogramFeatureExtractor.h"
#include "ImageProcess.h"
#include "GrayImage.h"
#include "FeatureExtractorHelper\FeatureExtractorHelper.h"
#include <atlbase.h>
#include <vector>
using std::vector;

namespace ImageAnalysis
{
#pragma region  CCorrelogramFeatureExtractor
	CDoubleFeature CCorrelogramFeatureExtractor::Extract(const IRgbImage& img, const int id) const
	{
		return ImageAnalysisHelper::ExtractImageFeature(this, img, id);
	}

	void CCorrelogramFeatureExtractor::Extract(CDoubleFeature& feature, const IRgbImage& img) const
	{
        feature.Resize(this->FeatureDim());
		ATLASSERT(feature.Size() == FEATURE_DIM);
		ImageAnalysisHelper::FeatureExtractorHelper::ExtractCorrelagramFeature(&img, 144/4,  feature.DataPtr());
		return;
	}

	const int CCorrelogramFeatureExtractor::FeatureDim() const
	{
		return FEATURE_DIM;
	}
#pragma endregion

#pragma region CRegionCorrelogramFeatureExtractor.
	inline int QuantizeRgb36(const RgbTriple& rgb)
	{
		HsvTriple hsv;
		RgbToHsv(hsv, rgb);
		return QuantizeHsv36(hsv);
	}

	CCorrelogramRegionFeatureExtractor::CCorrelogramRegionFeatureExtractor(int K)
	{
		if(K<=0 || K%2 == 0)
			throw feature_extractor_exception("K must be positive odd number");
		m_K = K;
	}
	CDoubleFeature CCorrelogramRegionFeatureExtractor::ExtractRegionFeature
							(const IImage& img, const IGrayImage& regionMap, unsigned int numOfRegion, const int id) const
	{
		_ASSERT(img.Height() == regionMap.Height() && img.Width() == regionMap.Width() );
		CDoubleFeature _fea(id, FeatureDim()*numOfRegion);

		int dis = (m_K-1)/2;

		//the number of pixels for every region, every quantized color 
		vector< vector<int> > hist(numOfRegion);
		for(unsigned int i=0; i<hist.size(); ++i)
			hist[i].resize(FeatureDim());
		///quantize the image color
		CGrayImage quantizeImage;
		quantizeImage.Allocate(img.Width(), img.Height());
		for(int y=0; y<img.Height(); ++y)
		{
			for(int x=0; x<img.Width(); ++x)
			{
				RgbTriple rgb = *(RgbTriple*)img.PixelPtr(x, y);
				*quantizeImage.PixelPtr(x, y) = QuantizeRgb36(rgb);
				hist[(*regionMap.PixelPtr(x, y))-1][*quantizeImage.PixelPtr(x, y)]++;
			}

		}

		//compute co-occurrence
		for(int y=0; y<img.Height(); ++y)
		{
			for(int x=0; x<img.Width(); ++x)
			{
				//up and down row
				if (y - dis >= 0)
				{
					for (int i = max(0, x - dis); i <= min(img.Width() - 1, x+dis); i++)
					{	
						if ( *regionMap.PixelPtr(x, y) == (*regionMap.PixelPtr(i, y-dis) ) &&
							 (*quantizeImage.PixelPtr(x, y) == *quantizeImage.PixelPtr(i, y-dis)) )
						{
							_fea[ ((*regionMap.PixelPtr(x,y))-1)*FeatureDim() + (*quantizeImage.PixelPtr(x, y))]++;
						}
					}
				}
				if (y + dis <img.Height())
				{
					for (int i = max(0, x - dis); i <= min(img.Width() - 1, x+dis); i++)
					{	
						if ( *regionMap.PixelPtr(x, y) == (*regionMap.PixelPtr(i, y+dis) ) &&
							 (*quantizeImage.PixelPtr(x, y) == *quantizeImage.PixelPtr(i, y+dis)) )
						{
							_fea[ ((*regionMap.PixelPtr(x,y))-1)*FeatureDim() + (*quantizeImage.PixelPtr(x, y))]++;
						}
					}
				}

				//left and right column
				if (x - dis >= 0)
				{
					for (int j = max(0, y - dis + 1) ; j <= min(img.Height()-1, y + dis - 1) ; j++)
					{
						if ( *regionMap.PixelPtr(x, y) == (*regionMap.PixelPtr(x-dis, j) ) &&
							 (*quantizeImage.PixelPtr(x, y) == *quantizeImage.PixelPtr(x-dis, j)) )
						{
							_fea[ ((*regionMap.PixelPtr(x,y))-1)*FeatureDim() + (*quantizeImage.PixelPtr(x, y))]++;
						}
					}
				}
				if (x + dis <img.Width())
				{
					for (int j = max(0, y - dis + 1) ; j <= min(img.Height()-1, y + dis - 1) ; j++)
					{
						if ( *regionMap.PixelPtr(x, y) == (*regionMap.PixelPtr(x+dis, j) ) &&
							 (*quantizeImage.PixelPtr(x, y) == *quantizeImage.PixelPtr(x+dis, j)) )
						{
							_fea[ ((*regionMap.PixelPtr(x,y))-1)*FeatureDim() + (*quantizeImage.PixelPtr(x, y))]++;
						}
					}
				}
			}
		}

		//normalization
		for(unsigned int i=0; i<numOfRegion; ++i)
			for(int j=0; j<FeatureDim(); ++j)
			{
				if(hist[i][j] > 0)
					_fea[i*FeatureDim() +j] /= hist[i][j]*(m_K-1)*4;
			}

		return _fea;
	}

	const int CCorrelogramRegionFeatureExtractor::FeatureDim() const
	{
		return FEATURE_DIM;
	}
#pragma endregion
}
