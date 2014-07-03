#ifndef __XTL_VC2003_HPP__
#define __XTL_VC2003_HPP__

/*************************************************************************\
Oliver Liyin Copyright (c) 2004

Module Name:
    platform related defination

Abstract:

Notes:
    Support win32 and (VC > 7.1) only

Usage:

History:
    Created  on 2004 Nov 17 by t-yinli@microsoft.com

\*************************************************************************/

#pragma once

#include <windows.h>
#include <assert.h>
#include <tchar.h>
#include <stdlib.h>
#include <stdio.h>

/// Define VC2005 features missed in VC2003
/// Including some new keywords, new pragma, and secure string functions
///
#if(_MSC_VER < 1400)

/// following securite string functions are not supported before VC++8

typedef int errno_t;

#define _ftprintf_s _ftprintf
#define _vtprintf_s _vtprintf
#define _vstprintf_s _vstprintf
#define _ftscanf_s _ftscanf

/// In Vc2003, vsprintf takes only 3 parameters, wrap it
inline int vsprintf(char* buffer, size_t /*count*/, const char* format, va_list argptr)
{
    return vsprintf(buffer, format, argptr);
}

inline size_t _tcsnlen(const TCHAR *str, size_t /*maxSize*/)
{
    return _tcslen(str);
}

inline errno_t _tfopen_s(FILE** fp, const TCHAR* szFilename, const TCHAR* szMode)
{
    if(fp == NULL) return E_FAIL;
    *fp = _tfopen(szFilename, szMode);
    return (*fp == NULL) ? E_FAIL : S_OK;
}

inline errno_t _tcscpy_s(TCHAR *strDestination,
                         size_t sizeInBytes,
                         const TCHAR *strSource 
                         )
{
    _tcsncpy(strDestination, strSource, sizeInBytes);
    return 0;
}


inline errno_t _tcscat_s(TCHAR *strDestination,
                         size_t sizeInBytes,
                         const TCHAR *strSource 
                         )
{
    _tcsncat(strDestination, strSource, sizeInBytes);
    return 0;
}

inline errno_t _tcsset_s(TCHAR *str,
                         size_t /*sizeInBytes*/,
                         int c 
                         )
{
    _tcsset(str, (wchar_t) c);
    return 0;
}

inline errno_t _tcsnset_s(TCHAR *str,
                          size_t /*sizeInBytes*/,
                          int c,
                          size_t count 
                          )
{
    _tcsnset(str, (wchar_t) c, count);
    return 0;
}

#define _TRUNCATE size_t(-1)

inline errno_t _vsntprintf_s(TCHAR* buffer,
                             size_t sizeOfBuffer,
                             size_t count,
                             const TCHAR *format,
                             va_list argptr 
                             )
{
    if (count == _TRUNCATE) count = sizeOfBuffer - 1;
    count = __min(count, sizeOfBuffer - 1);
    _vsntprintf(buffer, count, format, argptr);
    return 0;
}

inline errno_t mbstowcs_s(size_t *pConvertedChars,
                          wchar_t *wcstr,
                          size_t /*sizeInWords*/,
                          const char *mbstr,
                          size_t count 
                          )
{
    *pConvertedChars =  mbstowcs(wcstr, mbstr,count);
    return 0;
}

inline errno_t wcstombs_s(size_t *pConvertedChars,
                          char *mbstr,
                          size_t /*sizeInBytes*/,
                          const wchar_t *wcstr,
                          size_t count 
                          )
{
    *pConvertedChars = wcstombs(mbstr, wcstr, count);
    return 0;
}

inline errno_t _tmakepath_s(TCHAR *path, size_t /*length*/,
                            const TCHAR *drive, 
                            const TCHAR *dir, 
                            const TCHAR *fname, 
                            const TCHAR *ext)
{
    if(path == NULL) return -1;
    _tmakepath(path, drive, dir, fname, ext);
    return 0;
}

inline errno_t _tsplitpath_s(const TCHAR *path, 
                             TCHAR *drive,
                             TCHAR *dir,
                             TCHAR *fname,
                             TCHAR *ext )
{
    if(path == NULL || drive == NULL || dir == NULL || fname == NULL || ext == NULL) return -1;
    _tsplitpath(path, drive, dir, fname, ext);
    return 0;
}

inline errno_t _tsplitpath_s(
                             const TCHAR * path,
                             TCHAR * drive,
                             size_t /*driveSizeInCharacters*/,
                             TCHAR * dir,
                             size_t /*dirSizeInCharacters*/,
                             TCHAR * fname,
                             size_t /*nameSizeInCharacters*/,
                             TCHAR * ext, 
                             size_t /*extSizeInCharacters*/
                             )
{
    if(path == NULL || drive == NULL || dir == NULL || fname == NULL || ext == NULL) return -1;
    _tsplitpath(path, drive, dir, fname, ext);
    return 0;
}

#endif

#endif//__XTL_VC2003_HPP__
