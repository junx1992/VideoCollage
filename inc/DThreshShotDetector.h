/******************************************************************************\

Microsoft Research Asia
Copyright (c) 2007 Microsoft Corporation

Module Name:
  Video Analysis Sdk

Abstract:
  This header file provide the class for double threshold shot detector 

Notes:
  

History:
  Created on 05/25/2007 by linjuny@microsoft.com
\******************************************************************************/
#pragma once
#include "VxComm.h"
#include "VideoParseInterface.h"

namespace VideoAnalysisHelper
{
    class CDThreshShotDetectorImp;

	///the process state
    enum ProcessState 
	{
		PROCESS_NONE = -1,                                      ///mean nothing happens
		PROCESS_FLASH_CANDIDATE,                                ///mean it is a flashlight candidate
		PROCESS_GRADUAL_TRANSITION_UNFINISH,                    ///mean it is still in the period of gradual transition
		PROCESS_NORMAL                                          ///mean it is in the common state
	};

    ///the result state after process
	enum ResultState 
	{
		RESULT_NONE = -1,
		RESULT_CUT,                                              ///mean it is a shot
		RESULT_CUT_PRE,                                          ///mean it is a shot in previous
		RESULT_GRADUAL_TRANSITION,                               ///mean it is still in the period of gradual transition
		RESULT_FLASH_CANDIDATE,                                  ///mean it may be a flashlight
		RESULT_FLASH,                                            ///mean it is a flashlight
		RESULT_NORMAL                                            ///mean it is in the common state
	};

    ///The threshold for shot detects
	struct ShotDetectThreshold
	{
		static double	dblDiffLowThresh;                    ///0.0959
		static double	dblDiffRatioThresh;                  ///3.0,	Threshold for the ratio of current histogram diff and previous histogram diff
		static double	dblDiffThresh;                       ///0.22	Threshold for histogram diff
		static double	dblKeyFrameThresh;                   ///0.22	Threshold for key frame extraction
		static int	iAvgYDiffThresh;                     ///3       Threshold for average intensity of a frame
		static int	iShotDuration;                       ///30		We assume that a shot could not be shorter than this
		static int	iFlashDuration;                      ///5		We assume that a flash light could not be longer than this
		static int	iGradualTransDuration;               ///7		we assume that a Gradual transition should not be less than this
		static int	iSkippedFrames;                      ///5		we should skip the first frames for denoising
    };
}

namespace VideoAnalysis
{
	using namespace VideoAnalysisHelper;
    ///double threshold shot detector, 
    ///this is the default shot detector
    class DLL_IN_EXPORT CDThreshShotDetector: public IShotDetector
    {
    public:
        ///if the parameter is NULL, then a default keyframe extractor is used.
        ///releaseImg is true, then the detector will not save the rgb image content of key frames because of large memeory overhead
        ///if it is false, then the detector will maintain the content for the user
       CDThreshShotDetector(IKeyframeExtractor* pKeyframeExtractor = NULL, bool releaseImg = true);
       ~CDThreshShotDetector(void);
    public:
        //implementation of IVideoParseReceiver
        virtual HRESULT OnNewSegment(IVideoSegment& segment);
        virtual HRESULT EndOfStream();
    public:
        //implementation of IVideoParseController
        virtual HRESULT AddReceiver(IVideoParseReceiver* const);
        virtual HRESULT RemoveReceiver(const IVideoParseReceiver* const);
    public:
        ///get the information of all the detected shots.
        ///NOTE: the keyframe in the shot is not available.
        virtual const CVideoSegmentList& GetShots() const;
    private:
        //not implemented, means do not allow assignment and copy
        CDThreshShotDetector(const CDThreshShotDetector&);
        CDThreshShotDetector& operator=(const CDThreshShotDetector&);
    private:
        CDThreshShotDetectorImp* m_pImp;
    };
}