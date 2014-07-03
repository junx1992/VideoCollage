#include "StdAfx.h"
#include "ImageAnalysisException.h"

namespace ImageAnalysis
{
    image_process_exception::image_process_exception(const char* const& what)
			:std::exception(what)
	{}

    image_process_exception::image_process_exception()
			:std::exception()
	{}

   
}