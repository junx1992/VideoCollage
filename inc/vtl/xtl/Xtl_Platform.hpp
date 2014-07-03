#ifndef __XTL_PLATFORM_HPP__
#define __XTL_PLATFORM_HPP__

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

#ifndef _WIN32
#   pragma message("--- VTL does not support non-win32 platform yet!")
#endif  /*_WIN32*/

#if(_MSC_VER < 1310)
#   pragma message("--- VTL does not work well with MSVC version before 7.1")
#endif

#if (_MANAGED != 1)
/// level 4 warning: using override and sealed without /clr
#pragma warning(disable:4481)
#endif

#if(_MSC_VER < 1400)
#pragma warning(disable:4068) /// unknown pragma region and pragma endregion
#pragma warning(disable:4615) /// unknown user warning type
#pragma warning(disable:4702) /// unreachable code in STL
#pragma warning(disable:4127) /// conditional expression is constant

/// some missing keywords that do not exist before VC++8

#define override
#define sealed
#define abstract

#include "Xtl_Vc2003.hpp"

#endif

/// The trace function display string messages on Console, works as _tprintf
#ifdef DISABLE_VTL_TRACE
inline void Trace(...) {}
#else
inline void Trace(const TCHAR* format, ...)
{
    assert(format != NULL);
    __assume(format != NULL);

    if (format != NULL)
    {
        va_list args;
        va_start(args, format);
        _vtprintf_s(format, args);

        TCHAR string[1024];
        _vstprintf_s(string, 1024, format, args);
        OutputDebugString(string);

        va_end(args);
    }
}

#endif


/// To remove all level 4 warning
/// Some function parameters are only used in assert
/// For example:
///     void resize(size_t _DEBUG_ONLY(newSize)) { assert(size == newSize);}
#ifdef _DEBUG
#   define _DEBUG_ONLY(S) S 
#else
#   define _DEBUG_ONLY(S)
#endif


#endif//__XTL_PLATFORM_HPP__
