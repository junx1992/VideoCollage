// The change of this file should be updated to KeyframeExtraction
#include "stdafx.h"
#include "stdafx.h"
#include "RgbImage.h"
#include <iostream>
#include <fstream>
#include <windows.h>
#include <memory>
#include <gdiplus.h>
#include <list>
#include <algorithm>
#include <math.h>
using namespace std;
using namespace Gdiplus;
using namespace ImageAnalysis;

#define LHThreshold 10

inline BYTE dbl2byte(double v)
{
	int i = (int)(v + 0.4999); // 
	int V = max(0, min(255, i));
	return (BYTE)V;
}

void EnhanceImage(CRgbImage &image)
{
	//MedianFilter(image);
	// histogram
	int hist_b[256], hist_g[256], hist_r[256];
	memset(hist_b, 0, 256 * sizeof(int));
	memset(hist_g, 0, 256 * sizeof(int));
	memset(hist_r, 0, 256 * sizeof(int));
	int width = image.Width(), height = image.Height();
	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			const RgbTriple &bgr = image(j, i);
			++hist_b[bgr.b];
			++hist_g[bgr.g];
			++hist_r[bgr.r];
		}
	}

	// statistics
	const int nPixels = width * height;
	const double expose = 0.98;
	int M = 0, Mr = 0, Mg = 0, Mb = 0, mr = 255, mg = 255, mb = 255, m = 255;
	int nb = 0, ng = 0, nr = 0;
	for (int i = 0; i < 256; ++i) {
		if (nb < nPixels * expose) { Mb = i; nb += hist_b[i]; }
		if (ng < nPixels * expose) { Mg = i; ng += hist_g[i]; }
		if (nr < nPixels * expose) { Mr = i; nr += hist_r[i]; }
	}
	nb = 0; ng = 0; nr = 0;
	for (int i = 255; i >= 0; --i) {
		if (nb < nPixels * expose) { mb = i; nb += hist_b[i]; }
		if (ng < nPixels * expose) { mg = i; ng += hist_g[i]; }
		if (nr < nPixels * expose) { mr = i; nr += hist_r[i]; }
	} 
	M = max(Mb, max(Mg, Mr));
	m = min(mb, min(mg, mr));
	if (Mb - mb == 255 && Mg - mg == 255 && Mr - mr == 255 || M - m < 20)
		return; // do nothing

	// rashape
	M = dbl2byte(M + 50 * exp(-M / 50.0));
	Mb = dbl2byte(Mb + 50 * exp(-Mb / 50.0));
	Mg = dbl2byte(Mg + 50 * exp(-Mg / 50.0));
	Mr = dbl2byte(Mr + 50 * exp(-Mr / 50.0));
	m = dbl2byte(m - 50 * exp(-(255.0-m) / 50.0));
	mb = dbl2byte(mb - 50 * exp(-(255.0-mb) / 50.0));
	mg = dbl2byte(mg - 50 * exp(-(255.0-mg) / 50.0));
	mr = dbl2byte(mr - 50 * exp(-(255.0-mr) / 50.0));

	// transform: I = J*t + A(1-t)
	// parameter estimation
	if (Mb - mb <= LHThreshold) { Mb = M; mb = m; }
	if (Mg - mg <= LHThreshold) { Mg = M; mg = m; }
	if (Mr - mr <= LHThreshold) { Mr = M; mr = m; }
	//cout << '[' << m << ", " << M << ']' << endl;
	double t = (double)(M - m) / 255;
	double tb = (double)(Mb - mb) / 255;
	double tg = (double)(Mg - mg) / 255;
	double tr = (double)(Mr - mr) / 255;
	double A = (double)m / (255.001 + m - M) * 255;
	double Ab = (double)mb / (255.001 + mb - Mb) * 255;
	double Ag = (double)mg / (255.001 + mg - Mg) * 255;
	double Ar = (double)mr / (255.001 + mr - Mr) * 255;
	//cout << "t(" << t << "), A(" <<  A << ")\n";

	// inverse transform
	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			image.PixelPtr(j, i)[0] = dbl2byte(((double)image(j, i).b - Ab * (1-tb)) / tb);
			image.PixelPtr(j, i)[1] = dbl2byte(((double)image(j, i).g - Ag * (1-tg)) / tg);
			image.PixelPtr(j, i)[2] = dbl2byte(((double)image(j, i).r - Ar * (1-tr)) / tr);
		}
	}
	//image.Save(outfile);
}
