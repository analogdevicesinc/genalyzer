/*
File        : $HeadURL: https://swdev.cld.analog.com/svn/projects/icdev/sandbox/users/pderouni/platform-upgrades/src/analysis/io_data.cpp $
Originator  : pderouni
Revision    : $Revision: 11416 $
Last Commit : $Date: 2019-03-27 11:58:01 -0400 (Wed, 27 Mar 2019) $
Last Editor : $Author: pderouni $
*/

#include "io_data.hpp"

namespace analysis {

io_data::unique_ptr io_data::read(std::istream&)
{
    throw base::exception("Not implemented");
}

void io_data::write_impl(std::ostream&) const
{
    throw base::exception("Not implemented");
}

bool io_data::equals_impl(const io_object&, bool) const
{
    throw base::exception("Not implemented");
}

std::string io_data::to_string_impl(int) const
{
    throw base::exception("Not implemented");
}

} // namespace analysis
