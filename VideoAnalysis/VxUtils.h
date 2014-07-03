/******************************************************************************\

Microsoft Research Asia
Copyright (c) 2007 Microsoft Corporation

Module Name:
  VxCore

Abstract:
  This header file provide the utils functions  

Notes:
  

History:
  Created on 08/22/2007 by linjuny@microsoft.com
\******************************************************************************/
#pragma once
#include "VxComm.h"
#include <string>

namespace VxCore
{
	DLL_IN_EXPORT std::string FormatErrorMessage(DWORD errorCode);

    ///video file name extension MAX length, such as: wmv(3), rmvb(4), it should be < VIDEOFILE_EXT_LEN
	#define VIDEOFILE_EXT_LEN  5

	
    ///by giving the full path file name, the helper struct parser the video file's info:extesion,name, etc.
	struct DLL_IN_EXPORT FileInfo
	{
			FileInfo();
			FileInfo(const wchar_t *videoFileName);
			FileInfo(const FileInfo & info);
			FileInfo & operator = (const FileInfo & info);

			wchar_t   m_FileExt[VIDEOFILE_EXT_LEN];              /// input file extension
			wchar_t   m_FileNameWithoutExt[MAX_PATH];       /// input file filename without extension
			wchar_t   m_FullPathName[MAX_PATH];             /// input file full path name
			wchar_t   m_FilePath[MAX_PATH];				 /// input file file path
			wchar_t   m_FileName[MAX_PATH];				 /// input file filename without path
	};
	 
    ///parse a file name (like 1213.wmv): 1213 to  nameWithoutExt, wmv to ext
    ///@param fileName [in]: file name (like 1213.wmv)
    ///@param nameWithoutExt [out]: the file name without extention(like 1213)
    ///@param ext [out]: the file name's extention(like wmv)
    DLL_IN_EXPORT bool ParseFileName(const wchar_t fileName[], wchar_t nameWithoutExt[], wchar_t ext[]);

    ///parse a long video full path name
    ///Notice: the input path should be like this "C:\A\B\C", '/' is not allowed
	DLL_IN_EXPORT FileInfo ParseFileFullPathName(const wchar_t *fileFullPathName);

}