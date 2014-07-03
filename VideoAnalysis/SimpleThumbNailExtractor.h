/******************************************************************************\

Microsoft Research Asia
Copyright (c) 2007 Microsoft Corporation

Module Name:
  Video Analysis Sdk

Abstract:
  This header file provide the class for Simple video motion thumb nail Extractor 

Notes:
  

History:
  Created on 07/13/2007 by v-huami@microsoft.com
\******************************************************************************/

#pragma once
#include "VxComm.h"

namespace VxCore
{
    struct FileInfo;
}

namespace VideoAnalysis
{
    class CVideoInfo;
    class CShot;
    class CVideoSegmentList;
         
    typedef void( *ProgressHandler)(double progress);

    ///simple thumbnail extractor
	class DLL_IN_EXPORT CSimpleVideoThumbnailExtractor
	{
    public:
		///extract the video thunmbnail for the whole video
        ///[in] videoInfo: the video information about the video which thumbnail is extracted from.
        ///     Such information include: bitrate, filesize, duration, frame-width, frame-height, frame-number;
        ///[in] fileInfo: the video file¡¯s full path name property like extension, name, floder, etc;
        ///[in] sceneList: all scenes detected by scene-detector;
        ///[in] thumbnailFileName: the thumbnail saved file name;
        ///[in] thumbnailDuaration: length(in second) of thumbnail defined by user.If it smaller than duration 
        ///     of video,then the length will be the duration of video;
        ///[in] profileFileName: the DES profile file name. we have provided one along with video analysis sdk
        ///[in] bOutputAudio: whether to output the audio in the thumbnail
        ///[in] handler: user can provide a progress handler function, it will be registered to thumbnail 
        ///     extracting engine.Every 1 second, the engine will notify the handler function the progress
        ///     of current extracing.The handler fuction must defined as following prototype:
		static HRESULT ExtractVideoMotionThumbnailToFile(const CVideoInfo & videoInfo,
                                                const VxCore::FileInfo & fileInfo,
												const CVideoSegmentList & sceneList,
												wchar_t* thumbnailFileName,
												double thumbnailDuaration,
												wchar_t* profileFileName,
												bool bOutputAudio,
                                                ProgressHandler handler = NULL 
												);
        ///preview the motion thumb nail in a window
        ///[in] videoInfo: the video information about the video which thumbnail is extracted from.
        ///     Such information include: bitrate, filesize, duration, frame-width, frame-height, frame-number;
        ///[in] fileInfo: the video file¡¯s full path name property like extension, name, floder, etc;
        ///[in] sceneList: all scenes detected by scene-detector;
        ///[in] thumbnailDuaration: length(in second) of thumbnail defined by user.If it smaller than duration 
        ///     of video,then the length will be the duration of video;
        ///[in] profileFileName: the DES profile file name. we have provided one along with video analysis sdk
        ///[in] bOutputAudio: whether to output the audio in the thumbnail
        ///[in] handler: user can provide a progress handler function, it will be registered to thumbnail 
        ///     extracting engine.Every 1 second, the engine will notify the handler function the progress
        ///     of current extracing.The handler fuction must defined as following prototype:
        static HRESULT  ExtractVideoMotionThumbnailToPreview(const CVideoInfo & videoInfo,
                                                const VxCore::FileInfo & fileInfo,
												const CVideoSegmentList & sceneList,
												double thumbnailDuaration,
												wchar_t* profileFileName,
												bool bOutputAudio,
                                                ProgressHandler handler = NULL 
												);
    };

	class DLL_IN_EXPORT CSimpleShotThumbnailExtractor
	{
    public:
		///extract shot thunmbnail, without transition effect
        ///[in] videoInfo: the video information about the video which thumbnail is extracted from.
        ///     Such information include: bitrate, filesize, duration, frame-width, frame-height, frame-number;
        ///[in] fileInfo: the video file¡¯s full path name property like extension, name, floder, etc;
        ///[in] shot: information about a shot(start-time, end-time, start-frame-id, end-frame-id, key-frame);
        ///[in] thumbnailFileName: the thumbnail saved file name;
        ///[in] thumbnailDuaration: length(in second) of thumbnail defined by user.If it smaller than duration 
        ///     of video,then the length will be the duration of video;
        ///[in] profileFileName: the DES profile file name. we have provided one along with video analysis sdk
        ///[in] bOutputAudio: whether to output the audio in the thumbnail
        ///[in] handler: user can provide a progress handler function, it will be registered to thumbnail 
        ///     extracting engine.Every 1 second, the engine will notify the handler function the progress
        ///     of current extracing.The handler fuction must defined as following prototype:
		static HRESULT ExtractShotMotionThumbnailToFile(const CVideoInfo & videoInfo,
                                               const VxCore::FileInfo & fileInfo,
											   const CShot & shot,
										       wchar_t* thumbnailFileName,
											   double thumbnailDuaration,
											   wchar_t* profileFileName,
											   bool bOutputAudio,
                                               ProgressHandler handler = NULL
											   );

         ///preview the motion thumb nail in a window
        ///[in] videoInfo: the video information about the video which thumbnail is extracted from.
        ///     Such information include: bitrate, filesize, duration, frame-width, frame-height, frame-number;
        ///[in] fileInfo: the video file¡¯s full path name property like extension, name, floder, etc;
        ///[in] shot: information about a shot(start-time, end-time, start-frame-id, end-frame-id, key-frame);
        ///[in] thumbnailDuaration: length(in second) of thumbnail defined by user.If it smaller than duration 
        ///     of video,then the length will be the duration of video;
        ///[in] profileFileName: the DES profile file name. we have provided one along with video analysis sdk
        ///[in] bOutputAudio: whether to output the audio in the thumbnail
        ///[in] handler: user can provide a progress handler function, it will be registered to thumbnail 
        ///     extracting engine.Every 1 second, the engine will notify the handler function the progress
        ///     of current extracing.The handler fuction must defined as following prototype:
         static HRESULT  ExtractShotMotionThumbnailToPreview(const CVideoInfo & videoInfo,
                                               const VxCore::FileInfo & fileInfo,
											   const CShot & shot,
											   double thumbnailDuaration,
											   wchar_t* profileFileName,
											   bool bOutputAudio,
                                               ProgressHandler handler = NULL
											   );
	};

}
