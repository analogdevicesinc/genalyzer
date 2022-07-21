#include "version.hpp"

#include <version.h>

#define TO_STRING(s) #s
#define MACRO_TO_STRING(s) TO_STRING(s)

namespace genalyzer_impl {

    std::string_view version_string()
    {
        static const char* s =
            MACRO_TO_STRING(GENALYZER_VERSION_MAJOR) "."
            MACRO_TO_STRING(GENALYZER_VERSION_MINOR) "."
            MACRO_TO_STRING(GENALYZER_VERSION_PATCH);
        return s;
    }

} // namespace genalyzer_impl