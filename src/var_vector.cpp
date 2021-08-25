/*
File        : $HeadURL: https://swdev.cld.analog.com/svn/projects/icdev/sandbox/users/pderouni/platform-upgrades/src/analysis/var_vector.cpp $
Originator  : pderouni
Revision    : $Revision: 11416 $
Last Commit : $Date: 2019-03-27 11:58:01 -0400 (Wed, 27 Mar 2019) $
Last Editor : $Author: pderouni $
*/

#include "var_vector.hpp"
#include "io_map.hpp"
#include "io_vector.hpp"
#include "var_item.hpp"
#include "var_map.hpp"

namespace analysis {

var_vector::unique_ptr var_vector::load(const std::string& filename)
{
    abstract_object::object_ptr obj = abstract_object::load(filename);
    if (ObjectType::VarVector != obj->object_type()) {
        throw base::exception("Not a VarVector Object");
    }
    return unique_ptr(static_cast<var_vector*>(obj.release()));
}

var_vector::bool_type var_vector::as_bool(diff_t i) const
{
    if_wrong_type_throw(i, VarType::Bool);
    return static_cast<const var_bool&>(
        *m_vars.at(static_cast<size_t>(i)))
        .value;
}

var_vector::int_type var_vector::as_int(diff_t i) const
{
    if_wrong_type_throw(i, VarType::Int);
    return static_cast<const var_int&>(
        *m_vars.at(static_cast<size_t>(i)))
        .value;
}

var_vector::real_type var_vector::as_real(diff_t i) const
{
    if_wrong_type_throw(i, VarType::Real);
    return static_cast<const var_real&>(
        *m_vars.at(static_cast<size_t>(i)))
        .value;
}

var_vector::cplx_type var_vector::as_cplx(diff_t i) const
{
    if_wrong_type_throw(i, VarType::Cplx);
    return static_cast<const var_cplx&>(
        *m_vars.at(static_cast<size_t>(i)))
        .value;
}

const var_vector::str_type& var_vector::as_str(diff_t i) const
{
    if_wrong_type_throw(i, VarType::Str);
    return static_cast<const var_str&>(
        *m_vars.at(static_cast<size_t>(i)))
        .value;
}

const var_map& var_vector::as_map(diff_t i) const
{
    if_wrong_type_throw(i, VarType::Map);
    return static_cast<const var_map&>(
        *m_vars.at(static_cast<size_t>(i)));
}

const var_vector& var_vector::as_vector(diff_t i) const
{
    if_wrong_type_throw(i, VarType::Vector);
    return static_cast<const var_vector&>(
        *m_vars.at(static_cast<size_t>(i)));
}

var_vector::map_ptr var_vector::as_shared_map(diff_t i)
{
    if_wrong_type_throw(i, VarType::Map);
    return std::static_pointer_cast<var_map>(
        m_vars.at(static_cast<size_t>(i)));
}

var_vector::vector_ptr var_vector::as_shared_vector(diff_t i)
{
    if_wrong_type_throw(i, VarType::Vector);
    return std::static_pointer_cast<var_vector>(
        m_vars.at(static_cast<size_t>(i)));
}

void var_vector::push_back(bool_type v)
{
    m_vars.push_back(var_bool::create(v));
}

void var_vector::push_back(int_type v)
{
    m_vars.push_back(var_int::create(v));
}

void var_vector::push_back(real_type v)
{
    m_vars.push_back(var_real::create(v));
}

void var_vector::push_back(cplx_type v)
{
    m_vars.push_back(var_cplx::create(v));
}

void var_vector::push_back(str_type v)
{
    m_vars.push_back(var_str::create(std::move(v)));
}

void var_vector::push_back(const char* v)
{
    m_vars.push_back(var_str::create(v));
}

void var_vector::push_back(var_map v)
{
    m_vars.push_back(var_map::create(std::move(v)));
}

void var_vector::push_back(var_vector v)
{
    m_vars.push_back(create(std::move(v)));
}

void var_vector::push_back(data_ptr v)
{
    m_vars.push_back(std::move(v));
}

void var_vector::save_state(io_map& state) const
{
    abstract_object::save_state(state);
    state.insert(object_data_key, save_var());
}

var_vector::io_ptr var_vector::save_var() const
{
    io_ptr io = io_vector::create();
    auto& v = static_cast<io_vector&>(*io);
    for (const auto& vp : m_vars) {
        v.push_back(vp->save_var());
    }
    return io;
}

var_vector::unique_ptr var_vector::load_state(const io_vector& data)
{
    auto vv = create();
    for (const auto& io_obj : data) {
        switch (io_obj->type()) {
        case io_object::Bool:
            vv->push_back(io_obj->as_bool());
            break;
        case io_object::Int:
            vv->push_back(io_obj->as_int());
            break;
        case io_object::Real:
            vv->push_back(io_obj->as_real());
            break;
        case io_object::Cplx:
            vv->push_back(io_obj->as_cplx());
            break;
        case io_object::Str:
            vv->push_back(io_obj->as_str());
            break;
        case io_object::Map:
            vv->push_back(var_map::load_state(io_obj->as_map()));
            break;
        case io_object::Vector:
            vv->push_back(var_vector::load_state(io_obj->as_vector()));
            break;
        }
    }
    return vv;
}

} // namespace analysis
