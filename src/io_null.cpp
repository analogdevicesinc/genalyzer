/*
File        : $HeadURL: https://swdev.cld.analog.com/svn/projects/icdev/sandbox/users/pderouni/platform-upgrades/src/analysis/io_null.cpp $
Originator  : pderouni
Revision    : $Revision: 12767 $
Last Commit : $Date: 2020-08-24 16:38:41 -0400 (Mon, 24 Aug 2020) $
Last Editor : $Author: pderouni $
*/

#include "io_null.hpp"

namespace analysis {

io_null::unique_ptr io_null::read(std::istream& stream)
{
    auto t = peek_obj_type(stream);
    if (Null == t) {
        stream.get();
    } else {
        throw base::exception("Expected " + io_type_map[Null]
            + ", got " + io_type_map[t]);
    }
    return create();
}

void io_null::write_impl(std::ostream& stream) const
{
    stream << yaff_begin_map[Null];
}

std::string io_null::to_string_impl(int) const
{
    return "null";
}

} // namespace analysis
