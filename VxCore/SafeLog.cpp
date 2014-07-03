#include "Stdafx.h"
#include "SafeLog.h"
#include <iostream>

namespace VxCore
{
     SafeLog::SafeLog()
     {
          InitializeCriticalSection(&m_LogLock); 
          m_StdCout = false;
     }
     
     SafeLog::~SafeLog()
     {
          DeleteCriticalSection(&m_LogLock);
          if( m_LogFile.is_open() )
              m_LogFile.close();
     }

     bool SafeLog::Open(const std::string & logName)
     {
          if( logName.empty() ) return false;

          //we support the "cout" output to console window
          if( logName == "stdcout" )
          {
              m_StdCout = true;
              return true;
          }

          if( m_LogFile.is_open() )
              m_LogFile.close();
          m_LogFile.open(logName.c_str());

          if( m_LogFile )
          {
              //set cout to be false, avoid output to console window
              m_StdCout = false;
              return true;
          } else
              return false;
     }
 
     void  SafeLog::Log(const std::string & msg)
     {
           if( !m_LogFile.is_open() && !m_StdCout ) return;

           EnterCriticalSection(&m_LogLock); 
           if( m_StdCout )
           {
               std::cout<<msg<<std::endl;
               std::cout.flush();
           } else {
               m_LogFile<<msg<<std::endl;
               m_LogFile.flush();
           }
           LeaveCriticalSection(&m_LogLock);  
     }
}