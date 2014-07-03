// Skin.cpp : Defines the entry point for the console application.
//
#include <Windows.h>
#include "stdafx.h"
#include <gdiplus.h>
#include <RgbImage.h>
#include "..\KeyframeExtraction\SkinFeatureExtractor.h"
#include <iostream>

using namespace std;
using namespace Gdiplus;
using namespace ImageAnalysis;
void GenerateSkinImage(wchar_t *fileName, wchar_t *targetFileName)
{
	CRgbImage img;
	if (img.Load(fileName) == S_OK) {
		CSkinFeatureExtractor *pSkinFeatureExtractor = new CSkinFeatureExtractor();
		CRgbImage out;
		double score = pSkinFeatureExtractor->Extract(img, out);
		cout << "Skin score: " << score << endl;
		wchar_t outFilename[512];
		wsprintf(outFilename, L"%s_%d.jpg", targetFileName, (int)(score*100));
		out.Save(outFilename);
		delete pSkinFeatureExtractor;
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	//process the parameters
	const int filePathBuffSize = 512;
	wchar_t sourceFilePath[filePathBuffSize] = {0}, targetFilePath[filePathBuffSize] = {0}, fileType[filePathBuffSize] = {0};
	int length = 0;

	if (argc < 2)
	{
		cout << "Wrong parameters." << endl;
	}
	//initialize GDI+, for the SDK uses GDI+.
	// can also use:
	// ImageAnalysis:CGdiplusLife::Initialize();
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	for (int i = 1; i < argc; ++i)
	{
		wcscpy_s(sourceFilePath, MAX_PATH, argv[i]);
		wcscpy_s(targetFilePath, MAX_PATH, sourceFilePath);
		for (size_t j = wcslen(targetFilePath); j > 0; --j)
			if (targetFilePath[j] == L'.') {
				targetFilePath[j] = L'\0';
				wcscpy_s(fileType, MAX_PATH, targetFilePath+j+1);
				break;
			}
		if (wcscmp(fileType, L"jpg") == 0 || wcscmp(fileType, L"bmp") == 0 || wcscmp(fileType, L"png") == 0) {
			//wcscat(targetFilePath, L"_skin.jpg");
			GenerateSkinImage(sourceFilePath, targetFilePath);
		}
	}

	//finilize GDI+
	// can also use:
	// ImageAnalysis:CGdiplusLife::Uninitialize();
	GdiplusShutdown(gdiplusToken);

	return 0;
}

