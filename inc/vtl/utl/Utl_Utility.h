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

#include <tchar.h>

namespace utl {

    /// Call back for progress bar
    typedef BOOL (CALLBACK *ProgressCallback)(int value,
                                              int minValue,
                                              int maxValue,
                                              const TCHAR* Title);

    /// Center the window to its parent
    /// if parent window is NULL, center to desktop
    void CenterWindow(HWND hWnd);

    /// Find the main window of current process
    /// Which is the root of parent and owner tree
    HWND GetMainWindow();

    /// [ If the given filename exists].
    bool FileExists(LPCTSTR szfilename);

    /// [ Is the given name exists, and it's a folder].
    bool FolderExists(LPCTSTR szfoldername);

    /// [ If the given name is not an existing folder, make one ].
    void MakeSureFolderExists(LPCTSTR szFolder);

//    /// nonstandard extension used: override specifier 'keyword'
//#pragma warning(disable:4481)
//
//    class CProgressHandler : public xtl::IProgress
//    {
//    public:
//        CProgressHandler(ProgressCallback callback)
//            : m_callback(callback), m_message(_T(""))
//        {
//        }
//
//        void SetMessage(const TCHAR* message)
//        {
//            m_message = message;
//        }
//
//    protected:
//        void RealizeValue(int value, int minValue, int maxValue) override
//        {
//            if(m_callback != NULL)
//            {
//                m_callback(value, minValue, maxValue, m_message);
//            }
//        }
//
//    private:
//        ProgressCallback m_callback;
//        const TCHAR* m_message;
//    };

}   /// namespace utl
