#include "FeatureExtractorHelper.h"
#include <windows.h>
#include "canny.h"
#include "FaceDetectorDLL.h"
#include "GraySHow.h"
#include "nrutil.h"
#include <math.h>
#include <crtdbg.h>

namespace ImageAnalysisHelper
{
	namespace FeatureExtractorHelper
	{
	#pragma region Correlagram feature extractor helper function
		int count_around(BYTE *img, int w, int h, int x0, int y0, int k)
		{
			int n = 0;
			int x, line_y;
			BYTE p0 = img[y0 * w + x0];

			//上面一行
			if (y0 - k >= 0)
			{
				line_y = (y0 - k) * w;
				for (x = max(0, x0 - k); x <= min(w - 1, x0 + k); x++)
					if (img[line_y + x] == p0)
						n++;
			}

			//下面一行
			if (y0 + k < h)
			{
				line_y = (y0 + k) * w;
				for (x = max(0, x0 - k); x <= min(w - 1, x0 + k); x++)
					if (img[line_y + x] == p0)
						n++;
			}

			//左面一行
			if (x0 - k >= 0)
			{
				x = x0 - k;
				for (line_y = max(0, y0 - k + 1) * w; line_y <= min(h - 1, y0 + k - 1) * w; line_y += w)
					if (img[line_y + x] == p0)
						n++;
			}

			//右面一行
			if (x0 + k < w)
			{
				x = x0 + k;
				for (line_y = max(0, y0 - k + 1) * w; line_y <= min(h - 1, y0 + k - 1) * w; line_y += w)
					if (img[line_y + x] == p0)
						n++;
			}

			return n;
		}
	
		void CalcAutocorrelogram(BYTE *img, int w, int h, int m, double *autoc, double *hist, int k)
		{
			int x, y, line_y;
			int p, n;
			memset(autoc, 0, sizeof(double) * m);
			memset(hist, 0, sizeof(double) * m);
			for (y = 0, line_y = 0; y < h; y++, line_y += w)
				for (x = 0; x < w; x++)
				{
					p = img[line_y + x];
					n = count_around(img, w, h, x, y, k);
					autoc[p] += n;
					hist[p]++;
				}
			for (x = 0; x < m; x++)
				if (hist[x] > 0)
					autoc[x] /= (hist[x] * 8 * k);
		}

		int vq_hsv36(double h, double s, double v)
		{
			int ih, is, iv, q;

			if (v <= 0.2)
				return 0;		// [0]
			
			if (s <= 0.2)
			{
				if (v >= 0.8)
					return 7;	// [7]
				q = (int)floor( (v - 0.2f) * 10 ) + 1;	// [1..6]
				_ASSERT(q < 36);
				return q;
			}

			if ((s > 0.2 ) && (s <= 0.65))
				is = 0;
			if ((s > 0.65 ) && (s <= 1))
				is = 1;
			if ((v > 0.2 ) && (v <= 0.7))
				iv = 0;
			if ((v > 0.7 ) && (v <= 1))
				iv = 1;

		//	int a[7] = {22, 45, 70, 155, 186, 278, 330};
			if (((h > 330) && (h <= 360)) || (h <= 22))
				ih = 0;
			if (( h > 22) && (h <=45))
				ih = 1;
			if (( h > 45) && (h <=70))
				ih = 2;
			if (( h > 70) && (h <=155))
				ih = 3;
			if (( h > 155) && (h <=186))
				ih = 4;
			if (( h > 186) && (h <=278))
				ih = 5;
			if (( h > 278) && (h <=330))
				ih = 6;
			q = ih * 4 + is * 2 + iv + 8;		// 8..35
			_ASSERT(q < 36);
			return q;
		}

		void RgbToHsv(BYTE r, BYTE g, BYTE b, double &h, double &s, double &v)
		{
			v = double(__max(r, __max(g, b)));
			
			double m = double(__min(r, __min(g, b)));
			
			if (int(v) == 0)
				s = 0;
			else
				s = (v - m) / v;

			if ((v - m) !=0)
			{
				double r1, g1, b1;
				
				r1 = (v - r) / (v - m);
				b1 = (v - b) / (v - m);
				g1 = (v - g) / (v - m);

				int iv = int(v), im = int(m);
				if (iv == r && im == g)
					h = 5 + b1;
				else if (iv == r && im != g)
					h = 1 - g1;
				else if (iv == g && im == b)
					h = 1 + r1;
				else if (iv == g && im != b)
					h = 3 - b1;
				else if (iv == b && im == r)
					h = 3 + g1;
				else if (iv == b && im != r)
					h = 5 - r1;

				h = h * 60;
				while (h >= 360) 
					h = h - 360;
				//Modified by pei;
				v = v / 255;	// Remodified by ZL, 1999.4.26, because v / 255 may be equal to 1
								// then HsvToQuantize(..) will return 166, which is illegal!
			}
			else
			{
				h = s = 0;
				v = v / 255;
			}//h = -1;
		}

		int QuantizeRgb(BYTE r, BYTE g, BYTE b, UINT nColorNum)
		{
			int index = 0;
			if (nColorNum == 64)
				index = (((r>>6)<<4) | ((g>>6)<<2) | ((b>>6)));
			else if (nColorNum == 36)
			{
				double h, s, v;
				RgbToHsv(r, g, b, h, s, v);
				index = vq_hsv36(h, s, v);
			}
			else
				_ASSERT(FALSE);
			return index;
		}

		int ExtractCorrelagramFeature(const IImage* pImage, UINT nColorNum,  double *pFeature)
		{
			double *fauto = pFeature;
			int w, h;
			w = pImage->Width();
			h = pImage->Height();
			BYTE *data = new BYTE[w * h];
			
			int x, y, index;
			for( y=0, index = 0; y<h; y++)
			{
				for( x=0; x<w; x++, index++ )
				{
					BYTE *pixel = (BYTE*) pImage->PixelPtr(x, y);
					data[index] = QuantizeRgb(*(pixel + 2), *(pixel + 1), *(pixel + 0), nColorNum);
				}
			}
			
			double *ch = new double[nColorNum];
			int m;
			m = nColorNum;
			CalcAutocorrelogram(data, w, h, m, 
				fauto + 0 * m, ch, 1);
			CalcAutocorrelogram(data, w, h, m, 
				fauto + 1 * m, ch, 3);
			CalcAutocorrelogram(data, w, h, m, 
				fauto + 2 * m, ch, 5);
			CalcAutocorrelogram(data, w, h, m, 
				fauto + 3 * m, ch, 7);
			delete []data;
			delete []ch;
			return 1;
		}
	#pragma endregion

	#pragma region EDH feature extractor helper function
		void hist(int w, int h, double high, double low, BYTE * data, double * histogram, int bnum, int mark)
		{
			int i, j, t;
			int num = 0;
			double * bin = new double[bnum];
			for(i = 0; i < bnum; i++)
				bin[i] = low + (high - low) * i / bnum;
			memset(histogram, 0, bnum * sizeof(double));
			for(i = 0; i < h; i++)
				for(j = 0; j < w; j++)
				{
					for(t = 0; t < bnum - 1; t++)
					{
						if((data[i * w + j] >= bin[t]) && (data[i * w + j] < bin[t + 1]))
						{
							histogram[t] ++;
							num ++;
							break;
						}
					}
					if((data[i * w + j] >= bin[bnum - 1]) && (data[i * w + j] <= high))
					{
						histogram[bnum - 1] ++;
						num ++;
					}
				}
			if(mark && num)
				for(i = 0; i < bnum; i++)
					histogram[i] /= num;

			delete[] bin;
		}

		int ExtractEDHFeature(const IImage* pImage, UINT nFeatureDim, UINT bnum, double *pFeature)
		{
			double * f = pFeature;
			int h = pImage->Height();
			int w = pImage->Width();
			BYTE * r = new BYTE[w * h];
			BYTE * g = new BYTE[w * h];
			BYTE * b = new BYTE[w * h];
			BYTE * edge_r = new BYTE[w * h];
			BYTE * edge_g = new BYTE[w * h];
			BYTE * edge_b = new BYTE[w * h];
			BYTE * mag = new BYTE[w * h];
			BYTE * ori = new BYTE[w * h];
			memset(f, 0, nFeatureDim * sizeof(double));

			int i, j;
			for(i = 0; i < h; i++)
				for(j = 0; j < w; j++)
				{
					BYTE * pixel = (BYTE*)pImage->PixelPtr(j, i);
					r[i * w + j] = *(pixel + 2);
					g[i * w + j] = *(pixel + 1);
					b[i * w + j] = *pixel;
				}

			for(int scale = 0; scale < 3; scale++)
			{
				memset(edge_r, 0, w * h);
				memset(edge_b, 0, w * h);
				memset(edge_g, 0, w * h);
				canny_core(1 * pow(2.0f, scale), w, h, r, edge_r, mag, ori);
				canny_core(1 * pow(2.0f, scale), w, h, g, edge_g, mag, ori);
				canny_core(1 * pow(2.0f, scale), w, h, b, edge_b, mag, ori);
				for(i = 0; i < h; i++)
					for(j = 0; j < w; j++)
					{
						edge_r[i * w + j] = max(edge_r[i * w + j], edge_g[i * w + j]);
						edge_r[i * w + j] = max(edge_r[i * w + j], edge_b[i * w + j]);				
					}
				hist(w, h, 255.0, 128.0, edge_r, f + scale * bnum, bnum, 1);
			}

			delete[] r;
			delete[] g;
			delete[] b;
			delete[] edge_r;
			delete[] edge_g;
			delete[] edge_b;
			delete[] mag;
			delete[] ori;
			return 1;
		}
	#pragma endregion

	#pragma region Face extraction helper function
		HRESULT FaceExtract(const IImage* pImage, const int iDim, double *pFeature)
		{
			HRESULT hr = S_OK;
			CFaceDetectorDLL theDetector;
			double dArea = 0;
			int iCount = 0;
			FaceRect* pRects = NULL;

			double dFaceSizes[500];
			int	nLargestFaceIndex;
			double dLargestFaceSize;
			int i;

			theDetector.Init();

			BYTE* pImageBuff = const_cast<BYTE*>(pImage->ImageOrigin());
			hr = theDetector.DetectFace(pImage->Width(), pImage->Height(), pImageBuff,
				0, 3, 1, TRUE, NULL, &iCount, &pRects);

			if (SUCCEEDED(hr))
			{
				for (int i = 0; i < iCount; i++)
				{
					dArea += (pRects[i].rBox.bottom - pRects[i].rBox.top) * (pRects[i].rBox.right - pRects[i].rBox.left);
				}

		        
			}

			for(i = 0; i < iCount; i ++)
			{
				dFaceSizes[i] = (pRects[i].rBox.bottom - pRects[i].rBox.top) * (pRects[i].rBox.right - pRects[i].rBox.left);
			}

			dArea /= pImage->Width() * pImage->Height();

			if(iCount == 0)
			{
				pFeature[0] = 0.;
				pFeature[1] = 0.;
				pFeature[2] = 0.;
				pFeature[3] = 0.;
				pFeature[4] = 0.;
				pFeature[5] = 0.;
				pFeature[6] = 0.;
			}
			else
			{
				nLargestFaceIndex = 0;
				dLargestFaceSize = dFaceSizes[0];
				for(i = 0; i < iCount; i ++)
				{
					if(dFaceSizes[i] > dLargestFaceSize)
					{
						dLargestFaceSize = dFaceSizes[i];
						nLargestFaceIndex = i;
					}
				}
				pFeature[0] = (double)iCount;
				pFeature[1] = dArea;
				pFeature[2] = dLargestFaceSize/(pImage->Width()*pImage->Height() );
				pFeature[3] = (double)pRects[nLargestFaceIndex].rBox.bottom/(pImage->Height()+0.0);
				pFeature[4] = (double)pRects[nLargestFaceIndex].rBox.top/(pImage->Height()+0.0);
				pFeature[5] = (double)pRects[nLargestFaceIndex].rBox.right/(pImage->Width()+0.0);
				pFeature[6] = (double)pRects[nLargestFaceIndex].rBox.left/(pImage->Width()+0.0);
			}
			
			theDetector.FreeFaceRect(pRects);

			return hr;
		}
	#pragma endregion

	#pragma region Co-ocurrence feature extractor helper function
		HRESULT CoocurrenceExtract(const IImage* pImage, double *pFeature )
		{
			HRESULT hr = S_OK;

			double dEnergy[4]			  = {0.0, 0.0, 0.0, 0.0};
			double dEntropy[4]			  = {0.0, 0.0, 0.0, 0.0};
			double dInertiaQuadrature[4]  = {0.0, 0.0, 0.0, 0.0};
			double dLocalCalm[4]		  = {0.0, 0.0, 0.0, 0.0};
		//	double dCorrelation[4]		  = {0.0, 0.0, 0.0, 0.0};
			double dEnergy1[4]			  = {0.0, 0.0, 0.0, 0.0};
			double dEntropy1[4]		      = {0.0, 0.0, 0.0, 0.0};
			double dInertiaQuadrature1[4] = {0.0, 0.0, 0.0, 0.0};
			double dLocalCalm1[4]		  = {0.0, 0.0, 0.0, 0.0};
			double dCorrelation1[4]	      = {0.0, 0.0, 0.0, 0.0};
			CGrayShow grayShow;
			grayShow.LoadImage(pImage->ImageOrigin(), pImage->Width(), pImage->Height(), pImage->Stride());
			unsigned char** arLocalImage;
			arLocalImage = cmatrix(0, grayShow.FilterWindowWidth-1, 0, grayShow.FilterWindowWidth-1);
			int rolltimeH = grayShow.ImageHeight/grayShow.FilterWindowWidth;
			int rolltimeW = grayShow.ImageWidth /grayShow.FilterWindowWidth;
			int i, j, k;
			int p,q;

			//将图像分成若干个窗口，计算其纹理均值
			for(i=0; i< rolltimeH; i++)
			{
				for(j=0; j<rolltimeW; j++)
				{
					//首先赋值给子窗口
					for(p=0; p<grayShow.FilterWindowWidth; p++)
					{
						for(q=0; q<grayShow.FilterWindowWidth; q++)
						{
							arLocalImage[p][q] = grayShow.ImageArray[i*grayShow.FilterWindowWidth+p][j*grayShow.FilterWindowWidth+q];
						}
					}
					grayShow.ComputeMatrix(arLocalImage, grayShow.FilterWindowWidth);
					grayShow.ComputeFeature(dEnergy1[0], dEntropy1[0], dInertiaQuadrature1[0], dCorrelation1[0], dLocalCalm1[0], grayShow.PMatrixH, grayShow.GrayLayerNum);
					grayShow.ComputeFeature(dEnergy1[1], dEntropy1[1], dInertiaQuadrature1[1], dCorrelation1[1], dLocalCalm1[1], grayShow.PMatrixV, grayShow.GrayLayerNum);
					grayShow.ComputeFeature(dEnergy1[2], dEntropy1[2], dInertiaQuadrature1[2], dCorrelation1[2], dLocalCalm1[2], grayShow.PMatrixRD, grayShow.GrayLayerNum);
					grayShow.ComputeFeature(dEnergy1[3], dEntropy1[3], dInertiaQuadrature1[3], dCorrelation1[3], dLocalCalm1[3], grayShow.PMatrixLD, grayShow.GrayLayerNum);
					for (k=0; k<4; k++)
					{
						dEnergy[k]              += dEnergy1[k];
						dEntropy[k]             += dEntropy1[k];
						dInertiaQuadrature[k]   += dInertiaQuadrature1[k];
						dLocalCalm[k]           += dLocalCalm1[k];
					}
				}
			}
			
			for (i=0; i<4; i++)
			{
				dEnergy[i]              /= (rolltimeH*rolltimeW);
				dEntropy[i]             /= (rolltimeH*rolltimeW);
				dInertiaQuadrature[i]   /= (rolltimeH*rolltimeW);
				dLocalCalm[i]           /= (rolltimeH*rolltimeW);
				pFeature[0+i] = dEnergy[i];
				pFeature[4+i] = dEntropy[i];
				pFeature[8+i] = dInertiaQuadrature[i];
				pFeature[12+i] = dLocalCalm[i];
			}

			free_cmatrix(arLocalImage, 0, grayShow.FilterWindowWidth-1, 0, grayShow.FilterWindowWidth-1);

			return hr;
		}
	#pragma endregion
	};
};