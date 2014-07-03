/******************************************************************************\

Microsoft Research Asia
Copyright (c) 2007 Microsoft Corporation

Module Name:
  VxCore

Abstract:
  This header file provide the thread-safe log functions  

Notes:
  

History:
  Created on 08/28/2007 by v-huami@microsoft.com
  Modified on 10/23/2013 by v-aotang@microsoft.com
\******************************************************************************/

#include "VxComm.h"
#include <string>

namespace VxCore
{
    ///open anywhere you like, but you MUST call LogOpen before LogOut
    ///if you call LogOpen many times, current call will cause previous call to close  the previous log file.
    ///What't more, it is thread safe.
    ///and you can use "stdcout" to let the log print on your console window
    ///if LogOpen fail to open a log file specialized file name by "logName", it will return false
    ///it return true when everything is OK.
    bool DLL_IN_EXPORT LogOpen(const std::string & logName);
    ///when you have any log just call LogOut, but DO REMBER to call LogOpen first
    void DLL_IN_EXPORT LogOut(const std::string & msg);
}