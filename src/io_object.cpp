/*
File        : $HeadURL: https://swdev.cld.analog.com/svn/projects/icdev/sandbox/users/pderouni/platform-upgrades/src/analysis/io_object.cpp $
Originator  : pderouni
Revision    : $Revision: 12767 $
Last Commit : $Date: 2020-08-24 16:38:41 -0400 (Mon, 24 Aug 2020) $
Last Editor : $Author: pderouni $
*/

#include "io_object.hpp"
#include "file_utils.hpp"
#include "io_data.hpp"
#include "io_item.hpp"
#include "io_map.hpp"
#include "io_null.hpp"
#include "io_vector.hpp"
#include <fstream>

namespace analysis {

io_object::base_ptr io_object::load(const std::string& filename)
{
    if (filename.empty()) {
        throw base::exception("Filename empty");
    }
    auto type = utils::get_file_type(filename); // can throw
    switch (type) {
    case FileType::Bin:
    case FileType::Json:
    case FileType::Xml:
        throw base::exception("Binary, Json, Xml not supported");
    case FileType::Yaff: {
        std::ifstream stream(filename);
        if (!stream.is_open()) {
            throw base::exception("Unable to open file: " + filename);
        }
        if (stream.good()) {
            return read(stream);
        }
        break;
    }
    }
    return io_null::create();
}

io_object::base_ptr io_object::read(std::istream& stream)
{
    switch (peek_obj_type(stream)) {
    case Bool:
        return io_bool::read(stream);
    case Cplx:
        return io_cplx::read(stream);
    case Data:
        return io_data::read(stream);
    case Int:
        return io_int::read(stream);
    case Map:
        return io_map::read(stream);
    case Real:
        return io_real::read(stream);
    case Str:
        return io_str::read(stream);
    case Vector:
        return io_vector::read(stream);
    default:
        return io_null::read(stream);
    }
}

void io_object::save(std::string filename, FileType filetype) const
{
    if (filename.empty()) {
        throw base::exception("Filename empty");
    }
    utils::append_file_ext(filename, filetype);
    switch (filetype) {
    case FileType::Bin:
    case FileType::Json:
    case FileType::Xml: {
        throw base::exception("Binary, Json, Xml not supported");
    }
    case FileType::Yaff: {
        std::ofstream stream(filename, std::ios_base::trunc);
        if (!stream.is_open()) {
            throw base::exception("Unable to open file");
        }
        write(stream);
        break;
    }
    }
}

io_object::bool_type io_object::as_bool() const
{
    if (Bool == type()) {
        return static_cast<const io_bool&>(*this).value;
    } else {
        throw base::exception("Bad IO Object cast");
    }
}

io_object::cplx_type io_object::as_cplx() const
{
    if (Cplx == type()) {
        return static_cast<const io_cplx&>(*this).value;
    } else {
        throw base::exception("Bad IO Object cast");
    }
}

const io_data& io_object::as_data() const
{
    if (Data == type()) {
        return static_cast<const io_data&>(*this);
    } else {
        throw base::exception("Bad IO Object cast");
    }
}

io_object::int_type io_object::as_int() const
{
    if (Int == type()) {
        return static_cast<const io_int&>(*this).value;
    } else {
        throw base::exception("Bad IO Object cast");
    }
}

const io_map& io_object::as_map() const
{
    if (Map == type()) {
        return static_cast<const io_map&>(*this);
    } else {
        throw base::exception("Bad IO Object cast");
    }
}

io_null io_object::as_null() const
{
    if (Null == type()) {
        return io_null();
    } else {
        throw base::exception("Bad IO Object cast");
    }
}

io_object::real_type io_object::as_real() const
{
    if (Real == type()) {
        return static_cast<const io_real&>(*this).value;
    } else {
        throw base::exception("Bad IO Object cast");
    }
}

const io_object::str_type& io_object::as_str() const
{
    if (Str == type()) {
        return static_cast<const io_str&>(*this).value;
    } else {
        throw base::exception("Bad IO Object cast");
    }
}

const io_vector& io_object::as_vector() const
{
    if (Vector == type()) {
        return static_cast<const io_vector&>(*this);
    } else {
        throw base::exception("Bad IO Object cast");
    }
}

io_object::IOType io_object::peek_obj_type(std::istream& stream)
{
    std::string c(1, static_cast<char>(stream.peek()));
    if (yaff_begin_map.contains(c)) {
        return yaff_begin_map[c];
    } else {
        throw base::exception("Got invalid YAFF character");
    }
}

const enum_map<io_object::IOType> io_object::io_type_map("IOType",
    { { io_object::Bool, "Bool" },
        { io_object::Cplx, "Cplx" },
        { io_object::Data, "Data" },
        { io_object::Int, "Int" },
        { io_object::Map, "Map" },
        { io_object::Null, "Null" },
        { io_object::Real, "Real" },
        { io_object::Str, "Str" },
        { io_object::Vector, "Vector" } });

const enum_map<io_object::IOType> io_object::yaff_begin_map("IOType",
    { { io_object::Bool, "B" },
        { io_object::Cplx, "C" },
        { io_object::Data, "[" },
        { io_object::Int, "I" },
        { io_object::Map, "{" },
        { io_object::Null, "N" },
        { io_object::Real, "R" },
        { io_object::Str, "S" },
        { io_object::Vector, "(" } });

const enum_map<io_object::IOType> io_object::yaff_end_map("IOType",
    { { io_object::Data, "]" },
        { io_object::Map, "}" },
        { io_object::Vector, ")" } });

} // namespace analysis
