/*
File        : $HeadURL: https://swdev.cld.analog.com/svn/projects/icdev/sandbox/users/pderouni/platform-upgrades/src/analysis/var_map.cpp $
Originator  : pderouni
Revision    : $Revision: 12191 $
Last Commit : $Date: 2020-01-17 15:04:50 -0500 (Fri, 17 Jan 2020) $
Last Editor : $Author: pderouni $
*/

#include "var_map.hpp"
#include "io_map.hpp"
#include "var_item.hpp"
#include "var_vector.hpp"

namespace analysis {

var_map::unique_ptr var_map::load(const std::string& filename)
{
    abstract_object::object_ptr obj = abstract_object::load(filename);
    if (ObjectType::VarMap != obj->object_type()) {
        throw base::exception("Not a VarMap Object");
    }
    return unique_ptr(static_cast<var_map*>(obj.release()));
}

var_map::bool_type var_map::as_bool(const str_t& key) const
{
    if_wrong_type_throw(key, VarType::Bool);
    return static_cast<const var_bool&>(*m_vars.at(key)).value;
}

var_map::int_type var_map::as_int(const str_t& key) const
{
    if_wrong_type_throw(key, VarType::Int);
    return static_cast<const var_int&>(*m_vars.at(key)).value;
}

var_map::real_type var_map::as_real(const str_t& key) const
{
    if_wrong_type_throw(key, VarType::Real);
    return static_cast<const var_real&>(*m_vars.at(key)).value;
}

var_map::cplx_type var_map::as_cplx(const str_t& key) const
{
    if_wrong_type_throw(key, VarType::Cplx);
    return static_cast<const var_cplx&>(*m_vars.at(key)).value;
}

const var_map::str_type& var_map::as_str(const str_t& key) const
{
    if_wrong_type_throw(key, VarType::Str);
    return static_cast<const var_str&>(*m_vars.at(key)).value;
}

const var_map& var_map::as_map(const str_t& key) const
{
    if_wrong_type_throw(key, VarType::Map);
    return static_cast<const var_map&>(*m_vars.at(key));
}

const var_vector& var_map::as_vector(const str_t& key) const
{
    if_wrong_type_throw(key, VarType::Vector);
    return static_cast<const var_vector&>(*m_vars.at(key));
}

var_map::map_ptr var_map::as_shared_map(const str_t& key)
{
    if_wrong_type_throw(key, VarType::Map);
    return std::static_pointer_cast<var_map>(m_vars.at(key));
}

var_map::vector_ptr var_map::as_shared_vector(const str_t& key)
{
    if_wrong_type_throw(key, VarType::Vector);
    return std::static_pointer_cast<var_vector>(m_vars.at(key));
}

bool var_map::insert(const str_t& key, bool_type v)
{
    return contains(key) ? false : insert(key, var_bool::create(v));
}

bool var_map::insert(const str_t& key, int_type v)
{
    return contains(key) ? false : insert(key, var_int::create(v));
}

bool var_map::insert(const str_t& key, real_type v)
{
    return contains(key) ? false : insert(key, var_real::create(v));
}

bool var_map::insert(const str_t& key, cplx_type v)
{
    return contains(key) ? false : insert(key, var_cplx::create(v));
}

bool var_map::insert(const str_t& key, str_type v)
{
    return contains(key) ? false
                         : insert(key, var_str::create(std::move(v)));
}

bool var_map::insert(const str_t& key, const char* v)
{
    return contains(key) ? false : insert(key, var_str::create(v));
}

bool var_map::insert(const str_t& key, var_map v)
{
    return contains(key) ? false : insert(key, create(std::move(v)));
}

bool var_map::insert(const str_t& key, var_vector v)
{
    return contains(key) ? false
                         : insert(key, var_vector::create(std::move(v)));
}

bool var_map::insert(const str_t& key, data_ptr v)
{
    bool inserted = false;
    std::tie(std::ignore, inserted) = m_vars.emplace(key, std::move(v));
    if (inserted) {
        m_keys.push_back(key);
    }
    return inserted;
}

void var_map::save_state(io_map& state) const
{
    abstract_object::save_state(state);
    state.insert(object_data_key, save_var());
}

var_map::io_ptr var_map::save_var() const
{
    io_ptr io = io_map::create();
    auto& m = static_cast<io_map&>(*io);
    for (const auto& k : m_keys) {
        m.insert(k, m_vars.at(k)->save_var());
    }
    return io;
}

var_map::unique_ptr var_map::load_state(const io_map& data)
{
    auto vm = create();
    for (const auto& k : data) {
        const io_object& io_obj = data.at(k);
        switch (io_obj.type()) {
        case io_object::Bool:
            vm->insert(k, io_obj.as_bool());
            break;
        case io_object::Int:
            vm->insert(k, io_obj.as_int());
            break;
        case io_object::Real:
            vm->insert(k, io_obj.as_real());
            break;
        case io_object::Cplx:
            vm->insert(k, io_obj.as_cplx());
            break;
        case io_object::Str:
            vm->insert(k, io_obj.as_str());
            break;
        case io_object::Map:
            vm->insert(k, var_map::load_state(io_obj.as_map()));
            break;
        case io_object::Vector:
            vm->insert(k, var_vector::load_state(io_obj.as_vector()));
            break;
        }
    }
    return vm;
}

} // namespace analysis
