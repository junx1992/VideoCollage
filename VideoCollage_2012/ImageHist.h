/***************************************************************************\

Module Name:
CImageHist

Abstract:
Extract HSV component entropy from a BGR image

History:

Created on 04/08/2005 by f-tmei (Tao Mei)

\***************************************************************************/
// ImageHist.h: interface for the CImageHist class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IMAGEHIST_H__5EC80B41_6D4C_4C0E_8BD4_8C9F7CADD9E3__INCLUDED_)
#define AFX_IMAGEHIST_H__5EC80B41_6D4C_4C0E_8BD4_8C9F7CADD9E3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <opencv2\opencv.hpp>
#include<Windows.h>

#define H_HIST_BIN 16
#define S_HIST_BIN 16
#define V_HIST_BIN 32

// #define RGB_TO_HSV				// for windows image format
#define BGR_TO_HSV			// for intel image format

#define CHECK_POINTER(x) if(!x) { printf("\nError: null pointer!\n"); exit(0); }

class CImageHist
{
public:
	CImageHist(){};
	CImageHist(IplImage *pImage);
	virtual ~CImageHist();

public:

	float m_flHEntropy;				// H-component entropy
	float m_flSEntropy;				// S-component entropy
	float m_flVEntropy;				// V-component entropy
	float *m_pHHist;
	float *m_pSHist;
	float *m_pVHist;
	UINT m_nWidth;
	UINT m_nHeight;

public:

	void CalcValHist();
	void CalcSatHist();
	void CalcHueHist();

	IplImage* m_pHSVImage;
	IplImage* m_pHImage;
	IplImage* m_pSImage;
	IplImage* m_pVImage;

	void SetImage(IplImage* pImage);
};

#endif // !defined(AFX_IMAGEHIST_H__5EC80B41_6D4C_4C0E_8BD4_8C9F7CADD9E3__INCLUDED_)

extern CImageHist* m_pImageHist;
