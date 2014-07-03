                      /******************************************************************************\

Microsoft Research Asia
Copyright (c) 2007 Microsoft Corporation

Module Name:
  Video Analysis Sdk

Abstract:
  This header file provide the classes for key frame saver

Notes:
   right now, we just save the key frame in jpeg format
  

History:
  Created on 06/22/2007 by v-huami@microsoft.com
\******************************************************************************/

#pragma once
#include "VxComm.h"
#include "VideoParseInterface.h"
#include <Windows.h>

namespace VideoAnalysis
{
    class  IVideoSegment;
     ///the key frame saver class, it save the key frame into JPEG format
     class DLL_IN_EXPORT CKeyframeSaver:  public IVideoParseReceiver
     {
     public:
         ///saveDir is the directory where the key frames are saved, it's the user's reponsibility to make sure
         ///that such directory is validate
         ///when width = 0 or height = 0, the key frames are saved accroding to its original size
         ///otherwise, the frames are saved according to user assigns
         CKeyframeSaver(const wchar_t * const saveDir, int width=0, int height=0);
    public:
         //implemention of  IVideoParseReceiver interface
         virtual HRESULT OnNewSegment(IVideoSegment& segment);
         virtual HRESULT EndOfStream();
    private:
          wchar_t m_wszSaveDir[MAX_PATH];        //the directory where key frames are saved
          int m_Width;                           //the frame width
          int m_Height;                          //the frame height
        };
}