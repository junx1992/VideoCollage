/******************************************************************************\

Microsoft Research Asia
Copyright (c) 2007 Microsoft Corporation

Module Name:
  Video Analysis Sdk

Abstract:
  This header file provide the class for Simple Keyframe Extractor 

Notes:
  Re-wrote this class for the purpose of reusing the saved frames for motion/face/entropy detection

History:
  Created on 06/13/2007 by linjuny@microsoft.com
\******************************************************************************/
#pragma once
#include "VxComm.h"
#include "VideoParseInterface.h"
#include "RgbImage.h"
#include "GrayImage.h"
#include "VxFeature.h"
#include "ShotFeatureExtractor.h"
#include "SkinFeatureExtractor.h"
#include "DifferenceFeatureExtractor.h"
#include "ColorEntropyFeatureExtractor.h"
#include <fstream>
#include <string>

namespace VideoAnalysis
{
    ///using detrective
    using ImageAnalysis::RgbTriple;
    using ImageAnalysis::CRgbImage;
    using ImageAnalysis::CGrayImage;
    using VxCore::CIntFeature;

     ///forward declaration
    class CFrameList;
    class IVideoSegment;
    class CFrame;
    
    ///the default key frame extractor
    class CAdvancedKeyframeExtractor: public IKeyframeExtractor
    {
    public:
        CAdvancedKeyframeExtractor(void);
		~CAdvancedKeyframeExtractor(void);

        ///called when there is a new segment, and it will return a key frame list,
        /// *important* the returned key frame list will be invalidate when next call to OnNewSegment
        virtual CFrameList& OnNewSegment(IVideoSegment& segment);
        ///inform the extractor there is a new frame and whether it can be a key frame candidate
        virtual HRESULT OnNewFrame(CFrame& frame, bool isKeyframeCandidate);
    protected:
        ///the typedef for CRGB4096Histo
        typedef CIntFeature CRGB4096Histo;
        ///add a key frame candidate in the frame buffer
        void AddKeyFrameCandidate(CFrame & condKey);
        ///save pre-frame's feature
        void SaveFrameFeature(CFrame & frame);
        ///extract some key frames
        void ChooseKeyframe(unsigned int frameBgnId, unsigned int frameEndId);
    private:
        ///threshold for key frame candidate
        int m_Shot;
        ///hold the frame index
        int m_FrameId;
        ///the key frame cadidate of current shot
        CFrameList m_FrameArray;
        ///the key frame list
        CFrameList m_KeyframeList;
        ///wether the pre-frame is a black screen
        bool m_isBlackScreenPre;
        ///the histogram of previous key frame candidate
        CRGB4096Histo m_HistRef;
		int m_RecievedFrames;
		std::wstring logpath;
		std::ofstream m_log; // add by yang
	public:
		void SetLogPath(wchar_t *logfile); // add by yang
		///Frame Feature Extractor
		CShotFeatureExtractor m_DifferenceExtractor, m_EntropyExtractor, m_SkinExtractor;
    };
}
