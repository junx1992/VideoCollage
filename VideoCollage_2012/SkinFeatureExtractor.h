#pragma once
#include <Windows.h>
#include "FeatureExtractorBase.h"
#include <IImageFeatureExtractor.h>
#include <ColorHistogramFeatureExtractor.h>
#include <FaceFeatureExtractor.h>
#include <RgbImage.h>
#include <fstream>
#include <string>
#include <iostream>

#define ENABLE_SEGMENT				false
#define M_PI_OVER_3                 1.0471975511965977461542144610932
#define M_PI_TIMES_2                6.2831853071795864769252867665590
#define SIZEH 32
#define SIZES 32
#define CA_HSV_MAX_DARK_VALUE       0.125
#define g_histogram_num 7
#define SA_SKIN_PIXEL_THRESHOLD		80
#define SKIN_THRESHOLD				64
namespace ImageAnalysis
{
	class Segments
	{
	public:
		int n;
		int *label;
		Segments() : n(0), label(NULL)
		{
		}
		~Segments()
		{
			delete []label;
		}
	};
	using namespace std;
	class CSkinFeatureExtractor : public CFeatureExtractorForRgbImageBase
	{
			public:
		CSkinFeatureExtractor();
		~CSkinFeatureExtractor();
		virtual CDoubleFeature Extract(const IRgbImage &img, const int id=UNDEFINED_FEATURE_ID) const;
		double Extract(const IRgbImage &img, CRgbImage &outimage);
		virtual void Extract(CDoubleFeature &feature, const IRgbImage &img) const;

		virtual const int  FeatureDim() const;
	private:
		//
		// histogram for skin pixels
		//
		static double g_Histogram_Skin[g_histogram_num][SIZEH][SIZES];
		
		//
		// histogram for non-skin pixels
		//
		static double g_Histogram_NonSkin[SIZEH][SIZES];

		// statistics
		bool isEnableSegment;
		static const int FEATURE_DIM = 3;
		static const int clusternum = 1;
		static const int  histnum = 1;
		static const bool bSkinMap = false;


		const int LevelNum;
		const double UnitNum;
		double *pAdultScore;
		//int iSampleNum;
		bool m_bEnableROIDetection;
		bool m_bEnableFaceDetection;
		CFaceFeatureExtractor m_faceFeatureExtractor;
		inline void init_histograms(const char foldername[], int histnum)
		{
			//char skinhistfn[] = "D:\\Results\\AdultFiltering\\grouping\\training-skinhistogram\\Positive\\5-skin_histogram.txt";
			//char nonskinhistfn[] = "D:\\Results\\AdultFiltering\\grouping\\training-skinhistogram\\Negative\\1-nonskin_histogram.txt";
			
			char skinhistfn[MAX_PATH];// = "D:\\Projects\\AdultFiltering\\Labeling\\5-skin_histogram.txt";
			sprintf_s(skinhistfn, "%s%d-skin_histogram.txt", foldername, histnum);
			char nonskinhistfn[MAX_PATH]; //= "D:\\Projects\\AdultFiltering\\Labeling\\1-nonskin_histogram.txt";
			sprintf_s(nonskinhistfn, "%s1-nonskin_histogram.txt", foldername);

			load_skinhistogram(skinhistfn);
			load_nonskinhistogram(nonskinhistfn);
		}
		void load_skinhistogram(const char fn[])
		{
			using namespace std;
			ifstream fin(fn);
			if (!fin.good()) {
				cout << "No histogram file\n";
				return;
			}
			for (int i = 0; i < clusternum; ++i)
			{
				for (int h = 0; h < SIZEH; ++h)
				{
					for (int s = 0; s < SIZES; ++s)
					{
						fin >> g_Histogram_Skin[i][h][s];
						//cout << g_Histogram_Skin[i][h][s] << endl;
					}
				}
			}

			fin.close();
		}

		//
		// load histograms from files
		//
		void load_nonskinhistogram(const char fn[])
		{
			using namespace std;

			ifstream fin(fn);
			for (int h = 0; h < SIZEH; ++h)
			{
				for (int s = 0; s < SIZES; ++s)
				{
					fin >> g_Histogram_NonSkin[h][s];
				}
			}
			fin.close();
		}

		inline static BYTE Ratio2Byte(double ratio)
		{
			BYTE result = 0;
			if (ratio >= 1)
			{
				ratio = ratio - 1;
				if (ratio >= 5)	ratio = 5;
				result = static_cast<BYTE>(ratio * 190/5 + 64);
			}
			else {
				ratio = 1 - ratio;
				result = static_cast<BYTE>(64 - ratio * 64);
			}

			return result;
		}
		inline static bool RgbToHsv (const BYTE btR, const BYTE btG, const BYTE btB, DOUBLE &dH, DOUBLE &dS, DOUBLE &dV)
		{
			bool hr = true;

			do
			{
				BYTE btMax = (btR > btG)? btR : btG;
				if (btB > btMax) btMax = btB;

				BYTE btMin = (btR < btG)? btR : btG;
				if (btB < btMin) btMin = btB;

				// H
				if (btMax > btMin)
				{
					if (btR == btMax && btG == btMin)
					{
						dH = (5.0 + DOUBLE(btMax - btB) / (btMax - btMin)) * M_PI_OVER_3;
					}
					else if (btR == btMax && btG != btMin)
					{
						dH = (1.0  -  DOUBLE(btMax - btG) / (btMax - btMin)) * M_PI_OVER_3;
					}
					else if (btG == btMax && btB == btMin)
					{
						dH = (1.0 + DOUBLE(btMax - btR) / (btMax - btMin)) * M_PI_OVER_3;
					}
					else if (btG == btMax && btB != btMin)
					{
						dH = (3.0  -  DOUBLE(btMax - btB) / (btMax - btMin)) * M_PI_OVER_3;
					}
					else if (btB == btMax && btR == btMin)
					{
						dH = (3.0 + DOUBLE(btMax - btG) / (btMax - btMin)) * M_PI_OVER_3;
					}
					else
					{
						dH = (5  -  DOUBLE(btMax - btR) / (btMax - btMin)) * M_PI_OVER_3;
					}
				}
				else
				{
					dH = 0.0;
				}

				// S and V
				dS = (btMax != 0)? DOUBLE(btMax - btMin) / btMax : 0.0;
				dV = DOUBLE(btMax) / 255;
			}
			while (false);

			return hr;
		}
		inline static void EvaluateLikelihood(const BYTE R, const BYTE G, const BYTE B, DOUBLE dLikelihoodOnSkin[], DOUBLE &dLikelihoodOffSkin)
		{
			DOUBLE dH;
			DOUBLE dS; 
			DOUBLE dV;
			RgbToHsv(R, G, B, dH, dS, dV);
			DWORD dwH = 0;
			DWORD dwS = 0;
			if (dV > CA_HSV_MAX_DARK_VALUE)
			{
				dwH = DWORD(dH/M_PI_TIMES_2 * (SIZEH-1) + 0.5);
				dwS = DWORD(dS * (SIZES-1) + 0.5);
			}

			// get prob from the histograms
			get_prob(dwH, dwS, dLikelihoodOnSkin, dLikelihoodOffSkin);
		}
		inline static void get_prob(const int iH, const int iS, double *prob_skin, double &prob_nonskin)
		{
			for (int i = 0; i < clusternum; ++i)
			{
				prob_skin[i] = g_Histogram_Skin[i][iH][iS];
			}
			
			prob_nonskin = g_Histogram_NonSkin[iH][iS];
		}

		inline static void get_prob(const int iH, const int iS, double &prob_skin, double &prob_nonskin)
	{
		prob_skin = g_Histogram_Skin[0][iH][iS];
		for (int i = 1; i < clusternum; ++i)
		{
			if (prob_skin < g_Histogram_Skin[i][iH][iS])
			{
				prob_skin = g_Histogram_Skin[i][iH][iS];
			}
		}
		
		prob_nonskin = g_Histogram_NonSkin[iH][iS];
	}
		double detect_image_pixel(const IRgbImage &img, CRgbImage &skinimage, double &faceArea, double &n) const;
		int segment(const IRgbImage &input, Segments &output) const;
		CRgbImage SkinDetector(const IRgbImage &InImage) const;
		void SkinAdapter(const DOUBLE **ppLikelihoodOnSkin, const DOUBLE *pLikelihoodOffSkin, CRgbImage &SoftMap, int &iSkinType) const;
		//
		// Segment the image into skin and non-skin regions
		// Input: 
		//       pLikelihoodOnSkin: the likelihood that each pixel is on skin 
		//       pLikelihoodOffSkin: the likelihood taht each pixel is off skin
		//
		void SkinSegmentor(const DOUBLE *pLikelihoodOnSkin, const DOUBLE *pLikelihoodOffSkin, const CRgbImage &InImage, CRgbImage &SegmentMap) const;
		double AdultScore(const IRgbImage &softMap, const CDoubleFeature &face, const Segments &segment) const;
	};
}