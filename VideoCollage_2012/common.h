/******************************************************************************\

Microsoft Research Asia
Copyright (c) 2007 Microsoft Corporation

Module Name:
Video Analysis Sdk

Abstract:
This header file provide the common definitions

Notes:


History:
Created on 08/02/2007 by v-huami@microsoft.com
Updated on 10/24/2013 by v-aotang@microsoft.com
\******************************************************************************/

#pragma once

///the global feature 
#define VA_HVS64HISTO                           0x1
#define VA_RGB4096HISTO                         0x2
#define VA_RGB256HISTO                          0x4
#define VA_BLOCKWISE_COLORMOMENT                0x8
#define VA__FACE                                0x10
#define VA_COT                                  0x20
#define VA_EDH                                  0x40
#define VA_WAVELETTEXTURE                       0x80
#define VA_CORRELOGRAMFEATURE                   0x100

///the region feature
#define VA_CORRELOGRAMREGION                    0x1
#define VA_LAB123_COLORMOMENT                   0x2
#define VA_SHAPE                                0x4
#define VA_REGIONMAP                            0x8

///the structure
#define VA_SUBSHOT                              0x1
#define VA_SHOT                                 0x2
#define VA_SCENE                                0x4
#define VA_KEYFRAME                             0x8

///the motion
#define VA_MOTION                               0x1
#define VA_STATIC                               0x2

///extract working thread message,indicates extracting is over
#define EXTRACT_OVER                            WM_USER+100

#include <string>
#include"AdvancedKeyframeExtractor.h"

bool OpenFloder(const wchar_t * displayName, std::wstring & dir);
bool OpenFile(const wchar_t * filter, std::wstring & file);
bool SaveFile(const wchar_t * filter, std::wstring & savedName);

///the extract config 
struct ExtractConfig
{
	ExtractConfig()
	{
		Reset();
	}

	void Reset()
	{
		m_Features = 0;
		m_Regions = 0;
		///shot must be extracted!!!
		m_Structures = VA_SHOT;
		m_Thumbnail = 0;
		m_GME = 0;

		m_KeyframeWidth = 0;
		m_KeyframeHeight = 0;
		m_StaticWidth = 0;
		m_StaticHeight = 0;

		m_SavedFloder = L"";
		m_SrcFile = L"";
		m_SrcFloder = L"";
	}

	unsigned __int64 m_Features;
	unsigned __int64 m_Regions;
	unsigned __int64 m_Structures;
	unsigned __int64 m_Thumbnail;
	bool m_GME;
	int m_KeyframeWidth;
	int m_KeyframeHeight;
	int m_StaticWidth;
	int m_StaticHeight;
	std::wstring m_SavedFloder;
	std::wstring m_SrcFile;
	std::wstring m_SrcFloder;
};

#include "VideoAnalysisEngine.h"
#include "IImageFeatureExtractor.h"
#include "VideoParseInterface.h"
#include <vector>

void PrintProgress(VideoAnalysis::CVideoAnalysisEngine* const pEngine);
void PrintProgress(double progress);

struct FeatureExtractorHelper
{
	FeatureExtractorHelper() :m_Extractor(NULL){}
	~FeatureExtractorHelper(){ delete m_Extractor; }
	std::wstring m_SavedName;
	ImageAnalysis::IImageFeatureExtractor* m_Extractor;
};

struct RegionExtractorHelper
{
	RegionExtractorHelper() :m_Extractor(NULL){}
	~RegionExtractorHelper(){ delete m_Extractor; }
	std::wstring m_SavedName;
	ImageAnalysis::IImageRegionFeatureExtractor* m_Extractor;
};

struct MetaDataExtractConfig
{
	MetaDataExtractConfig(const std::wstring & videofile);
	~MetaDataExtractConfig();

	//the engine
	VideoAnalysis::CVideoAnalysisEngine vaEngine;
	//GME
	VideoAnalysis::IVideoFeatureExtractor * AffineGMEExtractor;
	//motion
	VideoAnalysis::IVideoParseReceiver * StaticThumbnailExtractor;
	//structure
	ImageAnalysis::IImageFeatureExtractor * Rgb256Extractor;
	VideoAnalysis::IVideoFeatureExtractor * FeatureExtractor;
	VideoAnalysis::IVideoParseReceiver * ShotKeyframeSaver;
	VideoAnalysis::ISubshotDetector*SubshotDetector;
	VideoAnalysis::IShotDetector * ShotDetector;
	VideoAnalysis::IOfflineSceneDetector * SceneDetector;
	//for feature
	std::vector<FeatureExtractorHelper*> FrameFeatureHelpers;
	std::vector<VideoAnalysis::IVideoFeatureExtractor*> FrameFeatureExtractors;
	//for region
	std::vector<RegionExtractorHelper*>  RegionFeatureHelpers;
	std::vector<VideoAnalysis::IVideoFeatureExtractor*> RegionFeatureExtractors;
	VideoAnalysis::IVideoParseReceiver * RegionMapSaver;

	VideoAnalysis::CAdvancedKeyframeExtractor keyframeExtractor;
	VideoAnalysis::IVideoFeatureExtractor *pRGBFrameFeatureExtractor;
//	ImageAnalysis::CRGB256HistogramFeatureExtractor rgb256HistogramFeatureExtractor;

};
