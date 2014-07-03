#include "Stdafx.h"
#include "common.h"
#include <shlobj.h>
#include <iostream>
#include <iomanip>
#include "DThreshShotDetector.h"
#include"AdvancedKeyframeExtractor.h"
using namespace std;
using namespace VideoAnalysis;
using namespace ImageAnalysis;

struct ComInitHelper
{
	ComInitHelper(){ CoInitialize(NULL); }
	~ComInitHelper(){ CoUninitialize(); }
};

bool OpenFloder(const wchar_t * displayName, wstring & dir)
{
	ComInitHelper comInit;
	wstring   strFilePath;
	wchar_t   pszBuffer[MAX_PATH];
	LPITEMIDLIST   pidl;
	BROWSEINFO   bi;
	bi.hwndOwner = NULL;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = pszBuffer;
	bi.lpszTitle = displayName;
	bi.ulFlags = BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS | BIF_EDITBOX | BIF_NEWDIALOGSTYLE;
	bi.lpfn = NULL;
	bi.lParam = 0;
	if ((pidl = SHBrowseForFolder(&bi)) != NULL)
	{
		if (SHGetPathFromIDList(pidl, pszBuffer))
			dir = pszBuffer;
		else
			return false;
	}

	return true;
}

bool OpenFile(const wchar_t * filter, std::wstring & file)
{
	OPENFILENAME ofn;               // common dialog box structure
	wchar_t szFile[MAX_PATH];       // buffer for file name

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = szFile;
	//
	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
	// use the contents of szFile to initialize itself.
	//
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	// Display the Open dialog box. 
	if (GetOpenFileName(&ofn) == TRUE)
		file = ofn.lpstrFile;
	else if (CommDlgExtendedError() != 0)      //=0 means the user cancel or close the file select dialog
		return false;

	return true;
}

bool SaveFile(const wchar_t * filter, wstring & savedName)
{
	OPENFILENAME ofn;               // common dialog box structure
	wchar_t szFile[MAX_PATH];       // buffer for file name

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));

	szFile[0] = '\0';
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = NULL;
	ofn.lpstrFilter = filter;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile) / sizeof(*szFile);
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_SHOWHELP | OFN_OVERWRITEPROMPT;

	// Display the Filename common dialog box. The 
	// filename specified by the user is passed 
	// to the CreateEnhMetaFile function and used to 
	// store the metafile on disk. 

	if (GetSaveFileName(&ofn))
		savedName = ofn.lpstrFile;
	else if (CommDlgExtendedError() != 0)      //=0 means the user cancel or close the file select dialog
		return false;

	return true;
}

void PrintProgress(CVideoAnalysisEngine* const pEngine)
{
	cout << setiosflags(ios_base::fixed);
	cout << setprecision(2) << pEngine->GetProgress() * 100 << "%   completed \r";
}

void PrintProgress(double progress)
{
	cout << setiosflags(ios_base::fixed);
	cout << setprecision(2) << progress * 100 << "%   completed \r";
}

MetaDataExtractConfig::MetaDataExtractConfig(const wstring & videofile)
{

	//junx
	//ShotDetector=new CDThreshShotDetector(&keyframeExtractor, false);	
	//pRGBFrameFeatureExtractor = new CFrameFeatureExtractor(&rgb256HistogramFeatureExtractor); // register into frame feature extractor which is a receiver
	//(*ShotDetector).AddReceiver(pRGBFrameFeatureExtractor); // it is just for shot, not each frame. What a shame.
	

	vaEngine.Initialize(videofile.c_str());
//	vaEngine.AddReceiver(ShotDetector);


	//GME
	AffineGMEExtractor = NULL;
	//motion
	StaticThumbnailExtractor = NULL;
	//structure
	Rgb256Extractor = NULL;
	FeatureExtractor = NULL;
	ShotKeyframeSaver = NULL;
	SubshotDetector = NULL;
	SceneDetector = NULL;
	//region
	RegionMapSaver = NULL;
}

MetaDataExtractConfig::~MetaDataExtractConfig()
{
	//GME
	delete AffineGMEExtractor;
	//motion
	delete StaticThumbnailExtractor;
	//structure
	delete Rgb256Extractor;
	delete FeatureExtractor;
	delete ShotKeyframeSaver;
	delete ShotDetector;
	delete SubshotDetector;
	delete SceneDetector;
	//region
	delete RegionMapSaver;

	//for region
	size_t size = FrameFeatureHelpers.size();
	for (size_t i = 0; i < size; ++i)
	{
		delete FrameFeatureHelpers[i];
		delete FrameFeatureExtractors[i];
	}

	size = RegionFeatureHelpers.size();
	for (size_t i = 0; i < size; ++i)
	{
		delete RegionFeatureHelpers[i];
		delete RegionFeatureExtractors[i];
	}
}