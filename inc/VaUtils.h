/******************************************************************************\

Microsoft Research Asia
Copyright (c) 2007 Microsoft Corporation

Module Name:
  Video Sdk

Abstract:
  This header file provide the utils functions used by video sdk

Notes:
  

History:
  Created on 05/11/2007 by linjuny@microsoft.com
\******************************************************************************/
#pragma once
#include "VxComm.h"
#include <Windows.h>
#include <string>

namespace VideoAnalysis
{
	///test whether it is a media file which can be supported 
	DLL_IN_EXPORT bool IsMediaFile(const wchar_t * fileName);

    ///the class get the video information(like duration, frame number, frame width, frame height, etc.)
	class DLL_IN_EXPORT CVideoInfo
	{
    public:
			CVideoInfo();
			CVideoInfo(const wchar_t *videoFullPathName);
			CVideoInfo(const CVideoInfo & info);
			CVideoInfo & operator = (const CVideoInfo & info);
    public:  
            int GetFrameWidth()const{ return m_FrameWidth; }
            int GetFrameHeight()const{ return m_FrameHeight; }
            int GetFrameNum()const{ return m_FrameNum; }
            double GetDuration()const{ return m_Duration; }
            double GetBitRate()const{ return m_BitRate; }
            unsigned long GetFileSize()const{ return m_FileSize; }

            void SetFrameWidth(int width){ m_FrameWidth = width; }
            void SetFrameHeight(int height){ m_FrameHeight = height; }
            void SetFrameNum(int num){ m_FrameNum = num; }
            void SetDuration(double duration){ m_Duration = duration; }
            void SetBitRate(double br){ m_BitRate = br; }
            void SetFileSize(unsigned long size){ m_FileSize = size; }
	private:
            int m_FrameWidth;                          ///the video frame width
			int m_FrameHeight;                         ///the video frame height
			int m_FrameNum;                            ///the frame number that the video includes
			double m_Duration;                         ///the duration of the video in 100ns
			double m_BitRate;                            ///the bitrate of the video file in bps
			unsigned long m_FileSize;                 ///the size of the video file in byte
	};

    ///extract the video info
	DLL_IN_EXPORT CVideoInfo ParseVideoInfo(const wchar_t *videoFullPathName);

}