/******************************************************************************\

Microsoft Research Asia
Copyright (c) 2007 Microsoft Corporation

Module Name:
  Video Analysis Sdk

Abstract:
  This header file provide the class for offline scene detect

Notes:
  

History:
  Created on 06/28/2007 by v-huami@microsoft.com
\******************************************************************************/

#pragma once
#include "VideoParseInterface.h"
#include "VideoSegments.h"

namespace VxCore
{
	//interface for DataSet
	class IDataSet;
}

namespace VideoAnalysis
{
    ///the class for offline entropy scene detector
    class DLL_IN_EXPORT CEntropyOfflineSceneDetector: public IOfflineSceneDetector 
    {
	public:
        ///  ShotList: result of shot detect
        ///  Rgb256Histo: result of CRGB256HistogramFeatureExtractor 
        ///  maxNumOfScene: the max number of scenes is needed to be extracted  
        CEntropyOfflineSceneDetector(const CVideoSegmentList & shotList, const VxCore::IDataSet & rgb256Histo, int maxNumOfScene);
   		
        virtual const CVideoSegmentList& DetectSegmentOffline();
        virtual const CVideoSegmentList& GetScenes()const;
   private:
        const CVideoSegmentList & m_ShotList;
        const VxCore::IDataSet & m_RGB256Histogram;
        int m_NumOfScene;
        CVideoSegmentList m_SceneList;
	};    
}