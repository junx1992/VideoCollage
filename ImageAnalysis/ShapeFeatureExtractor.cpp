#include "StdAfx.h"
#include "ShapeFeatureExtractor.h"
#include <vector>
using std::vector;
#include <math.h>

namespace ImageAnalysis
{
	struct CRegionShape
	{
		POINT m_center;
		int m_radius;
		int m_numOfPixels;
		CRegionShape()
			:m_radius(0), m_numOfPixels(0)
		{
			m_center.x = 0;
			m_center.y = 0;
		}
	};

	const int CShapeFeatureExtractor::FeatureDim() const
	{
		return FEATURE_DIM;
	}

	CDoubleFeature CShapeFeatureExtractor::ExtractRegionFeature(const IImage& img, const IGrayImage& regionMap,unsigned int numOfRegion, const int id) const
	{
		_ASSERT(img.Height() == regionMap.Height() && img.Width() == regionMap.Width() );
		CDoubleFeature _fea(id, FeatureDim()*numOfRegion);

		vector<CRegionShape> _regionShapes(numOfRegion);
		//compute the statistical summary of the region
		for(int y=0; y<img.Height(); ++y)
		{
			for(int x=0; x<img.Width(); ++x)
			{
				int m = *regionMap.PixelPtr(x, y)-1;
				_regionShapes[m].m_center.x += x;
				_regionShapes[m].m_center.y += y;
				_regionShapes[m].m_numOfPixels++;
			}
		}
		for(unsigned int i=0; i<numOfRegion; ++i)
		{
			_regionShapes[i].m_center.x /= _regionShapes[i].m_numOfPixels;
			_regionShapes[i].m_center.y /= _regionShapes[i].m_numOfPixels;
		}

		//compute the radius and normalized inertai I(R_m, gamma)
		vector<double> I_Rm_1(numOfRegion, .0);
		vector<double> I_Rm_2(numOfRegion, .0);
		vector<double> I_Rm_3(numOfRegion, .0);

		for(int y=0; y<img.Height(); ++y)
		{
			for(int x=0; x<img.Width(); ++x)
			{
				int m = *regionMap.PixelPtr(x, y)-1;
				int radius = (y-_regionShapes[m].m_center.y)*(y-_regionShapes[m].m_center.y) + 
							    (x-_regionShapes[m].m_center.x)*(x-_regionShapes[m].m_center.x);
				if(radius > _regionShapes[m].m_radius)
					_regionShapes[m].m_radius = radius;

				//normalized inertia
				I_Rm_1[m] += pow((double)radius, 0.5);
				I_Rm_2[m] += pow((double)radius, 1);
				I_Rm_3[m] += pow((double)radius, 1.5);
			}
		}
		for(unsigned int m=0; m<numOfRegion; ++m)
		{
			_regionShapes[m].m_radius = (int)sqrt((double)_regionShapes[m].m_radius);
			_ASSERT(_regionShapes[m].m_radius<=sqrt((double)(img.Width()*img.Width()+img.Height()*img.Height())) );
			
			I_Rm_1[m] /= pow(_regionShapes[m].m_numOfPixels, 1.0 + 0.5 );
			I_Rm_2[m] /= pow(_regionShapes[m].m_numOfPixels, 1.0 + 1.0 );
			I_Rm_3[m] /= pow(_regionShapes[m].m_numOfPixels, 1.0 + 1.5 );
		}

		//compute the shape feature based on normalized inertia
		vector<double> I_1(numOfRegion, .0);
		vector<double> I_2(numOfRegion, .0);
		vector<double> I_3(numOfRegion, .0);
		vector<int> numOfPixels(numOfRegion, 0);
		for(unsigned int m=0; m<numOfRegion; ++m)
		{
			for(int y=-_regionShapes[m].m_radius; y<_regionShapes[m].m_radius; ++y)
			{
				for(int x=-_regionShapes[m].m_radius; x<_regionShapes[m].m_radius; ++x)
				{
					//compute I_1, I_2, I_3
					int tmp1 = x*x+y*y;
					if(tmp1 < _regionShapes[m].m_radius * _regionShapes[m].m_radius)
					{
						I_1[m] += pow( tmp1, 1.0/2.0);
						I_2[m] += pow( tmp1, 2.0/2.0);
						I_3[m] += pow( tmp1, 3.0/2.0);
						numOfPixels[m]++;
					}
				}
			}
			_ASSERT( numOfPixels[m] > 0);
			I_1[m] /= pow( numOfPixels[m], 1.0+1.0/2.0);
			I_2[m] /= pow( numOfPixels[m], 1.0+2.0/2.0);
			I_3[m] /= pow( numOfPixels[m], 1.0+3.0/2.0);

			I_Rm_1[m] /= I_1[m];
			I_Rm_2[m] /= I_2[m];
			I_Rm_3[m] /= I_3[m];
		}
		_ASSERT(FeatureDim() == 3);
		for(unsigned int m=0; m<numOfRegion; ++m)
		{
			_fea[m*FeatureDim()]   = I_Rm_1[m];
			_fea[m*FeatureDim()+1] = I_Rm_2[m];
			_fea[m*FeatureDim()+2] = I_Rm_3[m];
		}
		return _fea;
	}
}