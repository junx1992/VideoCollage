#pragma once
#include "FeatureExtractorBase.h"
#include "RgbImage.h"
#include "iostream"
namespace ImageAnalysis
{
	class CDifferenceFeatureExtractor : public CFeatureExtractorForRgbImageBase
	{
	private:
		mutable CRgbImage *m_pPreImage, *m_pCurImage;
		mutable int m_PreId;
	public:
		CDifferenceFeatureExtractor() : m_PreId(-2), m_pCurImage(NULL), m_pPreImage(NULL)
		{
		}

		virtual CDoubleFeature Extract(const IRgbImage &img, const int id=UNDEFINED_FEATURE_ID) const
		{ // entrance
			//cout << id << endl;
			CDoubleFeature feature(id, 1, -1);
			if (id != m_PreId)
				Extract(feature, img);
			feature.Id(id - 1);  // id - 1 !!!! actually this difference score belongs to previous frame
			//feature.m_id--;
			return feature;
		}
		virtual void Extract(CDoubleFeature &feature, const IRgbImage &img) const
		{
			m_pCurImage = new CRgbImage;
			m_pCurImage->Copy(img);
			if (m_PreId + 1 == feature.Id()) 
				feature[0] = CalImageDiff(*m_pCurImage, *m_pPreImage) / (img.Width() * img.Height() * 3 * 255);
			delete m_pPreImage;
			m_pPreImage = m_pCurImage;
			m_pCurImage = NULL;
			m_PreId = feature.Id();
		}
		const int FeatureDim () const { return 1; }
		static double CalImageDiff(CRgbImage &a, CRgbImage &b)
		{
			//ofstream outFile("Value.txt");
			int height = a.Height(), width = a.Width();
			unsigned int s = 0;
			for (int y = 0; y < height; ++y) {
				for (int x = 0; x < width; ++x) {
					int v = CalPixelDiff(a(x, y), b(x, y));
					//outFile << v << '\t';
					s += v;
				}
				//outFile << endl;
			}
			//outFile.close();
			return s;
		}
		static inline int CalPixelDiff(const RgbTriple &a, const RgbTriple &b)
		{
			
			int v = abs((int)a.r - (int)b.r) + abs((int)a.g - (int)b.g) + abs((int)a.b - (int)b.b);
			return v;
		}
	};
}