#include "StdAfx.h"
#include "VxCoreException.h"

namespace VxCore
{
	feature_access_exception::feature_access_exception(const char* const& what)
			:std::exception(what)
	{}

	feature_access_exception::feature_access_exception()
			:std::exception()
	{}

	feature_extractor_exception::feature_extractor_exception(const char* const& what)
			:std::exception(what)
	{}
	feature_extractor_exception::feature_extractor_exception()
			:std::exception()
	{}

	persistence_exception::persistence_exception(const char* const& what)
			    :std::exception(what)
    {}

    persistence_exception::persistence_exception()
			    :std::exception()
	{}
}