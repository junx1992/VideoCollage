                                                     /******************************************************************************\

Microsoft Research Asia
Copyright (c) 2007 Microsoft Corporation

Module Name:
  Video Analysis Sdk

Abstract:
  This header file provide the classes for region map saver

Notes:
   right now, we just save the key frame in jpeg format
  

History:
  Created on 06/22/2007 by v-huami@microsoft.com
  Updated on 10/24/2013 by v-aotang@microsoft.com
\******************************************************************************/

#pragma once
#include "VideoParseInterface.h"
#include <Windows.h>

namespace VideoAnalysis
{
   class  IVideoSegment;
}

///save the region map extracted by segmentor into a bitmap 
class CRegionMapSaver:  public VideoAnalysis::IVideoParseReceiver
{
public:
    CRegionMapSaver(const wchar_t * const saveDir);

public:
    //implemention of  IVideoParseReceiver interface
    virtual HRESULT OnNewSegment(VideoAnalysis::IVideoSegment& segment);
    virtual HRESULT EndOfStream();

private:
    wchar_t m_wszSaveDir[MAX_PATH];
};
