#include "stdafx.h"
#include "ColorMomentFeatureExtractor.h"
#include <atlbase.h>
#include "VxFeature.h"
#include "VxFeatureHelper.h"
#include "FeatureExtractorHelperUsingFECom.h"
#include "ImageProcess.h"
#include <math.h>
#include <float.h>
#include <cassert>


namespace ImageAnalysis
{
    using namespace VxCore;
#pragma region  CColorLabMoment123FeatureExtractor
	void CColorLabMoment123FeatureExtractor::Extract(CDoubleFeature& feature, const IRgbImage& img) const
	{
		ATLASSERT(feature.Size() == FEATURE_DIM);
		CFloatFeature _float_feature(UNDEFINED_FEATURE_ID, FEATURE_DIM);
		Extract(_float_feature, img);
		VxCore::CopyFeatureData(feature, _float_feature);
	}
	CDoubleFeature CColorLabMoment123FeatureExtractor::Extract(const IRgbImage& img, const int id) const
	{
		return ImageAnalysisHelper::ExtractImageFeature(this, img, id);
	}

	void CColorLabMoment123FeatureExtractor::Extract(CFloatFeature& feature, const IRgbImage& img) const
	{
        feature.Resize(this->FeatureDim());
		ATLASSERT(feature.Size() == FEATURE_DIM);
		return Extract(feature.DataPtr(), img);
	}

	void CColorLabMoment123FeatureExtractor::Extract(float* pFeature, const IRgbImage& img) const
	{
		int height= img.Height();
		int width = img.Width();
		
		BITMAPINFOHEADER pbmi;
		const BYTE* pbData = img.ImageOrigin();

		pbmi.biHeight = height;
		pbmi.biWidth = width;

		pbmi.biBitCount = 24;
		pbmi.biPlanes = 1;
		pbmi.biCompression = 0;
		pbmi.biSize = sizeof(BITMAPINFOHEADER);

		EXCEPTION_IF_FAIL((*m_FECom)->ContainDIB(&pbmi, pbData, NULL), feature_extractor_exception, "can't call feature extractor com dll");
		EXCEPTION_IF_FAIL((*m_FECom)->Extract(FTYPE_ColorLabMoment123, norVectorLength, pFeature), feature_extractor_exception, "can't call feature extractor com dll");
		EXCEPTION_IF_FAIL((*m_FECom)->DeContainDIB(), feature_extractor_exception, "can't call feature extractor com dll");
		return;
	}

	const int CColorLabMoment123FeatureExtractor::FeatureDim() const
	{
		return FEATURE_DIM;
	}

	CDoubleFeature CColorLabMoment123FeatureExtractor::ExtractRegionFeature(const IImage& img, const IGrayImage& regionMap, unsigned int numOfRegion, const int id) const
	{
		_ASSERT(img.Height() == regionMap.Height() && img.Width() == regionMap.Width() );
		CDoubleFeature _fea(id, numOfRegion*FEATURE_DIM);
		CFloatFeature luv;
		const IRgbImage* pRgbImg = dynamic_cast<const IRgbImage*>(&img);
        
        assert( pRgbImg != NULL );
        if( pRgbImg == NULL )
			throw feature_extractor_exception("the input should be rgb image");
        
        Rgb2Luv(luv, pRgbImg);

		CIntVector regionSizes(numOfRegion);
		// 1-order
		for(int i = 0; i < img.Height(); i++)
		{
			for(int j = 0; j < img.Width(); j ++)
			{
				int m = *(regionMap.PixelPtr(j, i)) - 1;
				
				_ASSERT( m>=0 && m<= (int)numOfRegion-1);
				
				++regionSizes[m];

				int luvOffset = (i*img.Width()+j)*3;
				_fea[m*FEATURE_DIM] += luv[luvOffset];
				_fea[m*FEATURE_DIM+3] += luv[luvOffset+1]; 
				_fea[m*FEATURE_DIM+6] += luv[luvOffset+2];
			}
		}

		

		for(unsigned int m = 0; m < numOfRegion; m++)
		{
			if (regionSizes[m] != 0) 
			{
				_fea[m*FEATURE_DIM] /= regionSizes[m];
				_fea[m*FEATURE_DIM+3] /= regionSizes[m];
				_fea[m*FEATURE_DIM+6] /= regionSizes[m];
			}
		}

		// 2 & 3-order
		for(int i = 0; i < img.Height(); i++)
		{
			for(int j = 0; j < img.Width(); j ++)
			{
				int m = *(regionMap.PixelPtr(j, i)) - 1;
				int luvOffset = (i*img.Width()+j)*3;

				// L color-component
				double LDeviation = luv[luvOffset] - _fea[m*FEATURE_DIM];
				_fea[m*FEATURE_DIM+1] += LDeviation * LDeviation;
				_fea[m*FEATURE_DIM+2] += LDeviation * LDeviation * LDeviation;
				
				// a color-component
				double UDeviation = luv[luvOffset+1] - _fea[m*FEATURE_DIM+3];
				_fea[m*FEATURE_DIM+4] += UDeviation * UDeviation;
				_fea[m*FEATURE_DIM+5] += UDeviation * UDeviation * UDeviation;
				
				// b color-component
				double VDeviation = luv[luvOffset+2] - _fea[m*FEATURE_DIM+6];
				_fea[m*FEATURE_DIM+7] += VDeviation * VDeviation;
				_fea[m*FEATURE_DIM+8] += VDeviation * VDeviation * VDeviation;
			}
		}

		for(unsigned int m = 0; m < numOfRegion; m++)
		{
			if (regionSizes[m] != 0) 
			{
				_ASSERT(_fea[m*FEATURE_DIM+1] >= 0);
				_fea[m*FEATURE_DIM+1] = pow(_fea[m*FEATURE_DIM+1]/regionSizes[m], 0.5 );

				double ONE_THIRDS = 1.0/3.0;

				if(_fea[m*FEATURE_DIM+2] > 0)
					_fea[m*FEATURE_DIM+2] = pow(_fea[m*FEATURE_DIM+2]/regionSizes[m], ONE_THIRDS );
				else
					_fea[m*FEATURE_DIM+2] = -pow(-_fea[m*FEATURE_DIM+2]/regionSizes[m], ONE_THIRDS );

				_ASSERT(_fea[m*FEATURE_DIM+4] >= 0);
				_fea[m*FEATURE_DIM+4] = pow(_fea[m*FEATURE_DIM+4]/regionSizes[m], 0.5 );

				if(_fea[m*FEATURE_DIM+5] > 0)
					_fea[m*FEATURE_DIM+5] = pow(_fea[m*FEATURE_DIM+5]/regionSizes[m], ONE_THIRDS );
				else
					_fea[m*FEATURE_DIM+5] = -pow(-_fea[m*FEATURE_DIM+5]/regionSizes[m], ONE_THIRDS );

				_ASSERT(_fea[m*FEATURE_DIM+7] >= 0);
				_fea[m*FEATURE_DIM+7] = pow(_fea[m*FEATURE_DIM+7]/regionSizes[m], 0.5 );
				
				if(_fea[m*FEATURE_DIM+8] > 0)
					_fea[m*FEATURE_DIM+8] = pow(_fea[m*FEATURE_DIM+8]/regionSizes[m], ONE_THIRDS );
				else
					_fea[m*FEATURE_DIM+8] = -pow(-_fea[m*FEATURE_DIM+8]/regionSizes[m], ONE_THIRDS );

				for(int i=0; i<FEATURE_DIM; ++i)
					_ASSERT( !_isnan(_fea[m*FEATURE_DIM+i]));
			}
		}
		return _fea;
	}
#pragma endregion

#pragma region  CBlockWiseColorLabMoment123FeatureExtractor
	void CBlockWiseColorLabMoment123FeatureExtractor::Extract(CDoubleFeature& feature, const IRgbImage& img) const
	{
        feature.Resize(this->FeatureDim());
		ATLASSERT(feature.Size() == FEATURE_DIM);
		CFloatFeature _float_feature(UNDEFINED_FEATURE_ID, FEATURE_DIM);
		Extract(_float_feature, img);
		VxCore::CopyFeatureData(feature, _float_feature);
	}
	CDoubleFeature CBlockWiseColorLabMoment123FeatureExtractor::Extract(const IRgbImage& img, const int id) const
	{
		return ImageAnalysisHelper::ExtractImageFeature(this, img, id);
	}

	void CBlockWiseColorLabMoment123FeatureExtractor::Extract(CFloatFeature& feature, const IRgbImage& img) const
	{
		const static int BLOCK_NUM = 5;
		ATLASSERT(feature.Size() == FEATURE_DIM);
		int height= img.Height();
		int width = img.Width();
		int localWidth = width / BLOCK_NUM;
		int localHeight = height / BLOCK_NUM;
		const IImageEx* pImgEx = dynamic_cast<const IImageEx*>(&img);

		assert( pImgEx != NULL );
		if( pImgEx == NULL )
            throw feature_extractor_exception("the imput IRgbImage* parameter should be able to be casted to IImageEx");

        IRgbImage* pSubImg = NULL;
		for(int i = 0, k=0; i<BLOCK_NUM; ++i)
		{
			for(int j=0; j<BLOCK_NUM; ++j)
			{
				if(NULL == pSubImg)
					pSubImg=  static_cast<IRgbImage*>(pImgEx->SubImage(RECTANGLE(i*localWidth, j*localHeight, 
														i*localWidth+localWidth, j*localHeight+localHeight) ) );
				else
				{
					IImageEx* pSubImgEx = dynamic_cast<IImageEx*>(pSubImg);
					if(pSubImgEx == NULL)
					{
						SAFE_RELEASE(pSubImg);
						pSubImg =  static_cast<IRgbImage*>(pImgEx->SubImage(RECTANGLE(i*localWidth, j*localHeight, 
														i*localWidth+localWidth, j*localHeight+localHeight) ) );
					}
					else
					{
						pSubImgEx->Copy(img, RECTANGLE(i*localWidth, j*localHeight, 
														i*localWidth+localWidth, j*localHeight+localHeight) );
					}
				}
				m_ColorMomentFE.Extract(feature.DataPtr() + k*m_ColorMomentFE.FeatureDim(), *pSubImg);
				++k;
				
			}
		}
		if(NULL != pSubImg)
			pSubImg->Release();
		return;
	}

	const int CBlockWiseColorLabMoment123FeatureExtractor::FeatureDim() const
	{
		return FEATURE_DIM;
	}
#pragma endregion
}
