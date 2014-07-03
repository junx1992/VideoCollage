/*************************************************************************\
Microsoft Research Asia
Copyright (c) 2003 Microsoft Corporation

Module Name:
    Utility Lib: General utility helpers
  
Abstract:
    
Notes:
    There is a bug in VC7.1 "wchar.h" line 290
    _CRTIMP intptr_t __cdecl _wfindfirst(wchar_t *, struct _wfinddata_t *);

    This will break build in Unicode mode.
    You should add const before wchar_t* in "wchar.h"
    _CRTIMP intptr_t __cdecl _wfindfirst(const wchar_t *, struct _wfinddata_t *);

    This modification is safe because both MSDN and wchar.c define it with "const"

History:
    Created  on 2003 Aug. 16 by oliver_liyin
          
\*************************************************************************/

#pragma once

#include "utl/Utl_SafeString.hpp"

/// -------------------------------------------------------------
/// Usage:
///     DEFINE_DEBUG_TIMER(some_timer, 2);
///     for(int k = 0; k < K; k ++)
///     {
///         DoSomething();
///         ACCUMULATE_DEBUG_TIMER(some_timer, 0);
///         DoAnotherThing();
///         ACCUMULATE_DEBUG_TIMER(some_timer, 1);
///     }
///     DISPLAY_DEBUG_TIMER(some_timer, 1);
///
#define DEFINE_DEBUG_TIMER(name, N)                             \
    static LARGE_INTEGER name##now, name##last, name##frequency;\
    static LONGLONG name##C[N];                                 \
    for(int k = 0; k < N; k ++) name##C[N] = 0;                 \
    QueryPerformanceFrequency(&name##frequency);                \
    QueryPerformanceCounter(&name##last)

#define ACCUMULATE_DEBUG_TIMER(name, K)                         \
    QueryPerformanceCounter(&name##now);                        \
    name##C[K] += name##now.QuadPart - name##last.QuadPart;     \
    name##last = name##now

#define DISPLAY_DEBUG_TIMER(name, N)                                \
    for(int k = 0; k < N; k ++)                                     \
    {                                                               \
        Trace(_T("%s, Duration[%d] = %.3f\n"),                      \
        _T(#name), k, name##C[k] / float(name##frequency.QuadPart));\
    }


namespace utl
{
    class TimeStamp_Hidden
    {
    public:
        void Reset  () const {}
        void Prefix (...) const {}
        void Stamp  (...) const {}
    };

#ifdef DISABLE_VTL_TRACE
    typedef TimeStamp_Hidden TimeStamp;
#else

    ///  Time stamp: display time stamp and information on console
    class TimeStamp
    {
    public:
        explicit TimeStamp()
        {
            QueryPerformanceFrequency(&m_iFrequency);
            Reset();
        }

        void Reset()
        {
            QueryPerformanceCounter(&m_iLast);
            m_iStart = m_iLast;
            m_szPrefix = m_szMessage = _T("");
        }

        void Prefix(const TCHAR* szPrefix)
        {
            m_szPrefix = szPrefix;
        }

        void Stamp(const TCHAR* szFormat, ...)
        {
            va_list args;
            va_start(args, szFormat);
            m_szMessage.FormatV(szFormat, args);
            va_end(args);

            LARGE_INTEGER iNow = {0};
            QueryPerformanceCounter(&iNow);

            const float flDeltaLast = float(
                iNow.QuadPart - m_iLast.QuadPart) / m_iFrequency.QuadPart;
            const float flDeltaStart = float(
                iNow.QuadPart - m_iStart.QuadPart) / m_iFrequency.QuadPart;

            Trace(_T("%s[D=%.3fs, S=%.3fs] %s\n"),
                (LPCTSTR)m_szPrefix, flDeltaLast, flDeltaStart, (LPCTSTR)m_szMessage);

            m_iLast = iNow;
        }


    protected:
        LARGE_INTEGER m_iStart;
        LARGE_INTEGER m_iLast;
        LARGE_INTEGER m_iFrequency;
        utl::SafeString m_szPrefix;
        utl::SafeString m_szMessage;

        const static size_t m_sizeOfBuffer = 600;
    };

#endif

}   /// namespace utl
