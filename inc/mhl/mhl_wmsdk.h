
/***************************************************************************************************\
Microsoft Research Asia
Copyright (c) 2006 Microsoft Corporation

Module Name:
	This module contains classes for video file sinker configuration
	1. If we want output avi, then AVI Compress API provide to create compression filters
	2. If we want to output window media, then
	2.1 We can get a system profile by WMV System Profile API
	2.2 We can get a custom profile by WMV Custom Profile API. These APIs will enumerate all of custom 
	profile (.prx) in specified folder.
	2.3 We can get a profile copied from source video by CWMVSourceProfile.

History:
	Modified	on 18/05/2006	by hwei@microsoft.com
          
\***************************************************************************************************/

#pragma once

#include <objbase.h>    //  For COM macro
#include <windows.h>    //  For windows macro

#include <comip.h>      //  For _com_ptr_t
#include <comdef.h>     //  For _COM_SMARTPTR_TYPEDEF
#include <initguid.h>   //  For DEFINE_GUID

#include <objbase.h>    //  For DECLARE_INTERFACE
#include <dshowasf.h>   //  For IWMProfile

//*******************************************************************************************************
//	Declare interface
//*******************************************************************************************************
// {1631C6FD-CF91-4a6a-AB01-D0D5F5A081E7}
DEFINE_GUID(IID_IProfilePrintor, 
0x1631c6fd, 0xcf91, 0x4a6a, 0xab, 0x1, 0xd0, 0xd5, 0xf5, 0xa0, 0x81, 0xe7);
DECLARE_INTERFACE_(IProfilePrintor, IUnknown)
{
	STDMETHOD_(void, Print) (THIS_	DWORD dwIdx, LPCWSTR wszName, LPCWSTR wszDesc) PURE;
};

// {1B5AD94E-7338-41ea-86FF-06DD235F8288}
DEFINE_GUID(IID_IProfileManager, 
0x1b5ad94e, 0x7338, 0x41ea, 0x86, 0xff, 0x6, 0xdd, 0x23, 0x5f, 0x82, 0x88);
DECLARE_INTERFACE_(IProfileManager, IUnknown)
{
	STDMETHOD(Init)     (THIS_ LPCWSTR wszReserved)             PURE;
    STDMETHOD(Uninit)   (THIS_ )                                PURE;
    STDMETHOD(List)     (THIS_ IProfilePrintor *pPrinter)       PURE;
    STDMETHOD(Load)     (THIS_ DWORD dwIdx, void **ppObject)    PURE;
};

//*******************************************************************************************************
//	Declare smart pointer 
//*******************************************************************************************************
interface __declspec(uuid("1631C6FD-CF91-4a6a-AB01-D0D5F5A081E7"))   IProfilePrintor;
interface __declspec(uuid("1B5AD94E-7338-41ea-86FF-06DD235F8288"))   IProfileManager;

_COM_SMARTPTR_TYPEDEF(IProfilePrintor, __uuidof(IProfilePrintor));
_COM_SMARTPTR_TYPEDEF(IProfileManager, __uuidof(IProfileManager));

namespace mhl
{
    //*******************************************************************************************************
    //	This class implements IProfilePrintor interface, for outputing profiles to console
    //*******************************************************************************************************    
    class CPrintProfileToConsole : public IProfilePrintor
    {
    public:
        //  IUnknown interface
        STDMETHODIMP QueryInterface(REFIID iid, void **ppvObject)
        {
            (*ppvObject) = this;
            return S_OK;
        }
        STDMETHODIMP_(ULONG) AddRef()
        {
            return 1;
        }
        STDMETHODIMP_(ULONG) Release()
        {
            return 1;
        }
        //  IProfilePrintor interface
        STDMETHODIMP_(void) Print(__in DWORD dwIdx, __in_z LPCWSTR wszName, __in_z LPCWSTR wszDesc)
        {
            wprintf(L"[%02d]%s\n", dwIdx, wszName);
        }
    };

    //*******************************************************************************************************
    //	Profile API
    //*******************************************************************************************************
    HRESULT ProfileManagerInit(__in_z LPCWSTR wszPath);
    HRESULT ProfileManagerUninit();
    HRESULT ProfileManagerSave(__deref_in IWMProfile *pProfile, __in_z LPCWSTR wszProfileFullPathName);

    //*******************************************************************************************************
    //	For avi writer, create a compress filter
    //*******************************************************************************************************
    HRESULT AVICompressList(__deref_in IProfilePrintor *pPrintor);
    HRESULT AVICompressLoad(__out IBaseFilter **ppFilter, __in DWORD dwCompressIdx);
    HRESULT AVICompressCount(__out DWORD &dwCompressCount);

    //*******************************************************************************************************
    //	For wmv writer, load profile of system
    //*******************************************************************************************************
    HRESULT WMVSystemList(__deref_in IProfilePrintor *pPrintor);
    HRESULT WMVSystemLoad(__out IWMProfile **ppProfile, __in DWORD dwProfileIdx);
    HRESULT WMVSystemCount(__out DWORD &dwProfileCount);

    //*******************************************************************************************************
    //	For wmv writer, load profile specified
    //*******************************************************************************************************
    HRESULT WMVCustomList(__deref_in IProfilePrintor *pPrintor);
	HRESULT WMVCustomLoad(__out IWMProfile **ppProfile, __in_z LPCWSTR wszProfileFullPathName);
    HRESULT WMVCustomLoad(__out IWMProfile **ppProfile, __in DWORD dwProfileIdx);
    HRESULT WMVCustomCount(__out DWORD &dwProfileCount);
    HRESULT WMVCustomInit(__in_z LPCWSTR wszCustomProfileFolder);
    HRESULT WMVCustomUninit();

    //*******************************************************************************************************
    //	For wmv writer, load profile from source video
    //*******************************************************************************************************
    HRESULT WMVSourceLoad(
        __out IWMProfile **ppProfile, 
        __in_z LPCWSTR wszVideoFullPathName, 
        __in BOOL fWMVFlag,
        __in BOOL fAudioExist,
        __in DWORD dwBitRate
        );

    //*******************************************************************************************************
    //	Wrapper class for profile API
    //*******************************************************************************************************
    class CAVICompressWrapper : public IProfileManager
    {
    public:
        //  IUnknown interface
        STDMETHOD(QueryInterface)   (REFIID iid, void **ppvObject);
        STDMETHOD_(ULONG, AddRef)   ();
        STDMETHOD_(ULONG, Release)  ();
        //  IProfileManager interface
        STDMETHOD(Init)             (__in_z LPCWSTR wszReserved);
        STDMETHOD(Uninit)           ();
        STDMETHOD(List)             (__deref_in IProfilePrintor *pPrintor);
        STDMETHOD(Load)             (__in DWORD dwIdx, __out void **ppObject);
    };

    class CWMVSystemWrapper : public IProfileManager
    {
    public:
        //  IUnknown interface
        STDMETHOD(QueryInterface)   (REFIID iid, void **ppvObject);
        STDMETHOD_(ULONG, AddRef)   ();
        STDMETHOD_(ULONG, Release)  ();
        //  IProfileManager interface
        STDMETHOD(Init)             (__in_z LPCWSTR wszReserved);
        STDMETHOD(Uninit)           ();
        STDMETHOD(List)             (__deref_in IProfilePrintor *pPrintor);
        STDMETHOD(Load)             (__in DWORD dwIdx, __out void **ppObject);
    };

    class CWMVCustomWrapper : public IProfileManager
    {
    public:
        //  IUnknown interface
        STDMETHOD(QueryInterface)   (REFIID iid, void **ppvObject);
        STDMETHOD_(ULONG, AddRef)   ();
        STDMETHOD_(ULONG, Release)  ();
        //  IProfileManager interface
        STDMETHOD(Init)             (__in_z LPCWSTR wszCustomProfileFolder);
        STDMETHOD(Uninit)           ();
        STDMETHOD(List)             (__deref_in IProfilePrintor *pPrintor);
        STDMETHOD(Load)             (__in DWORD dwIdx, __out void **ppObject);
    };

    class CWMVSourceWrapper : public IProfileManager
    {
    public:
        //  IUnknown interface
        STDMETHOD(QueryInterface)   (REFIID iid, void **ppvObject);
        STDMETHOD_(ULONG, AddRef)   ();
        STDMETHOD_(ULONG, Release)  ();
        //  IProfileManager interface
        STDMETHOD(Init)             (__in_z LPCWSTR wszFullPathName);
        STDMETHOD(Uninit)           ();
        STDMETHOD(List)             (__deref_in IProfilePrintor *pPrintor);
        STDMETHOD(Load)             (__in DWORD dwIdx, __out void **ppObject);

    private:
        BOOL    m_fAudioExist;
        BOOL    m_fSourceIsWmv;
        DWORD   m_dwBitRate;
        WCHAR   m_wszFullPathName[MAX_PATH];
    };
}









