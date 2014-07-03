/******************************************************************************\

Microsoft Research Asia
Copyright (c) 2007 Microsoft Corporation

Module Name:
  Image Sdk

Abstract:
  This header file provide definition of all exception which will be thown in VideoAnalysisSdk

Notes:

History:
  Created on 04/29/2007 by linjuny@microsoft.com
\******************************************************************************/
#pragma once
#include "VxComm.h"
#include <exception>
namespace VideoAnalysis
{
    ///exception in the video segment related class
	class DLL_IN_EXPORT video_segment_exception: public std::exception
	{
	public:
		video_segment_exception(const char* const& what);
		video_segment_exception();
	};

    ///exception in the class CVideoAnalysisEngine
    class DLL_IN_EXPORT video_analysis_engine_exception: public std::exception
	{
	public:
		video_analysis_engine_exception(const char* const& what);
		video_analysis_engine_exception();
	};

    ///exception in the class CVideoAnalysisEngine
    class DLL_IN_EXPORT video_analysis_motioncomposer_exception: public std::exception
	{
	public:
		video_analysis_motioncomposer_exception(const char* const& what);
		video_analysis_motioncomposer_exception();
	};

}