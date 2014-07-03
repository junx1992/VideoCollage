
#pragma once

#include <streams.h>

namespace mhl
{
	//	@group media type 
	AM_MEDIA_TYPE *NewMediaType(
        __in ULONG cb = sizeof(AM_MEDIA_TYPE)
        );

	HRESULT CheckMediaExist(
        __in_z LPCWSTR wszFullPathName, 
        __in REFGUID guid
        );
	HRESULT ExtractMediaType(
        __in_z LPCWSTR wszFullPathName, 
        __in REFGUID guid, 
        __deref_out AM_MEDIA_TYPE *pmt
        );
	HRESULT ExtractMediaDuration(
        __in_z LPCWSTR wszFullPathName, 
        __out double &dblDurationOfVideo, 
        __out double &dblDurationOfAudio
        );
	HRESULT ExtractMediaSize(
        __in_z LPCWSTR wszFullPathName, 
        __out UINT &uiWidth, 
        __out UINT &uiHeight
        );
    HRESULT ExtractMediaFrameQuantity(
        __in_z LPCWSTR wszFullPathName,
        __out LONGLONG &llNumOfFrame
        );
}