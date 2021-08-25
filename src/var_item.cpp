/*
File        : $HeadURL: https://swdev.cld.analog.com/svn/projects/icdev/sandbox/users/pderouni/platform-upgrades/src/analysis/var_item.cpp $
Originator  : pderouni
Revision    : $Revision: 11416 $
Last Commit : $Date: 2019-03-27 11:58:01 -0400 (Wed, 27 Mar 2019) $
Last Editor : $Author: pderouni $
*/

#include "var_item.hpp"
#include "io_item.hpp"

namespace analysis {

template <>
var_data::io_ptr var_bool::save_var() const
{
    return io_bool::create(value);
}

template <>
VarType var_bool::var_type_impl() const
{
    return VarType::Bool;
}

template <>
var_data::io_ptr var_int::save_var() const
{
    return io_int::create(value);
}

template <>
VarType var_int::var_type_impl() const
{
    return VarType::Int;
}

template <>
var_data::io_ptr var_real::save_var() const
{
    return io_real::create(value);
}

template <>
VarType var_real::var_type_impl() const
{
    return VarType::Real;
}

template <>
var_data::io_ptr var_cplx::save_var() const
{
    return io_cplx::create(value);
}

template <>
VarType var_cplx::var_type_impl() const
{
    return VarType::Cplx;
}

template <>
var_data::io_ptr var_str::save_var() const
{
    return io_str::create(value);
}

template <>
VarType var_str::var_type_impl() const
{
    return VarType::Str;
}

template class var_item<var_data::bool_type>;
template class var_item<var_data::int_type>;
template class var_item<var_data::real_type>;
template class var_item<var_data::cplx_type>;
template class var_item<var_data::str_type>;

} // namespace analysis
