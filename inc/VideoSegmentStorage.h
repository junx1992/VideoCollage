/******************************************************************************\

Microsoft Research Asia
Copyright (c) 2007 Microsoft Corporation

Module Name:
  Video Analysis Sdk

Abstract:
  This header file provide the helper functions for saving the video features

Notes:
  

History:
  Created on 06/26/2007 by v-huami@microsoft.com
\******************************************************************************/
#pragma once
#include "VxComm.h"

namespace VxCore
{
    struct FileInfo;
}
namespace VideoAnalysis
{
    class CVideoInfo;
    class CVideoSegmentList;

    ///save a shot list into text file
    ///the save format is <shotId bgnframeId endframeId bgntime endtime keyframenum keyframeid... >
    DLL_IN_EXPORT HRESULT SaveShotsToText(const CVideoSegmentList & shotList, const wchar_t * const wszFullPathFileName);
    ///save a subshot list into text file
    ///the save format is <subshotId shotId bgnframeId endframeId bgntime endtime keyframeid>
    DLL_IN_EXPORT HRESULT SaveSubshotsToText(const CVideoSegmentList & subshotList, const wchar_t * const wszFullPathFileName);
    ///save a scene list into text file
    ///the save format is <sceneid keyframenum keyframeid....>
    DLL_IN_EXPORT HRESULT SaveScenesToText(const CVideoSegmentList & sceneList, const wchar_t * const wszFullPathFileName);

    ///save a subshot list into xml file
    DLL_IN_EXPORT HRESULT SaveSubshotsToXML(const CVideoSegmentList & subshotList, const wchar_t * const wszFullPathFileName);
    ///save a shot list into xml file
    DLL_IN_EXPORT HRESULT SaveShotsToXML(const CVideoSegmentList & shotList, const wchar_t * const wszFullPathFileName);
    ///save a scene into xml file
    DLL_IN_EXPORT HRESULT SaveScenesToXML(const CVideoSegmentList & sceneList, const wchar_t * const wszFullPathFileName);
    ///save video file information into xml file
    ///the information includes: filename, Duration,FileByteSize,FramesNum,FrameWidth,FrameHeight
    DLL_IN_EXPORT HRESULT SaveVideoInfoToXML(const CVideoInfo & videoInfo, const VxCore::FileInfo & fileInfo, const wchar_t * const wszFullPathFileName);

    ///load a subshot list from text file
    DLL_IN_EXPORT HRESULT LoadSubshotsFromText(CVideoSegmentList & subshotList, const wchar_t * const wszFullPathFileName);
    ///load a shot list from text file
    DLL_IN_EXPORT HRESULT LoadShotsFromText(CVideoSegmentList & shotList, const wchar_t * const wszFullPathFileName);
    ///load a scene list from text file
    DLL_IN_EXPORT HRESULT LoadScenesFromText(CVideoSegmentList & sceneList, const wchar_t * const wszFullPathFileName);
    ///save video file information into xml file
    ///the information includes: filename, Duration,FileByteSize,FramesNum,FrameWidth,FrameHeight
    DLL_IN_EXPORT HRESULT LoadVideoInfoFromXML(CVideoInfo & videoInfo, VxCore::FileInfo & fileInfo, const wchar_t * const wszFullPathFileName);

    ///load a subshot list from xml file
    DLL_IN_EXPORT HRESULT LoadSubshotsFromXML(CVideoSegmentList & subshotList, const wchar_t * const wszFullPathFileName);
    ///load a shot list from xml file
    DLL_IN_EXPORT HRESULT LoadShotsFromXML(CVideoSegmentList & shotList, const wchar_t * const wszFullPathFileName);
    ///load a scene from xml file
    DLL_IN_EXPORT HRESULT LoadScenesFromXML(CVideoSegmentList & sceneList, const wchar_t * const wszFullPathFileName);

}