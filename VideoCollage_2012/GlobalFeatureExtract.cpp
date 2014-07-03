#include "Stdafx.h"
#include "Common.h"
#include "featureExtract.h"
#include "ColorHistogramFeatureExtractor.h"
#include "ColorMomentFeatureExtractor.h"
#include "CorrelogramFeatureExtractor.h"
#include "EdgeFeatureExtractor.h"
#include "FaceFeatureExtractor.h"
#include "TextureFeatureExtractor.h"
#include "FrameFeatureExtractor.h"
#include "VideoAnalysisEngine.h"
#include "DThreshShotDetector.h"
#include "VaUtils.h"
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
               FeatureExtractorHelper * helper = new FeatureExtractorHelper;\
               helper->m_SavedName = name;\
               helper->m_Extractor = new extractor;\
               extractorArray.push_back(helper);\
            }\
    }while(false)


bool ConfigGlobalFeatureExtract(const ExtractConfig & config, MetaDataExtractConfig & extConfig)
{
#ifdef _DEBUG
        cout<<"Config key frame feature extractor"<<endl;
#endif 

         try{
                TEST_TO_ADD(config.m_Features, VA_HVS64HISTO, extConfig.FrameFeatureHelpers, CHSV64HistogramFeatureExtractor, L"HSV64.txt");
                TEST_TO_ADD(config.m_Features, VA_RGB4096HISTO, extConfig.FrameFeatureHelpers, CRGB4096HistogramFeatureExtractor, L"RGB4096.txt");
                TEST_TO_ADD(config.m_Features, VA_RGB256HISTO, extConfig.FrameFeatureHelpers, CRGB256HistogramFeatureExtractor, L"RGB256.txt");
                TEST_TO_ADD(config.m_Features, VA_BLOCKWISE_COLORMOMENT, extConfig.FrameFeatureHelpers, CBlockWiseColorLabMoment123FeatureExtractor, L"ColorMoment255.txt");
                TEST_TO_ADD(config.m_Features, VA__FACE, extConfig.FrameFeatureHelpers, CFaceFeatureExtractor, L"Face7.txt");
                TEST_TO_ADD(config.m_Features, VA_COT, extConfig.FrameFeatureHelpers, CCOTFeatureExtractor, L"COT16.txt");
                TEST_TO_ADD(config.m_Features, VA_EDH, extConfig.FrameFeatureHelpers, CEDHFeatureExtractor, L"EDH75.txt");
                TEST_TO_ADD(config.m_Features, VA_WAVELETTEXTURE, extConfig.FrameFeatureHelpers, CWaveletTextureFeatureExtractor, L"WaveletTexture128.txt");
                TEST_TO_ADD(config.m_Features, VA_CORRELOGRAMFEATURE, extConfig.FrameFeatureHelpers, CCorrelogramFeatureExtractor, L"Correlogram144.txt");
        }catch(std::exception & e){
                cout<<endl<<e.what()<<endl;
                return false;
        }
        size_t size = extConfig.FrameFeatureHelpers.size();
        if( size == 0 )  return true;

        //create the saving dir
        wstring dirName = config.m_SavedFloder+L"\\Feature";
        if( _wmkdir(dirName.c_str()) == -1 )
        {
              if( errno != EEXIST )
              {
                  wcout<<endl<<"Create feature saving dir for video "<<config.m_SrcFile<<" failed!"<<endl;
                  return false;
              }
        }

        //register the feature extractors
        for( size_t i = 0; i < size; ++i )
        { 
              extConfig.FrameFeatureExtractors.push_back(new CFrameFeatureExtractor(extConfig.FrameFeatureHelpers[i]->m_Extractor));
              extConfig.ShotDetector->AddReceiver(extConfig.FrameFeatureExtractors.back());
        }
        
        return true;
}