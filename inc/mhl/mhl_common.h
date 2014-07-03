
#pragma once

#pragma warning(disable : 4819)

#define POINTER_64 __ptr64

#include <windows.h>                //	For window declaration
#include <cguid.h>
#include <atlbase.h>                //	For CComPtr
#include <comutil.h>                //  For _variant_t
#include <qedit.h>                  //  For some DirectShow components, such as IMediaDet, INullRender, IRenderEngine

#include "logout.h"                 //  For LOG macro
#include "commmacro.h"              //  For Safe allocate and release macro

#pragma comment(lib, "wmvcore")     //  For window media API
#pragma comment(lib, "strmiids")    //  For DirectShow GUIDs

//  inline functions
inline BOOL _iswmv(LPCWSTR wszFullPathName)
{
    LPCWSTR ch = wcsrchr(wszFullPathName, L'.');
    if ( ch && _wcsicmp(ch+1, L"wmv") == 0 )
    {
        return TRUE;
    }
    return FALSE;
}

inline long _filebits(LPCWSTR wszFullPathName)
{
	struct _stat64i32 file_stats;
	if ( 0 != _wstat(wszFullPathName, &file_stats) )
    {
	   return 0;
    }
	return file_stats.st_size * 8;
}


