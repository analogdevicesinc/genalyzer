/*
File        : $HeadURL: https://swdev.cld.analog.com/svn/projects/icdev/sandbox/users/pderouni/platform-upgrades/src/analysis/abstract_object.cpp $
Originator  : pderouni
Revision    : $Revision: 12191 $
Last Commit : $Date: 2020-01-17 15:04:50 -0500 (Fri, 17 Jan 2020) $
Last Editor : $Author: pderouni $
*/

#include "abstract_object.hpp"
#include "enum_maps.hpp"
#include "fft_analysis2.hpp"
#include "io_map.hpp"
#include "var_map.hpp"
#include "var_vector.hpp"

namespace analysis {

abstract_object::object_ptr
abstract_object::load(const std::string& filename)
{
    const io_object::base_ptr state_ptr = io_object::load(filename);
    auto& state = state_ptr->as_map();
    auto& type = state.at(object_type_map.enum_name()).as_str();
    auto& data = state.at(object_data_key);
    switch (object_type_map[type]) {
    case ObjectType::FFTAnalysis: {
        return fft_analysis2::load_state(data.as_map());
    }
    case ObjectType::VarMap:
        return var_map::load_state(data.as_map());
    case ObjectType::VarVector:
        return var_vector::load_state(data.as_vector());
    default:
        throw base::exception("Cannot load object type");
    }
}

bool abstract_object::is_fft_analysis_v1(const std::string& filename)
{
    const io_object::base_ptr state_ptr = io_object::load(filename);
    const io_map& state = state_ptr->as_map();
    const str_t& type = state.at(object_type_map.enum_name()).as_str();
    if (ObjectType::FFTAnalysis == object_type_map[type]) {
        const io_object& data = state.at(object_data_key);
        return !data.as_map().contains("version");
    }
    return false;
}

void abstract_object::save(const std::string& filename,
    FileType type) const
{
    io_map state;
    save_state(state);
    state.save(filename, type);
}

bool abstract_object::equals(const abstract_object& obj) const
{
    io_map this_state, that_state;
    this->save_state(this_state);
    obj.save_state(that_state);
    return this_state == that_state;
}

std::string abstract_object::to_string() const
{
    io_map state;
    save_state(state);
    return state.to_string();
}

const std::string abstract_object::object_data_key = "ObjectData";

void abstract_object::save_state(io_map& state) const
{
    state.emplace(object_type_map.enum_name(),
        object_type_map[object_type()]);
}

} // namespace analysis
