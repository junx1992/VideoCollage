#include "StdAfx.h"
#include "VideoAnalysisException.h"

namespace VideoAnalysis
{
    video_segment_exception::video_segment_exception(const char* const& what)
			:std::exception(what)
	{}

    video_segment_exception::video_segment_exception()
			:std::exception()
	{}

    video_analysis_engine_exception::video_analysis_engine_exception(const char* const& what)
			:std::exception(what)
	{}

    video_analysis_engine_exception::video_analysis_engine_exception()
			:std::exception()
	{}

    video_analysis_motioncomposer_exception::video_analysis_motioncomposer_exception(const char* const& what)
			:std::exception(what)
	{}

    video_analysis_motioncomposer_exception::video_analysis_motioncomposer_exception()
			:std::exception()
	{}
}