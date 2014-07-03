
/***************************************************************************************************\
Microsoft Research Asia
Copyright (c) 2006 Microsoft Corporation

Module Name:
    This module use DES in dshow to create motion thumbnail

Note:
	At first we use group scene algorihm to group shots together, and choose the most valuable scenes
	as output clip.
	Then we use DES to make clips into a WMV. If the duration of the whole souce video is too shot, 
	then we output entire source video directly. 

History:
    Created						by anonymous
	Modified 	on 05/20/2006	by hwei@microsoft.com
          
\***************************************************************************************************/

#pragma once
#include <vector>
#include <cassert>
#include "VaUtils.h"
#include "VxUtils.h"

struct IWMProfile;

namespace VideoAnalysisHelper
{
    class CVideoEditServiceImp;
}

namespace VideoAnalysis
{
    ///configure parameters for video edit service
	struct DLL_IN_EXPORT VideoEditServiceConfig
	{
		public:
			static const double m_DurationOfTransInSec;  ///for transition expected duration
			static const double m_DurationOfClipInSec;   ///for clip expected duration, must more than c_dblDurationOfTransInSec
		
        public:
            ///do you need the audio output
			bool m_OutputAudio;
            ///do you need to output vidoe effects
			bool m_OutputVideoEffect;  

			///you can set video motion thumbnail duaration by giving it a fixed value or a percentage
			double m_ThumbTransDuarationInSec;
			
            ///thumbnail file(the output video thumbnail file's file-info, like filename, fullpathname, ext, etc.)
            VxCore::FileInfo m_MotionFileInfo;
            ///video file(the input video file's file-info, like filename, fullpathname, ext, etc.)
            VxCore::FileInfo m_VideoFileInfo;
            ///input video file info(size, frame width, bitrate, etc.)			
            CVideoInfo m_VideoInfo;
			
            ///wmv profile
			IWMProfile* m_WMVProfile;
			
		public:
            VideoEditServiceConfig();
			VideoEditServiceConfig(const CVideoInfo& videoInfo, const VxCore::FileInfo& fileInfo, bool bOutputAudio=true, bool bOutputVideoEffect=true);
            VideoEditServiceConfig(const VideoEditServiceConfig& cpy_config);
            VideoEditServiceConfig & operator = (const VideoEditServiceConfig& cpy_config);
		    ~VideoEditServiceConfig();
            
            void SetMotionFileName(WCHAR* MotionFileName);

			HRESULT SetProfileBySystemId(double dwSystemProfileId);
			HRESULT SetProfileByFile(const WCHAR* profileFileName);
            HRESULT GetProfile(IWMProfile** ppProfile)const;
	};

    ///the invtervals, the motion thumbnail is consisted of such intervals
	struct DLL_IN_EXPORT Interval
	{
		double beginTime;
		double endTime;
		Interval()
		{
			beginTime = 0.0;
			endTime = 0.0;
		}
		Interval(double _beginTime, double _endTime)
		{
			beginTime = _beginTime;
			endTime = _endTime;
		}
		bool operator<(const Interval& b)
		{
			assert(beginTime < endTime);
			assert(b.beginTime < b.endTime);
			return endTime < b.beginTime;
		}
	};

    typedef void( *ProgressHandler)(double progress);

    ///CVideoEditService provides low level service for thumbnail extract and preview.
    ///Evevry operation in SimpleVideoThumbnailExtractor is dispatched to VideoEditService.
	class DLL_IN_EXPORT CVideoEditService
	{
	public:
		 CVideoEditService();
		 ~CVideoEditService();

        ///build the video dshow filter graph
        ///[in] config: user should provide the configure parameter 
        ///[in] intervalArray: an array of invtervals when building the filter graph.
        HRESULT BuildGraph(const VideoEditServiceConfig & config, const std::vector<Interval>& intervalArray);
        ///save the motion thumb nail
        ///[in] TimeoutInsec: it is the time when the engine should stop to extract the thumbnail.The default
        ///     value  INFINITE means never stop until the whole video has been processed
        ///[in] handler: it is the progress handler function, the extraction engine will report the processing progress
        ///     to the handler, then the handler itself decides what to do
        HRESULT Save(double TimeoutInsec = INFINITE, ProgressHandler handler = NULL);
        ///preview the motion thumb nail in a window
        ///[in] TimeoutInsec: it is the time when the engine should stop to extract the thumbnail.The default
        ///     value  INFINITE means never stop until the whole video has been processed
        ///[in] handler: it is the progress handler function, the extraction engine will report the processing progress
        ///     to the handler, then the handler itself decides what to do
        HRESULT Preview(double TimeoutInsec = INFINITE, ProgressHandler handler = NULL);

	private:
        VideoAnalysisHelper::CVideoEditServiceImp * m_pImp;
	};

}