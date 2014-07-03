#include "stdafx.h"
#include "global.h"
//
//HRESULT InitializeWindowlessVMR(IGraphBuilder* pGB,IBaseFilter **ppVmr9,HWND gApp)
//{
//	IBaseFilter *pVmr=NULL;
//	if (!ppVmr9)
//		return E_POINTER;
//	*ppVmr9 = NULL;
//
//	// Create the VMR and add it to the filter graph.
//	HRESULT hr = CoCreateInstance(CLSID_VideoMixingRenderer9, NULL,
//		CLSCTX_INPROC, IID_IBaseFilter, (void**)&pVmr);
//	if (SUCCEEDED(hr)) 
//	{
//		hr = pGB->AddFilter(pVmr, L"Video Mixing Renderer 9");
//			if (SUCCEEDED(hr)) 
//			{
//				// Set the rendering mode and number of streams.
//				CComPtr <IVMRFilterConfig9> pConfig;
//
//				JIF(pVmr->QueryInterface(IID_IVMRFilterConfig9, 
//					(void**)&pConfig));
//				JIF(pConfig->SetRenderingMode(VMR9Mode_Windowless));
//				JIF(pConfig->SetNumberOfStreams(2));
//
//				// Set the bounding window and border for the video.
//				JIF(pVmr->QueryInterface(IID_IVMRWindowlessControl9, 
//					(void**)&pWC));
//				JIF(pWC->SetVideoClippingWindow(ghApp));
//				JIF(pWC->SetBorderColor(RGB(0,0,0)));  // Black border
//
//				// Get the mixer control interface for manipulation of video 
//				// stream output rectangles and alpha values.
//				JIF(pVmr->QueryInterface(IID_IVMRMixerControl9, 
//					(void**)&pMix));
//			}
//			// Don't release the pVmr interface because we are copying it into
//			// the caller's ppVmr9 pointer.
//			*ppVmr9 = pVmr;
//	}
//
//	return hr;
//}
//HRESULT SaveGraphFile(IGraphBuilder *pGraph, WCHAR *wszPath)
//{
//	const WCHAR wszStreamName[] = L"ActiveMovieGraph";
//	HRESULT hr;
//	IStorage *pStorage = NULL;
//	hr = StgCreateDocfile(
//		wszPath,
//		STGM_CREATE | STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
//		0, &pStorage);
//	if (FAILED(hr))
//	{
//		return hr;
//	}
//	IStream *pStream;
//	hr = pStorage->CreateStream(
//		wszStreamName,
//		STGM_WRITE | STGM_CREATE | STGM_SHARE_EXCLUSIVE,
//		0, 0, &pStream);
//	if (FAILED(hr))
//	{
//		pStorage->Release();
//		return hr;
//	}
//	IPersistStream *pPersist = NULL;
//	pGraph->QueryInterface(IID_IPersistStream, (void**)&pPersist);
//	hr = pPersist->Save(pStream, TRUE);
//	pStream->Release();
//	pPersist->Release();
//	if (SUCCEEDED(hr))
//	{
//		hr = pStorage->Commit(STGC_DEFAULT);
//	}
//	pStorage->Release();
//	return hr;
//}

//HRESULT GetPin(IBaseFilter * pFilter, PIN_DIRECTION dirrequired, int iNum, IPin **ppPin)
//{
//	CComPtr< IEnumPins > pEnum;
//	*ppPin = NULL;
//
//	if (!pFilter)
//		return E_POINTER;
//
//	HRESULT hr = pFilter->EnumPins(&pEnum);
//	if (FAILED(hr))
//		return hr;
//
//	ULONG ulFound;
//	IPin *pPin;
//	hr = E_FAIL;
//
//	while (S_OK == pEnum->Next(1, &pPin, &ulFound))
//	{
//		PIN_DIRECTION pindir = (PIN_DIRECTION)3;
//		pPin->QueryDirection(&pindir);
//		if (pindir == dirrequired)
//		{
//			/*AM_MEDIA_TYPE* pamt;
//			CComPtr<IEnumMediaTypes> pEnum;
//			hr = pPin->EnumMediaTypes(&pEnum);
//			if (FAILED(hr))
//			{
//			break;
//			}
//			hr = pEnum->Next(1, &pamt, NULL);
//			if (FAILED(hr))
//			{
//			break;
//			}
//			BOOL bVideo = IsEqualGUID(pamt->majortype, MEDIATYPE_Video);
//			if (bVideo)*/
//			{
//				if (iNum == 0)
//				{
//					*ppPin = pPin;  // Return the pin's interface
//					hr = S_OK;      // Found requested pin, so clear error
//					break;
//				}
//				iNum--;
//			}
//			pPin->Release();
//		}
//		return hr;
//	}
//}
//IPin * GetOutPin(IBaseFilter * pFilter, int nPin)
//{
//	CComPtr<IPin> pComPin;
//	GetPin(pFilter, PINDIR_OUTPUT, nPin, &pComPin);
//	return pComPin;
//}
//void WriteTL(IAMTimeline *pTL, IRenderEngine *pRender, WCHAR *fileName)
//{
//	IGraphBuilder   *pGraph = NULL;
//	ICaptureGraphBuilder2 *pBuilder = NULL;
//	IMediaControl   *pControl = NULL;
//	IMediaEvent     *pEvent = NULL;
//
//	// Build the graph.
//	HRESULT hr = pRender->SetTimelineObject(pTL);
//	hr = pRender->ConnectFrontEnd();
//
//	hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC,
//		IID_ICaptureGraphBuilder2, (void **)&pBuilder);
//
//	// Get a pointer to the graph front end.
//	hr = pRender->GetFilterGraph(&pGraph);
//	hr = pBuilder->SetFiltergraph(pGraph);
//
//	// Create the file-writing section.
//	IBaseFilter *pMux;
//	hr = pBuilder->SetOutputFileName(&MEDIASUBTYPE_Avi, fileName,
//		&pMux, NULL);
//
//	long NumGroups;
//	hr = pTL->GetGroupCount(&NumGroups);
//
//	// Loop through the groups and get the output pins.
//	for (int i = 0; i < NumGroups; i++)
//	{
//		IPin *pPin;
//		if (pRender->GetGroupOutputPin(i, &pPin) == S_OK)
//		{
//			// Connect the pin.
//			hr = pBuilder->RenderStream(NULL, NULL, pPin, NULL, pMux);
//			pPin->Release();
//		}
//	}
//
//	// Run the graph.
//	hr = pGraph->QueryInterface(IID_IMediaControl, (void **)&pControl);
//	hr = pGraph->QueryInterface(IID_IMediaEvent, (void **)&pEvent);
//	SaveGraphFile(pGraph, L"C:\\MyGraph_write.GRF");  // Save the graph
//	// file to disk
//	hr = pControl->Run();
//
//	long evCode;
//	hr = pEvent->WaitForCompletion(INFINITE, &evCode);
//	hr = pControl->Stop();
//
//	// Clean up.
//	if (pMux) {
//		pMux->Release();
//	}
//	pEvent->Release();
//	pControl->Release();
//	pGraph->Release();
//	pBuilder->Release();
//}
//
//float GetAttention(IplImage* pImage)
//{
//	int iAttWidth = pImage->width;
//	float totAttent = 0.0;
//	int iAttHeight = pImage->height;
//	int iStride = iAttWidth * 3, iBits = 24;
//	int iAttNum = 0, iSaliencyWidth = 0, iSaliencyHeight = 0;
//	BOOL fFaceAvailable = TRUE;
//	BYTE *pData = new BYTE[iAttWidth*iAttHeight * 3];
//	memcpy(pData, pImage->imageData, iAttWidth*iAttHeight * 3);
//	CFocusDetector *pFocusDetector = new CFocusDetector;
//	HRESULT hr = pFocusDetector->Initialize(pData, iAttWidth, iAttHeight, iStride, iBits);
//	if (S_OK != hr)
//	{
//		//( "\nError: failed to initialize focusdetector!\n" );
//		delete pFocusDetector;
//		return 0;
//	}
//	else
//	{
//		pFocusDetector->SetFaceInformation(NULL, 0);
//		float* pSaliencyMap = pFocusDetector->ExtractSaliencyMap(&iSaliencyWidth, &iSaliencyHeight);
//
//		for (int iy = 0; iy < iSaliencyHeight; iy++)
//		{
//			for (int ix = 0; ix < iSaliencyWidth; ix++)
//			{
//				totAttent += pSaliencyMap[iSaliencyWidth * (iSaliencyHeight - 1 - iy) + ix];
//			}
//		}
//		totAttent /= (iSaliencyWidth*iSaliencyHeight);
//
//	}
//	pFocusDetector->DeInitialize();
//	delete pFocusDetector;
//	delete pData;
//	return totAttent;
//}

void Myerff(double x, double *erfx, double *erfcx);

double R1(double x);

double R2(double x);

double R3(double x);

double normcdf(double x)
/*
This subroutine returns the value of the standard normal integral
from -INF to x.
It is based on the approximation to the error function given in
Kennedy & Gentle, pp. 90-91.
*/
{
	double erfx, erfcx, PHI;
	/*
	printf("The number passed to normcdf = %8.4f\n", x);
	*/
	if (x == 0.0)
		PHI = 0.5;
	else
	{
		x = x / sqrt(2.0);
		if (x < 0)
			x = -x;
		Myerff(x, &erfx, &erfcx);
		if (x < 0)
			PHI = erfcx / 2.0;
		else
			PHI = (1.0 + erfx) / 2.0;
	}
	return(PHI);
}

void Myerff(double x, double *erfx, double *erfcx)
/*
This subroutine returns the value of the error function.
It is based on the approximation given in
Kennedy & Gentle, pp. 90-91.
*/
{
	double pi = 3.1415926535;
	if (x < 0.46875)
	{
		*erfx = x*R1(x);
		*erfcx = 1.0 - *erfx;
	}
	else
	{
		if (x < 4.0)
			*erfcx = exp(-x*x)*R2(x);
		else
			*erfcx = (exp(-x*x) / x) * ((1.0 / sqrt(pi)) + R3(1.0 / (x*x)) / (x*x));
		*erfx = 1.0 - *erfcx;
	}
}


double R1(double x)
{
	int j;
	double p[4], q[4];
	double num, denom;

	p[0] = 2.4266795523e02;
	p[1] = 2.1979261618e01;
	p[2] = 6.9963834886;
	p[3] = -3.5609843702e-02;
	q[0] = 2.1505887587e02;
	q[1] = 9.1164905405e01;
	q[2] = 1.50827976304e01;
	q[3] = 1.0;

	num = 0.0;
	denom = 0.0;
	for (j = 0; j<4; j++)
	{
		num += p[j] * pow(x, (2.0 * (double)j));
		denom += q[j] * pow(x, (2.0 * (double)j));
	}
	return(num / denom);
}

double R2(double x)
{
	int j;
	double p[8], q[8];
	double num, denom;

	p[0] = 3.0045926102e02;
	p[1] = 4.5191895371e02;
	p[2] = 3.3932081673e02;
	p[3] = 1.5298928505e02;
	p[4] = 4.3162227222e01;
	p[5] = 7.2117582509;
	p[6] = 5.6419551748e-01;
	p[7] = -1.3686485738e-07;
	q[0] = 3.0045926096e02;
	q[1] = 7.9095092533e02;
	q[2] = 9.3135409485e02;
	q[3] = 6.3898026447e02;
	q[4] = 2.7758544474e02;
	q[5] = 7.7000152935e01;
	q[6] = 1.2782727320e01;
	q[7] = 1.0;

	num = 0.0;
	denom = 0.0;
	for (j = 0; j<8; j++)
	{
		num += p[j] * pow(x, (double)j);
		denom += q[j] * pow(x, (double)j);
	}
	return(num / denom);
}

double R3(double x)
{
	int j;
	double p[5], q[5];
	double num, denom;

	p[0] = -2.9961070770e-03;
	p[1] = -4.9473091062e-02;
	p[2] = -2.2695659264e-01;
	p[3] = -2.7866130861e-01;
	p[4] = -2.2319245973e-02;
	q[0] = 1.0620923053e-02;
	q[1] = 1.9130892611e-01;
	q[2] = 1.0516751071;
	q[3] = 1.9873320182;
	q[4] = 1.0;

	num = 0.0;
	denom = 0.0;
	for (j = 0; j<5; j++)
	{
		num += p[j] / pow(x, (2.0 * (double)j));
		denom += q[j] / pow(x, (2.0 * (double)j));
	}
	return(num / denom);
}