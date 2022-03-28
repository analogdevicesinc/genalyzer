#include "manager.hpp"

#include "enum_maps.hpp"
#include "exceptions.hpp"
#include "formatted_data.hpp"
#include "utils.hpp"

#include <map>
#include <regex>

namespace dcanalysis_impl::manager {

    static std::map<str_t, object::pointer> object_map;

    void clear()
    {
        object_map.clear();
    }

    bool equal(const str_t& key1, const str_t& key2)
    {
        contains(key1, true);
        contains(key2, true);
        const object& obj1 = *object_map.at(key1);
        const object& obj2 = *object_map.at(key2);
        return obj1.equals(obj2);
    }

    bool contains(const str_t& key, bool throw_if_not_found)
    {
        bool not_found = object_map.end() == object_map.find(key);
        if (not_found && throw_if_not_found) {
            throw runtime_error("manager::contains : key '" + key + "' not found");
        }
        return !not_found;
    }

    void remove(const str_t& key)
    {
        object_map.erase(key);
    }

    str_t save(const str_t& key, const str_t& filename)
    {
        contains(key, true);
        str_t fn = get_filename_from_object_key(key, filename);
        object_map.at(key)->save(fn);
        return fn;
    }

    size_t size()
    {
        return object_map.size();
    }

    str_t to_string(const str_t& key)
    {
        if (contains(key)) {
            return object_map.at(key)->to_string();
        } else {
            std::vector<str_vector> header { {"Key", object_type_map.name()} };
            std::vector<str_vector> rows;
            for (const auto& kv : object_map) {
                const str_t& obj_key = kv.first;
                ObjectType obj_type = object_map.at(obj_key)->object_type();
                rows.push_back( {obj_key, object_type_map.at(to_int(obj_type))} );
            }
            return table(header, rows, 2, true, true);
        }
    }

    str_t type_str(const str_t& key)
    {
        return object_type_map.at(to_int(type(key)));
    }

} // namespace dcanalysis::manager

namespace dcanalysis_impl::manager {

    void add_object(const str_t& key, object::pointer obj, bool replace)
    {
        if (!std::regex_match(key, std::regex(key_pattern))) {
            throw runtime_error("manager::add_object : invalid key");
        }
        if (!obj) {
            throw runtime_error("manager::add_object : object is null");
        }
        if (contains(key)) {
            if (replace) {
                object_map.erase(key);
            } else {
                throw runtime_error("manager::add_object : key already exists");
            }
        }
        object_map.insert({key, std::move(obj)});
    }

    str_t get_filename_from_object_key(const str_t& key, str_t filename)
    {
        static const std::regex json_ext_pat {".+[.]json$", std::regex::icase};
        if (filename.empty()) {
            filename = key;
        }
        if (!std::regex_match(filename, json_ext_pat)) {
            filename += ".json";
        }
        return filename;
    }

    object::pointer get_object(const str_t& key)
    {
        contains(key, true);
        object::pointer obj = object_map.at(key);
        return obj;
    }

    ObjectType type(const str_t& key)
    {
        contains(key, true);
        return object_map.at(key)->object_type();
    }

} // namespace dcanalysis::manager