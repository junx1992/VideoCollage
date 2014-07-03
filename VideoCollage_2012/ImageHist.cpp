// ImageHist.cpp: implementation of the CImageHist class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ImageHist.h"
#include "math.h"
#include ".\imagehist.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define DEBUG_NEW new(THIS_FILE, __LINE__)
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CImageHist::CImageHist(IplImage *pImage)
{
	if ((pImage->nChannels != 3) || (NULL == pImage))
	{
		printf("\nError: in CImageHist, image must be 3 component!\n");
		exit(0);
	}

	m_flHEntropy = 0.0f;
	m_flSEntropy = 0.0f;
	m_flVEntropy = 0.0f;
	m_pHHist = NULL;
	m_pSHist = NULL;
	m_pVHist = NULL;

	m_nHeight = pImage->height;
	m_nWidth = pImage->width;

	m_pHSVImage = cvCreateImage(cvSize(m_nWidth, m_nHeight), IPL_DEPTH_8U, 3);
	m_pHImage = cvCreateImage(cvSize(m_nWidth, m_nHeight), IPL_DEPTH_8U, 1);
	m_pSImage = cvCreateImage(cvSize(m_nWidth, m_nHeight), IPL_DEPTH_8U, 1);
	m_pVImage = cvCreateImage(cvSize(m_nWidth, m_nHeight), IPL_DEPTH_8U, 1);

#ifdef RGB_TO_HSV
	cvCvtColor(pImage, m_pHSVImage, CV_RGB2HSV);
#endif

#ifdef BGR_TO_HSV
	cvCvtColor(pImage, m_pHSVImage, CV_BGR2HSV);
#endif

	cvSplit(m_pHSVImage, m_pHImage, m_pSImage, m_pVImage, NULL);

	/************************************************************************/
	/* Calc Image HSV Histogram
	/************************************************************************/
	CalcHueHist();
	CalcSatHist();
	CalcValHist();

}

CImageHist::~CImageHist()
{
	if (NULL != m_pHImage)
	{
		cvReleaseImage(&m_pHImage); m_pHImage = NULL;
	}

	if (NULL != m_pSImage)
	{
		cvReleaseImage(&m_pSImage); m_pSImage = NULL;
	}

	if (NULL != m_pVImage)
	{
		cvReleaseImage(&m_pVImage); m_pVImage = NULL;
	}

	if (NULL != m_pHSVImage)
	{
		cvReleaseImage(&m_pHSVImage); m_pHSVImage = NULL;
	}

	if (NULL != m_pHHist)
	{
		delete[] m_pHHist; m_pHHist = NULL;
	}

	if (NULL != m_pVHist)
	{
		delete[] m_pVHist; m_pVHist = NULL;
	}

	if (NULL != m_pSHist)
	{
		delete[] m_pSHist; m_pSHist = NULL;
	}

}

void CImageHist::CalcHueHist()
{
	IplImage* h_plane[] = { m_pHImage };
	int h_hist_size[] = { H_HIST_BIN };
	float h_range[] = { 0, 180 };
	float* h_ranges[] = { h_range };

	CvHistogram* h_hist = cvCreateHist(1, h_hist_size, CV_HIST_ARRAY, h_ranges, 1);
	cvCalcHist(h_plane, h_hist, 0, 0);

	float fTotal = m_nWidth * m_nHeight;
	int h = 0;
	m_pHHist = new float[H_HIST_BIN];

	for (h = 0; h<H_HIST_BIN; h++)
	{
		float h_bin_val = cvQueryHistValue_1D(h_hist, h);
		m_pHHist[h] = h_bin_val / fTotal;
	}
	cvReleaseHist(&h_hist);

	// calc entropy
	float fSum = 0.0f, fP = 0.0f;
	for (h = 0; h<H_HIST_BIN; h++)
	{
		fP = m_pHHist[h];

		if (fP < 1e-6)
		{
			fSum += 0.0f;
		}
		else
		{
			fSum += fP * log(fP);
		}
	}

	m_flHEntropy = -fSum / log(float(H_HIST_BIN));
}

void CImageHist::CalcSatHist()
{
	IplImage* s_plane[] = { m_pSImage };
	int s_hist_size[] = { S_HIST_BIN };
	float s_range[] = { 0, 255 };
	float* s_ranges[] = { s_range };

	CvHistogram* s_hist = cvCreateHist(1, s_hist_size, CV_HIST_ARRAY, s_ranges, 1);
	cvCalcHist(s_plane, s_hist, 0, 0);

	float fTotal = m_nWidth * m_nHeight;
	int s = 0;
	m_pSHist = new float[S_HIST_BIN];
	for (s = 0; s<S_HIST_BIN; s++)
	{
		float s_bin_val = cvQueryHistValue_1D(s_hist, s);
		m_pSHist[s] = s_bin_val / fTotal;
	}
	cvReleaseHist(&s_hist);

	// calc entropy
	float fSum = 0.0f, fP = 0.0f;
	for (s = 0; s<S_HIST_BIN; s++)
	{
		fP = m_pSHist[s];

		if (fP < 1e-6)
		{
			fSum += 0.0f;
		}
		else
		{
			fSum += fP * log(fP);
		}
	}

	m_flSEntropy = -fSum / log(float(S_HIST_BIN));
}

void CImageHist::CalcValHist()
{
	IplImage* v_plane[1];
	v_plane[0] = m_pVImage;
	int v_hist_size[] = { V_HIST_BIN };
	float v_range[] = { 0, 255 };
	float* v_ranges[] = { v_range };

	CvHistogram* v_hist = cvCreateHist(1, v_hist_size, CV_HIST_ARRAY, v_ranges, 1);
	cvCalcHist(v_plane, v_hist, 0, 0);

	float fTotal = m_nWidth * m_nHeight;
	int v = 0;

	m_pVHist = new float[V_HIST_BIN];
	for (v = 0; v<V_HIST_BIN; v++)
	{
		float v_bin_val = cvQueryHistValue_1D(v_hist, v);
		m_pVHist[v] = v_bin_val / fTotal;
	}
	cvReleaseHist(&v_hist);

	// calc entropy
	float fSum = 0.0f, fP = 0.0f;
	for (v = 0; v<V_HIST_BIN; v++)
	{
		fP = m_pVHist[v];

		if (fP < 1e-6)
		{
			fSum += 0.0f;
		}
		else
		{
			fSum += fP * log(fP);
		}
	}

	m_flVEntropy = -fSum / log(float(V_HIST_BIN));
}

void CImageHist::SetImage(IplImage* pImage)
{
	if ((pImage->nChannels != 3) || (NULL == pImage))
	{
		printf("\nError: in CImageHist, image must be 3 component!\n");
		exit(0);
	}
	m_flHEntropy = 0.0f;
	m_flSEntropy = 0.0f;
	m_flVEntropy = 0.0f;
	m_pHHist = NULL;
	m_pSHist = NULL;
	m_pVHist = NULL;

	m_nHeight = pImage->height;
	m_nWidth = pImage->width;

	m_pHSVImage = cvCreateImage(cvSize(m_nWidth, m_nHeight), IPL_DEPTH_8U, 3);
	m_pHImage = cvCreateImage(cvSize(m_nWidth, m_nHeight), IPL_DEPTH_8U, 1);
	m_pSImage = cvCreateImage(cvSize(m_nWidth, m_nHeight), IPL_DEPTH_8U, 1);
	m_pVImage = cvCreateImage(cvSize(m_nWidth, m_nHeight), IPL_DEPTH_8U, 1);

#ifdef RGB_TO_HSV
	cvCvtColor(pImage, m_pHSVImage, CV_RGB2HSV);
#endif

#ifdef BGR_TO_HSV
	cvCvtColor(pImage, m_pHSVImage, CV_BGR2HSV);
#endif
	cvSplit(m_pHSVImage, m_pHImage, m_pSImage, m_pVImage, NULL);
	/************************************************************************/
	/* Calc Image HSV Histogram
	/************************************************************************/
	CalcHueHist();
	CalcSatHist();
	CalcValHist();
}
