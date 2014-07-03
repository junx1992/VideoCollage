#include "Stdafx.h"
#include "Common.h"
#include "featureExtract.h"
#include "ColorMomentFeatureExtractor.h"
#include "ShapeFeatureExtractor.h"
#include "CorrelogramFeatureExtractor.h"
#include "FrameRegionFeatureExtractor.h"
#include "VideoAnalysisEngine.h"
#include "DThreshShotDetector.h"
#include "VaUtils.h"
#include "RegionMapSaver.h"
#include "FrameJSegmentor.h"
#include <errno.h>
#include <Windows.h>
#include <vector>
#include <string>
#include <iostream>
using namespace std;
using namespace VideoAnalysis;
using namespace ImageAnalysis;

void PrintProgress(CVideoAnalysisEngine* const pEngine);


#define TEST_TO_ADD(features, certainFeature, extractorArray, extractor, name)\
    do{\
            if(features & certainFeature)\
            {\
               RegionExtractorHelper * helper = new RegionExtractorHelper;\
               helper->m_SavedName = name;\
               helper->m_Extractor = new extractor;\
               extractorArray.push_back(helper);\
            }\
    }while(false)

bool ConfigRegionFeatureExtract(const ExtractConfig & config, MetaDataExtractConfig & extConfig)
{
#ifdef _DEBUG
        cout<<"Config the region feature extractor"<<endl;
#endif

        try{
                TEST_TO_ADD(config.m_Regions, VA_CORRELOGRAMREGION, extConfig.RegionFeatureHelpers, CCorrelogramRegionFeatureExtractor, L"Correlogram36.txt");
                TEST_TO_ADD(config.m_Regions, VA_LAB123_COLORMOMENT, extConfig.RegionFeatureHelpers, CColorLabMoment123FeatureExtractor, L"ColorMoment9.txt");
                TEST_TO_ADD(config.m_Regions, VA_SHAPE, extConfig.RegionFeatureHelpers, CShapeFeatureExtractor, L"Shape3.txt");
        }catch(std::exception & e){
                cout<<endl<<e.what()<<endl;
                return false;
        }
        size_t size = extConfig.RegionFeatureHelpers.size();
        if( size == 0 &&  (config.m_Regions & VA_REGIONMAP) == 0 )  return true;

        //register the feature extractors
        for( size_t i = 0; i < size; ++i )
        { 
              extConfig.RegionFeatureExtractors.push_back(new CFrameRegionFeatureExtractor(extConfig.RegionFeatureHelpers[i]->m_Extractor));
              extConfig.ShotDetector->AddReceiver(extConfig.RegionFeatureExtractors.back());
        }

        if( config.m_Regions & VA_REGIONMAP )
        {
           //region map saving dir
           wstring regionDir = config.m_SavedFloder+L"\\RegionMap";
          extConfig.RegionMapSaver = new CRegionMapSaver(regionDir.c_str());
          extConfig.ShotDetector->AddReceiver(extConfig.RegionMapSaver);
        }

        return true;
}