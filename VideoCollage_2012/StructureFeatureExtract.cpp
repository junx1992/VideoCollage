#include "Stdafx.h"
#include "common.h"
#include "KeyframeSaver.h"
#include "featureExtract.h"
#include "ColorHistogramFeatureExtractor.h"
#include "EntropyOfflineSceneDetector.h"
#include "AffineOffineSubShotDetector.h"
#include "SabOnlineSubshotDetector.h"
#include "DThreshShotDetector.h"
#include "VideoSegmentStorage.h"
#include "AffineGlobalMotionEstimation.h"
#include "FrameFeatureExtractor.h"
#include "VaUtils.h"
#include <errno.h>
#include <Windows.h>
#include <iomanip>
#include <string>
#include <iostream>
using namespace std;
using namespace VideoAnalysis;
using namespace ImageAnalysis;


bool ConfigStructureFeatureExtract(const ExtractConfig & config, MetaDataExtractConfig & extConfig)
{
        if( config.m_Structures == 0 )
            return true;
#ifdef _DEBUG
       cout<<endl<<"Config structure feature extractor"<<endl;
#endif

       if( config.m_Structures & VA_SUBSHOT || config.m_GME )
       {
           extConfig.AffineGMEExtractor = new CAffineGlobalMotionEstimation;
           extConfig.vaEngine.AddReceiver(extConfig.AffineGMEExtractor);
       }

       //add key frame saver
       if( config.m_Structures & VA_KEYFRAME )
       {
           //key frame saving dir
           wstring keyframeDir = config.m_SavedFloder+L"\\Keyframe";
           extConfig.ShotKeyframeSaver = new CKeyframeSaver(keyframeDir.c_str(), config.m_KeyframeWidth, config.m_KeyframeHeight);
           extConfig.ShotDetector->AddReceiver(extConfig.ShotKeyframeSaver);
       }

       if( (config.m_Structures & VA_SCENE) || (config.m_Thumbnail & VA_MOTION) || (config.m_Thumbnail & VA_MOTION) )
       {
            extConfig.Rgb256Extractor = new CRGB256HistogramFeatureExtractor;
            extConfig.FeatureExtractor = new CFrameFeatureExtractor(extConfig.Rgb256Extractor);
            extConfig.ShotDetector->AddReceiver(extConfig.FeatureExtractor);
       }
      
       return true;
}

void ExtractSubshot(const ExtractConfig & config, MetaDataExtractConfig & extConfig)
{
         //do the sub shot detect
         if( config.m_Structures & VA_SUBSHOT )
         {
             //offline sub shot detector, we need the shot list and GME information
             IOfflineSubshotDetector* pOfflineSgtDetector = new CAffineOffineSubshotDetector(
                                                                                               extConfig.ShotDetector->GetShots(),
                                                                                               extConfig.AffineGMEExtractor->GetData());

             //online sub shot detector
             extConfig.SubshotDetector = new CSabOnlineSubshotDetector(extConfig.ShotDetector->GetShots(),pOfflineSgtDetector);
           
             //add key frame saver
             IVideoParseReceiver * pKeyframeSaver = NULL;
             if( config.m_Structures & VA_KEYFRAME )
             {
                 wstring keyframeSavedDir = config.m_SavedFloder+L"\\Keyframe";
                 pKeyframeSaver = new CKeyframeSaver(keyframeSavedDir.c_str(), config.m_KeyframeWidth, config.m_KeyframeHeight);
	             extConfig.SubshotDetector->AddReceiver(pKeyframeSaver);
             }

             CVideoAnalysisEngine subEngine;
             subEngine.Initialize(config.m_SrcFile.c_str());
             //register the online sub shot detector
             subEngine.AddReceiver(extConfig.SubshotDetector);

             //run the engine again
             cout<<"\tSubshot detection:"<<endl;
             subEngine.Run(PrintProgress, 100);
             cout<<endl;

             //stop
             subEngine.Stop();

             delete pOfflineSgtDetector;
             delete pKeyframeSaver;
       }
}

void ExtractScene(const ExtractConfig & config, MetaDataExtractConfig & extConfig)
{
       //do the scene  detect
       if( (config.m_Structures & VA_SCENE) || (config.m_Thumbnail & VA_MOTION) )
       {   
           if( config.m_Structures & VA_SUBSHOT )
               extConfig.SceneDetector = new CEntropyOfflineSceneDetector(dynamic_cast<CSabOnlineSubshotDetector*>(extConfig.SubshotDetector)->GetCompleteShots(), extConfig.FeatureExtractor->GetData(), 30);
           else
               extConfig.SceneDetector = new CEntropyOfflineSceneDetector(extConfig.ShotDetector->GetShots(), extConfig.FeatureExtractor->GetData(), 30);
           extConfig.SceneDetector->DetectSegmentOffline();
           cout<<"\tScene detection finished"<<endl;
       }
}
