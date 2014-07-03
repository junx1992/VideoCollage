#include "Stdafx.h"
#include <errno.h>
#include <Windows.h>
#include <string>
#include <iostream>
#include "common.h"
#include "featureExtract.h"
#include "VideoSegments.h"
#include "VideoSegmentStorage.h"
#include "EntropyOfflineSceneDetector.h"
#include "SabOnlineSubshotDetector.h"
#include "AffineGlobalMotionEstimation.h"
#include "FrameJSegmentor.h"
#include "VaUtils.h"
#include "GdiplusLife.h"
#include "Log.h"
using namespace ImageAnalysis;
using namespace std;
using namespace VxCore;
using namespace VideoAnalysis;



bool AllFeatureExtract(const ExtractConfig & config)
{
	//must need the saving floder
	if (config.m_SavedFloder.empty())
		return false;

	//nothing need to do
	if (!config.m_Features && !config.m_Structures && !config.m_Regions && !config.m_Thumbnail && !config.m_GME)
		return true;

	//create the saved floder
	if (_wmkdir(config.m_SavedFloder.c_str()) == -1)
	{
		if (errno != EEXIST)
		{
			wcout << endl << "Create saving floder " << config.m_SavedFloder << " failed!" << endl;
			return false;
		}
	}

	//initialize the CFrameJSegmentor
	CFrameJSegmentor::Initialize(3, -1, 0.6f);

	//the src is a floder       
	if (!config.m_SrcFloder.empty())
		AllFeatureExtractHelper_Dir(config);

	//the src is a file
	if (!config.m_SrcFile.empty())
		AllFeatureExtractHelper_File(config);

	return true;
}

bool AllFeatureExtractHelper_File(const ExtractConfig & config)
{
	wcout << endl << "Processing video " << config.m_SrcFile << endl;
	MetaDataExtractConfig  extractConfig(config.m_SrcFile);

	//open error log
	wstring logFile = config.m_SavedFloder + L"\\metadataExtract.log";
	char log[MAX_PATH] = { 0 };
	WideCharToMultiByte(CP_ACP, 0, logFile.c_str(), -1, log, MAX_PATH, NULL, NULL);

	//open the log
	LogOpen(log);
	//create floders
	CreateSavedFloders(config);
	//config the structure feature
	ConfigStructureFeatureExtract(config, extractConfig);
	//config the global feature
	ConfigGlobalFeatureExtract(config, extractConfig);
	//config the region feature
	ConfigRegionFeatureExtract(config, extractConfig);
	//config the thumb nail 
	ConfigThumbnailExtract(config, extractConfig);

	//the gdi initialize
	CGdiplusLife m_gdiLife;

	//run the engine
	cout << "\tShot detection";
	if (config.m_Features) cout << ", Feature extractioin";
	if (config.m_Regions) cout << ", Region extraction";
	if (config.m_GME) cout << ", GME";
	cout << " : " << endl;

	extractConfig.vaEngine.Run(PrintProgress, 100);
	cout << endl;

	//second round
	ExtractSubshot(config, extractConfig);
	ExtractScene(config, extractConfig);
	ExtractMotionThumbnail(config, extractConfig);

	//save
	SaveAll(config, extractConfig);

	return true;
}

bool AllFeatureExtractHelper_Dir(const ExtractConfig & config)
{
	WIN32_FIND_DATA   ffd;
	wstring select = config.m_SrcFloder + L"\\*.*";
	HANDLE hFind = FindFirstFile(select.c_str(), &ffd);

	if (hFind != INVALID_HANDLE_VALUE)
	{
		while (FindNextFile(hFind, &ffd))
		{
			if ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				if (wcscmp(ffd.cFileName, L".") == 0 || wcscmp(ffd.cFileName, L"..") == 0)
					continue;
				else {
					//do recursion scan for sub folder
					ExtractConfig recur_config = config;
					recur_config.m_SrcFloder = recur_config.m_SrcFloder + L"\\" + ffd.cFileName;
					recur_config.m_SavedFloder = recur_config.m_SavedFloder + L"\\" + ffd.cFileName;
					if (_wmkdir(recur_config.m_SavedFloder.c_str()) == -1)
					{
						if (errno != EEXIST)
						{
							wcout << endl << "Create saving dir for video floder " << ffd.cFileName << " failed!" << endl;
							continue;
						}
					}
					AllFeatureExtractHelper_Dir(recur_config);
					continue;
				}
			}

			wstring fullPath = config.m_SrcFloder + L'\\' + ffd.cFileName;
			//test whether it is a meidea file
			if (!IsMediaFile(fullPath.c_str()))
			{
				wcout << endl << ffd.cFileName << " is not a media file, so we skip it!" << endl;
				continue;
			}

			//make a saved floder for the file
			wstring dirName = config.m_SavedFloder + L"\\" + ffd.cFileName + L".metadata";
			//do the feature extract
			ExtractConfig localConfig = config;
			localConfig.m_SavedFloder = dirName;
			localConfig.m_SrcFile = fullPath;

			AllFeatureExtractHelper_File(localConfig);
		}
	}

	return true;
}


void SaveAll(const ExtractConfig & config, MetaDataExtractConfig & extConfig)
{
	wstring srcFile = config.m_SrcFile;
	//save video information
	VideoInfoSave(srcFile, config.m_SavedFloder, extConfig);
	//save structures
	if (config.m_Structures)
	{
		wstring dirName = config.m_SavedFloder + L"\\Structure";
		if (config.m_Structures & VA_SCENE)
		{
			const CVideoSegmentList & scenes = extConfig.SceneDetector->GetScenes();
			wstring savedTextFile = dirName + L"\\Shot.txt";
			wstring savedXMLFile = dirName + L"\\Structure.xml";
			//save the shot result to txt file
			SaveShotsToText(extConfig.ShotDetector->GetShots(), savedTextFile.c_str());
			//save the scene result to xml file
			SaveScenesToXML(scenes, savedXMLFile.c_str());
		}
		else if (config.m_Structures & VA_SUBSHOT) {
			wstring savedTextFile = dirName + L"\\Shot.txt";
			wstring savedXMLFile = dirName + L"\\Structure.xml";
			//save the shot result to txt file
			SaveShotsToText(extConfig.ShotDetector->GetShots(), savedTextFile.c_str());
			//save the shot result to xml file
			SaveShotsToXML(dynamic_cast<CSabOnlineSubshotDetector*>(extConfig.SubshotDetector)->GetCompleteShots(), savedXMLFile.c_str());
		}
		else if (config.m_Structures & VA_SHOT) {
			wstring savedTextFile = dirName + L"\\Shot.txt";
			wstring savedXMLFile = dirName + L"\\Structure.xml";
			//save the shot result to txt file
			SaveShotsToText(extConfig.ShotDetector->GetShots(), savedTextFile.c_str());
			//save the shot result to xml file
			SaveShotsToXML(extConfig.ShotDetector->GetShots(), savedXMLFile.c_str());
		}
	}

	//save global and region feature
	size_t size1 = extConfig.FrameFeatureHelpers.size();
	size_t size2 = extConfig.RegionFeatureHelpers.size();
	wstring dirName = config.m_SavedFloder + L"\\Feature";

	//save the GME result
	if (config.m_GME)
		(dynamic_cast<CAffineGlobalMotionEstimation*>(extConfig.AffineGMEExtractor))->GetData().Save(wstring(dirName + L"\\GME.txt").c_str());

	//save global feature
	for (size_t i = 0; i < size1; ++i)
	{
		wstring name = dirName + L"\\" + extConfig.FrameFeatureHelpers[i]->m_SavedName;
		extConfig.FrameFeatureExtractors[i]->GetData().Save(name.c_str());
	}

	//save region feature
	for (size_t i = 0; i < size2; ++i)
	{
		wstring name = dirName + L"\\" + extConfig.RegionFeatureHelpers[i]->m_SavedName;
		extConfig.RegionFeatureExtractors[i]->GetData().Save(name.c_str());
	}
}

bool  CreateSavedFloders(const ExtractConfig & config)
{
	//make a saved floder for the file
	if (_wmkdir(config.m_SavedFloder.c_str()) == -1)
	{
		if (errno != EEXIST)
		{
			wcout << endl << "Create saving floder for video " << config.m_SrcFile << " failed!" << endl;
			return false;
		}
	}

	//key frame saved floder
	if (config.m_Structures & VA_KEYFRAME)
	{
		//create key frame saving dir
		wstring keyframeDir = config.m_SavedFloder + L"\\Keyframe";
		if (_wmkdir(keyframeDir.c_str()) == -1)
		{
			if (errno != EEXIST)
			{
				wcout << endl << "Create key frame saving dir for video " << config.m_SrcFile << " failed!" << endl;
				return false;
			}
		}
	}

	//structrue floder
	if (config.m_Structures)
	{
		//create the "Structure" floder
		wstring dirName = config.m_SavedFloder + L"\\Structure";
		if (_wmkdir(dirName.c_str()) == -1)
		{
			if (errno != EEXIST)
			{
				wcout << endl << "Create Structure saving dir for video " << config.m_SrcFile << " failed!" << endl;
				return false;
			}
		}
	}

	//create the feature saved floder
	if (config.m_Features || config.m_Regions || config.m_GME)
	{
		wstring dirName = config.m_SavedFloder + L"\\Feature";
		if (_wmkdir(dirName.c_str()) == -1)
		{
			if (errno != EEXIST)
			{
				wcout << endl << "Create feature saving dir for video " << config.m_SrcFile << " failed!" << endl;
				return false;
			}
		}
	}

	//create the region map saved floder
	if (config.m_Regions & VA_REGIONMAP)
	{
		//create region map saving dir
		wstring regionDir = config.m_SavedFloder + L"\\RegionMap";
		if (_wmkdir(regionDir.c_str()) == -1)
		{
			if (errno != EEXIST)
			{
				wcout << endl << "Create region map saving dir for video " << config.m_SrcFile << " failed!" << endl;
				return false;
			}
		}
	}

	//create static thumbnail saved floder
	if (config.m_Thumbnail & VA_STATIC)
	{
		wstring dirName = config.m_SavedFloder + L"\\StaticThumb";
		if (_wmkdir(dirName.c_str()) == -1)
		{
			if (errno != EEXIST)
			{
				wcout << endl << "Create Static thumb nail saving dir for video " << config.m_SrcFile << " failed!" << endl;
				return false;
			}
		}
	}

	//create motion thumbnail saved floder
	if (config.m_Thumbnail & VA_MOTION)
	{
		//make a saving dir for the file
		wstring dirName = config.m_SavedFloder + L"\\MotionThumb";
		if (_wmkdir(dirName.c_str()) == -1)
		{
			if (errno != EEXIST)
			{
				wcout << endl << "Create motion thumb nail saving dir for video failed!" << endl;
				return false;
			}
		}
	}


	return true;
}
