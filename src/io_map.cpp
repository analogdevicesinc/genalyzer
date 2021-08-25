/*
File        : $HeadURL: https://swdev.cld.analog.com/svn/projects/icdev/sandbox/users/pderouni/platform-upgrades/src/analysis/io_map.cpp $
Originator  : pderouni
Revision    : $Revision: 12767 $
Last Commit : $Date: 2020-08-24 16:38:41 -0400 (Mon, 24 Aug 2020) $
Last Editor : $Author: pderouni $
*/

#include "io_map.hpp"
#include "io_item.hpp"
#include <iomanip>

namespace analysis {

io_map::unique_ptr io_map::read(std::istream& stream)
{
    auto t = peek_obj_type(stream);
    if (Map == t) {
        stream.get();
    } else {
        throw base::exception("Expected " + io_type_map[Map]
            + ", got " + io_type_map[t]);
    }
    auto map = create();
    while (stream) {
        if (yaff_end_map[Map][0] == stream.peek()) {
            stream.get();
            break;
        } else {
            auto key = io_str::read(stream);
            if (yaff_assoc_char != stream.get()) {
                stream.unget();
                throw base::exception("YAFF error reading "
                    + io_type_map[Map]);
            }
            map->insert(key->value, io_object::read(stream));
            if (yaff_sep_char == stream.peek()) {
                stream.get();
            }
        }
    }
    return map;
}

template <typename T>
bool io_map::emplace(const key_type& key, T v)
{
    return insert(key, io_item<T>::create(v));
}

template ICD_ANALYSIS_DECL bool io_map::emplace(const key_type&, bool_type);
template ICD_ANALYSIS_DECL bool io_map::emplace(const key_type&, int_type);
template ICD_ANALYSIS_DECL bool io_map::emplace(const key_type&, real_type);
template ICD_ANALYSIS_DECL bool io_map::emplace(const key_type&, cplx_type);

bool io_map::emplace(const key_type& key, const str_type& v)
{
    return insert(key, io_str::create(v));
}

bool io_map::emplace(const key_type& key, const char* v)
{
    return insert(key, io_str::create(v));
}

bool io_map::insert(const key_type& key, base_ptr obj)
{
    bool inserted = false;
    std::tie(std::ignore, inserted) = m_map.insert(
        std::make_pair(key, std::move(obj)));
    if (inserted) {
        m_keys.push_back(key);
        auto len = static_cast<int>(key.length());
        if (len > m_max_key_length) {
            m_max_key_length = len;
        }
    }
    return inserted;
}

int io_map::max_key_length() const
{
    std::string::size_type len = 0;
    for (const auto& k : m_keys) {
        if (k.length() > len) {
            len = k.length();
        }
    }
    return static_cast<int>(len);
}

void io_map::write_impl(std::ostream& stream) const
{
    stream << yaff_begin_map[Map];
    if (!m_keys.empty()) {
        for (const auto& k : m_keys) {
            io_str key(k);
            key.write(stream);
            stream << yaff_assoc_char;
            at(k).write(stream);
            stream << yaff_sep_char;
        }
        stream.seekp(-1, std::ios_base::end);
    }
    stream << yaff_end_map[Map];
}

bool io_map::equals_impl(const io_object& obj,
    bool ignore_numeric_type) const
{
    if (Map == obj.type()) {
        auto& that = static_cast<const io_map&>(obj);
        if (this->m_keys != that.m_keys) {
            return false;
        }
        for (const auto& k : m_keys) {
            if (!this->at(k).equals(that.at(k), ignore_numeric_type)) {
                return false;
            }
        }
        return true;
    }
    return false;
}

std::string io_map::to_string_impl(int indent) const
{
    std::ostringstream oss;
    oss << yaff_begin_map[Map];
    for (const auto& k : m_keys) {
        oss << '\n'
            << std::setw(indent + 2)
            << std::string()
            << std::setw(m_max_key_length + 2)
            << std::left
            << ('"' + k + '"')
            << " : "
            << at(k).to_string(indent + m_max_key_length + 7);
    }
    if (!empty()) {
        oss << '\n'
            << std::setw(indent) << std::string();
    }
    oss << yaff_end_map[Map];
    return oss.str();
}

const std::string io_map::json_id = "__MAP__";

} // namespace analysis
