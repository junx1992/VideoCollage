/******************************************************************************\

Microsoft Research Asia
Copyright (c) 2007 Microsoft Corporation

Module Name:
Video Sdk

Abstract:
This header file provide the template class for frame region feature factory.

Notes:

History:
Created on 08/01/2007 by linjuny@microsoft.com
Updated on 10/24/2013 by v-aotang@microsoft.com
\******************************************************************************/
#pragma once
#include <memory>

namespace  ImageAnalysis
{
	class IGrayImage;
	class CJSeg;
}

namespace VideoAnalysis
{
	class CFrame;
}

///Special for jseg, we cache the recently caculated segmentor region and numbers here
class CFrameJSegmentor
{
public:
	///NOTICE: Initialize MUST be called before any call to GetInstance, or you'll get a NULL
	///point of  CFrameSegmentor
	static void Initialize(int numberOfScale = -1, float quanThresh = -1, float mergeThresh = 0.4);
	///it's a singleton
	static CFrameJSegmentor * GetInstance();

	///get the segment detected by jseg
	ImageAnalysis::IGrayImage* GetSegment(const VideoAnalysis::CFrame & frame);

	///get the number of segment
	unsigned int GetSegmentNum(const VideoAnalysis::CFrame & frame);

	~CFrameJSegmentor();
private:
	CFrameJSegmentor(int numberOfScale = -1, float quanThresh = -1, float mergeThresh = 0.4);


	///!!!it is not implemented, which means don't allow any kind of copy
	CFrameJSegmentor(const CFrameJSegmentor &);
	CFrameJSegmentor & operator = (const CFrameJSegmentor &);

	///do the segment detect by jseg
	void DetectSemgment(const VideoAnalysis::CFrame & frame);

private:
	static std::auto_ptr<CFrameJSegmentor> m_Segmentor;
	ImageAnalysis::CJSeg * m_SegmentDetector;
	ImageAnalysis::IGrayImage* m_RegionMap;
	unsigned int m_SegmentNum;
	int m_FrameId;
};
