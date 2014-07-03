/******************************************************************************\

Microsoft Research Asia
Copyright (c) 2007 Microsoft Corporation

Module Name:
  Video Analysis Sdk

Abstract:
  This header file provide the class for online sub shot detect

Notes:
  

History:
  Created on 06/28/2007 by v-huami@microsoft.com
\******************************************************************************/

#pragma once
#include "VideoParseInterface.h"
#include "VideoParseBase.h"

namespace VideoAnalysisHelper
{
     class  CSabOnlineSubshotDetectorImp;   
}

namespace VideoAnalysis
{
	///Sub shot array based sub shot detector class
	class DLL_IN_EXPORT CSabOnlineSubshotDetector : public ISubshotDetector
	{
	public:
        ///[in] shotList: shot list 
        ///[in] pOfflineSgtDetector:before do any online subshot detect operation, we should first do offline
        ///     subshot detect on the shot list, so the user MUST provide the offline subshot detector
        ///[in] pKeyframeExtractor: the sub shot key frame extract, if it's NULL, then we'll use CSimpleKeyframeExtractor
        ///[in] releaseImg is true, then the detector will not save the rgb image content of key frames because of large memeory overhead
        ///     if it is false, then the detector will maintain the content for the user
		CSabOnlineSubshotDetector(const CVideoSegmentList& shotList,
                                  IOfflineSubshotDetector* pOfflineSgtDetector, 
                                  IKeyframeExtractor* pKeyframeExtractor = NULL,
                                  bool releaseImg = true);
        ~CSabOnlineSubshotDetector(void);
    public:
        //implementation of IVideoParseReceiver
        virtual HRESULT OnNewSegment(IVideoSegment& segment);
	    virtual HRESULT EndOfStream();
    public:
        //implementation of IVideoParseController
        virtual HRESULT AddReceiver(IVideoParseReceiver* const);
        virtual HRESULT RemoveReceiver(const IVideoParseReceiver* const);
    public:
  		///get the information of all the detected subshots.
		///NOTE: whether the keyframe's rgb image is validate in the subshot, according to the parameter *releaseImg* in ctor
		virtual const CVideoSegmentList& GetSubshots() const;

        ///get the shot list filled with sub shot information
        const  CVideoSegmentList& GetCompleteShots() const;
    private:
        CSabOnlineSubshotDetector(const CSabOnlineSubshotDetector&);
        CSabOnlineSubshotDetector& operator=(const CSabOnlineSubshotDetector&);

  private:
        VideoAnalysisHelper::CSabOnlineSubshotDetectorImp * m_pImp;
	};
}