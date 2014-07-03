#include "stdafx.h"
//#include "cv\cv.h"
#include <opencv2\opencv.hpp>
#include "FocusDetector.h"
#include "qedit.h"
#include "uuids.h"
#include "dshow.h"


#pragma	    once

float GetAttention(IplImage* pImage);

double normcdf(double x);

void WriteTL(IAMTimeline *pTL, IRenderEngine *pRender, WCHAR *fileName);
IPin * GetOutPin(IBaseFilter * pFilter, int nPin);

HRESULT SaveGraphFile(IGraphBuilder *pGraph, WCHAR *wszPath);