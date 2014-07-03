#include "StdAfx.h"
#include <atlbase.h>
#include <dshow.h>
#include <cassert>
#include "VaUtils.h"
#include "mhl_dshow.h"
using namespace std;

namespace VideoAnalysis
{
	///Com init and un-init helper structure
	struct ComInitHelper
	{
		 ComInitHelper(){ CoInitialize(NULL);  }
		 ~ComInitHelper(){ 	CoUninitialize();  }
	};

	///test whether it is a media file which can be supported 
	bool IsMediaFile(const wchar_t * fileName)
	{
		assert( fileName != NULL );

		ComInitHelper initHelper;

		CComPtr<IGraphBuilder> pGraph = NULL;
		CComPtr<IPin>	pVideoPin = NULL;
		HRESULT hr = pGraph.CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC);
		if( hr != S_OK )
			return false;

		//test wether we can render it
		//Skip to audio pin, just need the video pin
		hr = mhl::GetAVPinOnSourceFilter(pGraph, fileName, &pVideoPin, NULL);
		if( hr != S_OK )
			return false;

		hr = pGraph->Render(pVideoPin);
		if( FAILED(hr) )
			return false;
		else 
			return true;
	}

	HRESULT GetVideoDuration(const wchar_t * fileName, __int64 & duration)
	{
        assert( fileName != NULL );

        ComInitHelper initHelper;

        CComPtr<IGraphBuilder> pGraph;
        HRESULT hr = pGraph.CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC);
        if( hr != S_OK )
        return E_FAIL;

        //test wether we can render it
        hr = pGraph->RenderFile(fileName, NULL);
        if( FAILED(hr) )
        return E_FAIL;

        //get the duration
        CComPtr<IMediaSeeking> pMediaSeeking = NULL;
        pGraph->QueryInterface(IID_IMediaSeeking, (void **)&pMediaSeeking);

        if( FAILED(pMediaSeeking->GetDuration(&duration)) )
        return E_FAIL;

        return S_OK;    
	}
   

    CVideoInfo ParseVideoInfo(const wchar_t *videoFullPathName)
	{
           CVideoInfo info;
			//get duration
			__int64 duration = 0;
			if( GetVideoDuration(videoFullPathName, duration) == S_OK )
				info.SetDuration(duration*1.0/1e+7);
			else
				info.SetDuration(0.0);

			//get bit size
			struct _stat64i32 file_stats;

			if( _wstat(videoFullPathName, &file_stats) == 0 )
                info.SetFileSize(file_stats.st_size);

			//get bit rate
			if( info.GetDuration() == 0.0 )
				info.SetBitRate(0.0);
			else
				info.SetBitRate(info.GetFileSize()*8/ info.GetDuration());

            return info;
	}

	CVideoInfo::CVideoInfo()
	{
		  m_FrameWidth = 0;
		  m_FrameHeight = 0;
		  m_FrameNum = 0;
		  m_Duration = 0.0;
		  m_BitRate = 0.0;
		  m_FileSize = 0;
	}

	CVideoInfo::CVideoInfo(const wchar_t *videoFullPathName)
	{
          *this = ParseVideoInfo(videoFullPathName);
  	}

	CVideoInfo::CVideoInfo(const CVideoInfo & info)
	{
			m_FrameWidth = info.m_FrameWidth;
			m_FrameHeight = info.m_FrameHeight;
			m_FrameNum = info.m_FrameNum;
			m_Duration = info.m_Duration;
			m_BitRate = info.m_BitRate;
			m_FileSize = info.m_FileSize;
	}

	CVideoInfo & CVideoInfo::operator = (const CVideoInfo & info)
	{
			m_FrameWidth = info.m_FrameWidth;
			m_FrameHeight = info.m_FrameHeight;
			m_FrameNum = info.m_FrameNum;
			m_Duration = info.m_Duration;
			m_BitRate= info.m_BitRate;
			m_FileSize = info.m_FileSize;

			return *this;
	}


}