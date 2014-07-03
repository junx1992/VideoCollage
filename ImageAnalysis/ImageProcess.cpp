#include "StdAfx.h"
#include "ImageProcess.h"
#include "atlbase.h"
#include "GrayImage.h"
#include "ImageAnalysisException.h"
#include <math.h>

namespace ImageAnalysis
{
#pragma region Rgb2Gray
	//*************************************************************************************************************
	//	Const variable for rgb to gray accelerate calculation
	//*************************************************************************************************************
	const unsigned char R2Y[256] =
	{
		0,	0,	0,	0,	1,	1,	1,	2,	2,	2,	2,	3,	3,	3,	4,	4,	
		4,	5,	5,	5,	5,	6,	6,	6,	7,	7,	7,	8,	8,	8,	8,	9,	
		9,	9,	10,	10,	10,	11,	11,	11,	11,	12,	12,	12,	13,	13,	13,	14,	
		14,	14,	14,	15,	15,	15,	16,	16,	16,	17,	17,	17,	17,	18,	18,	18,	
		19,	19,	19,	20,	20,	20,	20,	21,	21,	21,	22,	22,	22,	23,	23,	23,	
		23,	24,	24,	24,	25,	25,	25,	26,	26,	26,	26,	27,	27,	27,	28,	28,	
		28,	29,	29,	29,	29,	30,	30,	30,	31,	31,	31,	31,	32,	32,	32,	33,	
		33,	33,	34,	34,	34,	34,	35,	35,	35,	36,	36,	36,	37,	37,	37,	37,	
		38,	38,	38,	39,	39,	39,	40,	40,	40,	40,	41,	41,	41,	42,	42,	42,	
		43,	43,	43,	43,	44,	44,	44,	45,	45,	45,	46,	46,	46,	46,	47,	47,	
		47,	48,	48,	48,	49,	49,	49,	49,	50,	50,	50,	51,	51,	51,	52,	52,	
		52,	52,	53,	53,	53,	54,	54,	54,	55,	55,	55,	55,	56,	56,	56,	57,	
		57,	57,	58,	58,	58,	58,	59,	59,	59,	60,	60,	60,	60,	61,	61,	61,	
		62,	62,	62,	63,	63,	63,	63,	64,	64,	64,	65,	65,	65,	66,	66,	66,	
		66,	67,	67,	67,	68,	68,	68,	69,	69,	69,	69,	70,	70,	70,	71,	71,	
		71,	72,	72,	72,	72,	73,	73,	73,	74,	74,	74,	75,	75,	75,	75,	76
	};

	const unsigned char G2Y[256] =
	{
		0,	0,	1,	1,	2,	2,	3,	4,	4,	5,	5,	6,	7,	7,	8,	8,	
		9,	9,	10,	11,	11,	12,	12,	13,	14,	14,	15,	15,	16,	17,	17,	18,	
		18,	19,	19,	20,	21,	21,	22,	22,	23,	24,	24,	25,	25,	26,	27,	27,	
		28,	28,	29,	29,	30,	31,	31,	32,	32,	33,	34,	34,	35,	35,	36,	36,	
		37,	38,	38,	39,	39,	40,	41,	41,	42,	42,	43,	44,	44,	45,	45,	46,	
		46,	47,	48,	48,	49,	49,	50,	51,	51,	52,	52,	53,	54,	54,	55,	55,	
		56,	56,	57,	58,	58,	59,	59,	60,	61,	61,	62,	62,	63,	63,	64,	65,	
		65,	66,	66,	67,	68,	68,	69,	69,	70,	71,	71,	72,	72,	73,	73,	74,	
		75,	75,	76,	76,	77,	78,	78,	79,	79,	80,	81,	81,	82,	82,	83,	83,	
		84,	85,	85,	86,	86,	87,	88,	88,	89,	89,	90,	90,	91,	92,	92,	93,	
		93,	94,	95,	95,	96,	96,	97,	98,	98,	99,	99,	100,100,101,102,102,	
		103,103,104,105,105,106,106,107,108,108,109,109,110,110,111,112,	
		112,113,113,114,115,115,116,116,117,117,118,119,119,120,120,121,	
		122,122,123,123,124,125,125,126,126,127,127,128,129,129,130,130,	
		131,132,132,133,133,134,135,135,136,136,137,137,138,139,139,140,	
		140,141,142,142,143,143,144,144,145,146,146,147,147,148,149,149
	};

	const unsigned char B2Y[256] = 
	{
		0,	0,	0,	0,	0,	0,	0,	0,	0,	1,	1,	1,	1,	1,	1,	1,	
		1,	1,	2,	2,	2,	2,	2,	2,	2,	2,	2,	3,	3,	3,	3,	3,	
		3,	3,	3,	3,	4,	4,	4,	4,	4,	4,	4,	4,	5,	5,	5,	5,	
		5,	5,	5,	5,	5,	6,	6,	6,	6,	6,	6,	6,	6,	6,	7,	7,	
		7,	7,	7,	7,	7,	7,	7,	8,	8,	8,	8,	8,	8,	8,	8,	9,	
		9,	9,	9,	9,	9,	9,	9,	9,	10,	10,	10,	10,	10,	10,	10,	10,	
		10,	11,	11,	11,	11,	11,	11,	11,	11,	11,	12,	12,	12,	12,	12,	12,	
		12,	12,	12,	13,	13,	13,	13,	13,	13,	13,	13,	14,	14,	14,	14,	14,	
		14,	14,	14,	14,	15,	15,	15,	15,	15,	15,	15,	15,	15,	16,	16,	16,	
		16,	16,	16,	16,	16,	16,	17,	17,	17,	17,	17,	17,	17,	17,	18,	18,	
		18,	18,	18,	18,	18,	18,	18,	19,	19,	19,	19,	19,	19,	19,	19,	19,	
		20,	20,	20,	20,	20,	20,	20,	20,	20,	21,	21,	21,	21,	21,	21,	21,	
		21,	22,	22,	22,	22,	22,	22,	22,	22,	22,	23,	23,	23,	23,	23,	23,	
		23,	23,	23,	24,	24,	24,	24,	24,	24,	24,	24,	24,	25,	25,	25,	25,	
		25,	25,	25,	25,	25,	26,	26,	26,	26,	26,	26,	26,	26,	27,	27,	27,	
		27,	27,	27,	27,	27,	27,	28,	28,	28,	28,	28,	28,	28,	28,	28,	29
	};

	HRESULT Rgb2Gray(IGrayImage* dst, const IRgbImage* const src)
	{
		ATLASSERT(src != NULL && dst != NULL);
		int width = src->Width();
		int height = src->Height();
		if(dst->Width() != src->Width() || dst->Height() != src->Height() )
		{
			ATLASSERT(false);
			return E_INVALIDARG;
		}

		int stride_src = src->Stride();
		int stride_dst = dst->Stride();
		unsigned char *p = (unsigned char*)src->RowPtr(0);
		unsigned char *q = (unsigned char*)dst->RowPtr(0);
		for ( int y = 0; y < height; ++y, p += stride_src, q += stride_dst )
		{
			RgbTriple  *rgb  = (RgbTriple*)p;
			BYTE *gray = (BYTE*)q;
			for ( int x = 0; x < width; ++x, ++gray, ++rgb )
			{
				unsigned char r = rgb->r;
				unsigned char g = rgb->g;
				unsigned char b = rgb->b;	
				*gray = B2Y[b] + G2Y[g] + R2Y[r];
			}
		}
		return S_OK;
	}

	IGrayImage* Rgb2Gray(const IRgbImage* const src)
	{
		ATLASSERT(src != NULL);
		CGrayImage* pRet = new CGrayImage();
		pRet->Allocate(src->Width(), src->Height());
		if(FAILED(Rgb2Gray(pRet, src)))
			return NULL;
		return pRet;
	}
#pragma endregion

#pragma region Brightness of image
	BYTE Brightness(const IRgbImage* const src)
	{
		ATLASSERT(src != NULL);
		int w = src->Width();
		int h = src->Height();
		long sum = 0;
		for(int i=0; i<h; ++i)
		{
			const RgbTriple* p = (const RgbTriple*)src->RowPtr(i);
			for(int j=0; j<w; ++j)
			{
				const RgbTriple  *rgb  = p++;
				unsigned char r = rgb->r;
				unsigned char g = rgb->g;
				unsigned char b = rgb->b;	
				sum += B2Y[b] + G2Y[g] + R2Y[r];
			}
		}
		return (BYTE)(sum/w/h);
	}

	BYTE Brightness(const IGrayImage* const src)
	{
		ATLASSERT(src != NULL);
		int w = src->Width();
		int h = src->Height();
		long sum = 0;
		for(int i=0; i<h; ++i)
		{
			const BYTE* p = src->RowPtr(i);
			for(int j=0; j<w; ++j)
				sum += (long) (*(p+j));
		}
		return (BYTE)(sum/w/h);
	}

	BYTE Brightness(const IImage* const src)
	{
		ATLASSERT(src != NULL);
		const IRgbImage* const pRgbImage  = dynamic_cast<const IRgbImage* const>(src);
		if(NULL != pRgbImage)
			return Brightness(pRgbImage);
		const IGrayImage* const pGrayImage  = dynamic_cast<const IGrayImage* const>(src);
		if(NULL != pGrayImage)
			return Brightness(pGrayImage);

       throw image_process_exception("not supported image interface");
	}
#pragma endregion
	
	void Rgb2Luv(CFloatVector& luv, const IRgbImage* const src)
	{
		ATLASSERT(src != NULL);

		if(luv.Size() != src->Width() * src->Height() *3)
		{
			luv.Resize(src->Width() * src->Height() *3);
		}

		double X0 = (0.607+0.174+0.201);
		double Y0 = (0.299+0.587+0.114);
		double Z0 = (      0.066+1.117);

		/* Y0 = 1.0 */
		double u20 = 4*X0/(X0+15*Y0+3*Z0);
		double v20 = 9*Y0/(X0+15*Y0+3*Z0);

		double r, g, b, x, y, X, Y, Z, den, u2,v2;
		for (int i=0;i<src->Height();i++)
			for(int j=0; j<src->Width(); j++)
			{
				r = (* (RgbTriple*) src->PixelPtr(j, i)).r;
				g = (* (RgbTriple*) src->PixelPtr(j, i)).g;
				b = (* (RgbTriple*) src->PixelPtr(j, i)).b;

				if (r<=20)  r=(double) (8.715e-4* r);
				else r=(double) pow((r+25.245)/280.245, 2.22);

				if (g<=20)  g=(double) (8.715e-4*g);
				else g=(double) pow((g+25.245)/280.245, 2.22);

				if (b<=20)  b=(double) (8.715e-4*b);
				else b=(double) pow((b+25.245)/280.245, 2.22);

				X = 0.412453*r + 0.357580*g + 0.180423*b;
				Y = 0.212671*r + 0.715160*g + 0.072169*b;
				Z = 0.019334*r + 0.119193*g + 0.950227*b;

				if (X==0.0 && Y==0.0 && Z==0.0)
				{
					x=1.0/3.0; y=1.0/3.0;
				}
				else
				{
					den=X+Y+Z;
					x=X/den; y=Y/den;
				}

				den=-2*x+12*y+3;
				u2=4*x/den;
				v2=9*y/den;

				int luvOffset = (i*src->Width()+j)*3;
				if (Y>0.008856) luv[luvOffset] = (float) (116*pow(Y,1.0/3.0)-16);
				else luv[luvOffset] = (float) (903.3*Y);
				luv[luvOffset+1] = (float) (13*luv[luvOffset]*(u2-u20));
				luv[luvOffset+2] = (float) (13*luv[luvOffset]*(v2-v20));
			}
	}

	void RgbToHsv(HsvTriple& hsv, const RgbTriple& rgb)
	{
		hsv.v = (float) (max(rgb.r, max(rgb.g, rgb.b)));
		
		float m = (float) (min(rgb.r, min(rgb.g, rgb.b)));
		
		if ((hsv.v) == 0)
			hsv.s = 0;
		else
			hsv.s = (hsv.v - m) / hsv.v;

		if ((hsv.v - m) !=0)
		{
			
			float r1 = (hsv.v - rgb.r) / (hsv.v - m);
			float b1 = (hsv.v - rgb.b) / (hsv.v - m);
			float g1 = (hsv.v - rgb.g) / (hsv.v - m);

			int iv = (int)(hsv.v), im = (int)(m);
			if (iv == rgb.r && im == rgb.g)
				hsv.h = 5 + b1;
			else if (iv == rgb.r && im != rgb.g)
				hsv.h = 1 - g1;
			else if (iv == rgb.g && im == rgb.b)
				hsv.h = 1 + r1;
			else if (iv == rgb.g && im != rgb.b)
				hsv.h = 3 - b1;
			else if (iv == rgb.b && im == rgb.r)
				hsv.h = 3 + g1;
			else if (iv == rgb.b && im != rgb.r)
				hsv.h = 5 - r1;

			hsv.h = hsv.h * 60;
			while (hsv.h >= 360) 
				hsv.h = hsv.h - 360;
			//Modified by pei;
			hsv.v = hsv.v / 255;	// Remodified by ZL, 1999.4.26, because v / 255 may be equal to 1
							// then HsvToQuantize(..) will return 166, which is illegal!
		}
		else
		{
			hsv.h = hsv.s = 0;
			hsv.v = hsv.v / 255;
		}//h = -1;
	}

	int QuantizeHsv36(const HsvTriple& hsv)
	{
		float h = hsv.h, s = hsv.s, v = hsv.v;
		if (v <= 0.2)
			return 0;		// [0]
		
		if (s <= 0.2)
		{
			if (v >= 0.8)
				return 7;	// [7]
			int q = (int)floor( (v - 0.2f) * 10 ) + 1;	// [1..6]
			_ASSERT(q < 36);
			return q;
		}

		int ih, is, iv;
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
		int q = ih * 4 + is * 2 + iv + 8;		// 8..35
		_ASSERT(q < 36);
		return q;
	}
}