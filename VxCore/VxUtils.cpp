#include "StdAfx.h"
#include "VxUtils.h"
#include "windows.h"
#include <shlwapi.h>

namespace VxCore
{
	std::string FormatErrorMessage(DWORD errorCode)
	{
		std::string ret;
		//Buffer that gets the error message string
		HLOCAL hlocal = NULL;  
		//Get the error code's textual description
		BOOL fOk = FormatMessageA(
		  FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, 
		  NULL, errorCode, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), 
		  (LPSTR)&hlocal, 0, NULL);
		if (hlocal != NULL)
		{
			ret = std::string((LPSTR)hlocal);
			LocalFree(hlocal);
		}
		return ret;
	}

        
    //get the filename and ext
	bool ParseFileName(const wchar_t fileName[], wchar_t nameWithoutExt[], wchar_t ext[])
	{
		   const wchar_t* _pch3 = wcsrchr(fileName, L'.');
		   if ( _pch3 )
		   {
				wcscpy_s(ext, VIDEOFILE_EXT_LEN, _pch3+1);
				const wchar_t *p = fileName;
				int i = 0;
				while( (p != _pch3) )
				{
					nameWithoutExt[i++] = *p;
					p++;
				}
				nameWithoutExt[i] = L'\0';
				return true;
		  }
		  return false;
	}

    FileInfo ParseFileFullPathName(const wchar_t *fileFullPathName)
	{
        FileInfo info;
		//file path & name
		swprintf_s(info.m_FullPathName, MAX_PATH, fileFullPathName);

		//file path
		swprintf_s(info.m_FilePath, MAX_PATH, info.m_FullPathName);
		PathRemoveFileSpecW(info.m_FilePath);
				
		//get video file short name
		wchar_t* _pch2 = wcsrchr(info.m_FullPathName, L'\\');
		if ( _pch2 )
			wcscpy_s(info.m_FileName, MAX_PATH, _pch2+1);
		else
			wmemset(info.m_FileName, L'\0', MAX_PATH);

		// Parse file name into prefix and suffix
		if ( !ParseFileName(info.m_FileName, info.m_FileNameWithoutExt, info.m_FileExt) )
		{
			wmemset(info.m_FileNameWithoutExt, L'\0', MAX_PATH);
			wmemset(info.m_FileExt, L'\0', VIDEOFILE_EXT_LEN);
		}

        return info;
	}

    FileInfo::FileInfo()
	{
		wmemset( m_FullPathName, L'\0', MAX_PATH);
		wmemset( m_FilePath, L'\0', MAX_PATH);
		wmemset( m_FileName, L'\0', MAX_PATH);
		wmemset( m_FileNameWithoutExt, L'\0', MAX_PATH);
		wmemset( m_FileExt, L'\0', VIDEOFILE_EXT_LEN);
	}

	FileInfo::FileInfo(const wchar_t *videoFullPathName)
	{
		*this = ParseFileFullPathName(videoFullPathName);       
	}
	   
	FileInfo::FileInfo(const FileInfo & info)
	{
		wcscpy_s( m_FullPathName, MAX_PATH, info.m_FullPathName );
		wcscpy_s( m_FilePath, MAX_PATH, info.m_FilePath );
		wcscpy_s( m_FileName, MAX_PATH, info.m_FileName );
		wcscpy_s( m_FileNameWithoutExt, MAX_PATH, info.m_FileNameWithoutExt );
		wcscpy_s( m_FileExt, VIDEOFILE_EXT_LEN, info.m_FileExt);
	}

	FileInfo & FileInfo::operator = (const FileInfo & info)
	{
		wcscpy_s( m_FullPathName, MAX_PATH, info.m_FullPathName );
		wcscpy_s( m_FilePath, MAX_PATH, info.m_FilePath );
		wcscpy_s( m_FileName, MAX_PATH, info.m_FileName );
		wcscpy_s( m_FileNameWithoutExt, MAX_PATH, info.m_FileNameWithoutExt );
		wcscpy_s( m_FileExt, VIDEOFILE_EXT_LEN, info.m_FileExt);

		return *this;
	}

}