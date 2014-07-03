#include "Stdafx.h"
#include "Log.h"
#include "SafeLog.h"


namespace VxCore
{
    SafeLog gblErrLog;

    bool LogOpen(const std::string & logName)
    {
            return gblErrLog.Open(logName.c_str());    
    }

    void LogOut(const std::string & msg)
    {
            gblErrLog.Log(msg);
    }    
}