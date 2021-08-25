/*
File        : $HeadURL: https://swdev.cld.analog.com/svn/projects/icdev/sandbox/users/pderouni/platform-upgrades/src/analysis/io_item.cpp $
Originator  : pderouni
Revision    : $Revision: 12767 $
Last Commit : $Date: 2020-08-24 16:38:41 -0400 (Mon, 24 Aug 2020) $
Last Editor : $Author: pderouni $
*/

#include "io_item.hpp"
#include "constants.hpp"
#include <iomanip>

namespace analysis {

template <typename T>
void io_item<T>::validate(std::istream& stream, IOType type)
{
    auto t = peek_obj_type(stream);
    if (type == t) {
        stream.get();
    } else {
        throw base::exception("Expected " + io_type_map[type]
            + ", got " + io_type_map[t]);
    }
}

/* For each template instantiation, must define:
     *    read(QXmlStreamReader&)
     *    read(std::istream&)
     *    to_json_impl()
     *    write_impl(QXmlStreamWriter&)
     *    write_impl(std::ostream&)
     *    equals_impl(const io_object&, bool)
     *    to_string_impl(int)
     *    type_impl()
     */

/* io_bool ***************************************************************/

template <>
io_bool::unique_ptr io_bool::read(std::istream& stream)
{
    validate(stream, Bool);
    bool_type x;
    stream >> x;
    if (!stream) {
        throw base::exception("YAFF error reading "
            + io_type_map[Bool]);
    }
    return create(x);
}

template <>
void io_bool::write_impl(std::ostream& stream) const
{
    stream << yaff_begin_map[Bool] << value;
}

template <>
bool io_bool::equals_impl(const io_object& obj, bool) const
{
    return (Bool == obj.type())
        ? (this->value == static_cast<const io_bool&>(obj).value)
        : false;
}

template <>
std::string io_bool::to_string_impl(int) const
{
    std::ostringstream s;
    s << std::boolalpha << value;
    return s.str();
}

template <>
io_object::IOType io_bool::type_impl() const
{
    return Bool;
}

/* io_int ****************************************************************/

template <>
io_int::unique_ptr io_int::read(std::istream& stream)
{
    validate(stream, Int);
    int_type x;
    stream >> std::hex >> x;
    if (!stream) {
        throw base::exception("YAFF error reading "
            + io_type_map[Int]);
    }
    return create(x);
}

template <>
void io_int::write_impl(std::ostream& stream) const
{
    stream << yaff_begin_map[Int] << std::hex << value;
}

template <>
bool io_int::equals_impl(const io_object& obj,
    bool ignore_numeric_type) const
{
    if (Int == obj.type()) {
        return this->value == static_cast<const io_int&>(obj).value;
    } else if (Real == obj.type() && ignore_numeric_type) {
        auto& that = static_cast<const io_real&>(obj);
        return static_cast<real_type>(this->value) == that.value;
    } else {
        return false;
    }
}

template <>
std::string io_int::to_string_impl(int) const
{
    return std::to_string(value);
}

template <>
io_object::IOType io_int::type_impl() const
{
    return Int;
}

/* io_real ***************************************************************/

template <>
io_real::unique_ptr io_real::read(std::istream& stream)
{
    validate(stream, Real);
    real_type x;
    stream >> std::hexfloat >> x;
    if (!stream) {
        throw base::exception("YAFF error reading "
            + io_type_map[Real]);
    }
    return create(x);
}

template <>
void io_real::write_impl(std::ostream& stream) const
{
    stream << yaff_begin_map[Real] << std::hexfloat << value;
}

template <>
bool io_real::equals_impl(const io_object& obj,
    bool ignore_numeric_type) const
{
    if (Real == obj.type()) {
        return this->value == static_cast<const io_real&>(obj).value;
    } else if (Int == obj.type() && ignore_numeric_type) {
        auto& that = static_cast<const io_int&>(obj);
        return this->value == static_cast<real_type>(that.value);
    } else {
        return false;
    }
}

template <>
std::string io_real::to_string_impl(int) const
{
    std::ostringstream s;
    s << std::defaultfloat
      << std::setprecision(real_t_max_digits10)
      << value;
    return s.str();
}

template <>
io_object::IOType io_real::type_impl() const
{
    return Real;
}

/* io_cplx ***************************************************************/

template <>
io_cplx::unique_ptr io_cplx::read(std::istream& stream)
{
    validate(stream, Cplx);
    auto msg = "YAFF error reading " + io_type_map[Cplx];
    real_type re, im;
    stream >> std::hexfloat >> re;
    if (!stream) {
        throw base::exception(msg);
    }
    if (yaff_sep_char != stream.get()) {
        stream.unget();
        throw base::exception(msg);
    }
    stream >> std::hexfloat >> im;
    if (!stream) {
        throw base::exception(msg);
    }
    return create({ re, im });
}

template <>
void io_cplx::write_impl(std::ostream& stream) const
{
    stream << yaff_begin_map[Cplx]
           << std::hexfloat
           << value.real()
           << yaff_sep_char
           << value.imag();
}

template <>
bool io_cplx::equals_impl(const io_object& obj, bool) const
{
    return (Cplx == obj.type())
        ? (this->value == static_cast<const io_cplx&>(obj).value)
        : false;
}

template <>
std::string io_cplx::to_string_impl(int) const
{
    std::ostringstream s;
    s << std::defaultfloat
      << std::setprecision(real_t_max_digits10)
      << value.real()
      << ' ' << ((0 > value.imag()) ? '-' : '+') << " j "
      << std::abs(value.imag());
    return s.str();
}

template <>
io_object::IOType io_cplx::type_impl() const
{
    return Cplx;
}

/* io_str ****************************************************************/

template <>
io_str::unique_ptr io_str::read(std::istream& stream)
{
    validate(stream, Str);
    auto msg = "YAFF error reading " + io_type_map[Str];
    int n = 0;
    stream >> n;
    if (!stream) {
        throw base::exception(msg);
    }
    if (0 > n) {
        throw base::exception(msg);
    }
    if (yaff_assoc_char != stream.get()) {
        stream.unget();
        throw base::exception(msg);
    }
    std::string x(static_cast<std::string::size_type>(n), '\0');
    stream.read(&x[0], n);
    if (!stream) {
        throw base::exception(msg);
    }
    return create(x);
}

template <>
void io_str::write_impl(std::ostream& stream) const
{
    stream << yaff_begin_map[Str]
           << value.length()
           << yaff_assoc_char
           << value;
}

template <>
bool io_str::equals_impl(const io_object& obj, bool) const
{
    return (Str == obj.type())
        ? (this->value == static_cast<const io_str&>(obj).value)
        : false;
}

template <>
std::string io_str::to_string_impl(int) const
{
    return '"' + value + '"';
}

template <>
io_object::IOType io_str::type_impl() const
{
    return Str;
}

/* explicit template instantiation ***************************************/

template class io_item<io_object::bool_type>;
template class io_item<io_object::int_type>;
template class io_item<io_object::real_type>;
template class io_item<io_object::cplx_type>;
template class io_item<io_object::str_type>;

} // namespace analysis
