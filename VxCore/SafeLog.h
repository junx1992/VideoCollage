/******************************************************************************\

Microsoft Research Asia
Copyright (c) 2007 Microsoft Corporation

Module Name:
  VxCore

Abstract:
  This header file provide the thread-safe log class  

Notes:
  

History:
  Created on 08/28/2007 by v-huami@microsoft.com
\******************************************************************************/

#include<fstream>
#include<string>
#include<Windows.h>

namespace VxCore
{
    class SafeLog
    {
    public:
         SafeLog::SafeLog();
         SafeLog::~SafeLog();
         bool SafeLog::Open(const std::string & logName);
         void  SafeLog::Log(const std::string & msg);

    private:
        //do not allow copy and assign
        SafeLog(const SafeLog &);
        SafeLog & operator = (const SafeLog &);

    private:
        std::ofstream m_LogFile;
        bool m_StdCout;
        CRITICAL_SECTION  m_LogLock;        
    };
}