/*
File        : $HeadURL: https://swdev.cld.analog.com/svn/projects/icdev/sandbox/users/pderouni/platform-upgrades/src/analysis/io_vector.cpp $
Originator  : pderouni
Revision    : $Revision: 12767 $
Last Commit : $Date: 2020-08-24 16:38:41 -0400 (Mon, 24 Aug 2020) $
Last Editor : $Author: pderouni $
*/

#include "io_vector.hpp"
#include "io_item.hpp"
#include <iomanip>

namespace analysis {

io_vector::unique_ptr io_vector::read(std::istream& stream)
{
    auto t = peek_obj_type(stream);
    if (Vector == t) {
        stream.get();
    } else {
        throw base::exception("Expected " + io_type_map[Vector]
            + ", got " + io_type_map[t]);
    }
    auto vec = create();
    while (stream) {
        if (yaff_end_map[Vector][0] == stream.peek()) {
            stream.get();
            break;
        } else {
            vec->push_back(io_object::read(stream));
            if (yaff_sep_char == stream.peek()) {
                stream.get();
            }
        }
    }
    return vec;
}

template <typename T>
void io_vector::emplace_back(T v)
{
    push_back(io_item<T>::create(v));
}

template ICD_ANALYSIS_DECL void io_vector::emplace_back(bool_type);
template ICD_ANALYSIS_DECL void io_vector::emplace_back(int_type);
template ICD_ANALYSIS_DECL void io_vector::emplace_back(real_type);
template ICD_ANALYSIS_DECL void io_vector::emplace_back(cplx_type);

void io_vector::emplace_back(const str_type& v)
{
    push_back(io_str::create(v));
}

void io_vector::emplace_back(const char* v)
{
    push_back(io_str::create(v));
}

void io_vector::write_impl(std::ostream& stream) const
{
    stream << yaff_begin_map[Vector];
    if (!m_vector.empty()) {
        for (const auto& obj : m_vector) {
            obj->write(stream);
            stream << yaff_sep_char;
        }
        stream.seekp(-1, std::ios_base::end);
    }
    stream << yaff_end_map[Vector];
}

bool io_vector::equals_impl(const io_object& obj,
    bool ignore_numeric_type) const
{
    if (Vector == obj.type()) {
        auto& that = static_cast<const io_vector&>(obj);
        if (this->size() != that.size()) {
            return false;
        }
        for (diff_t i = 0; i < size(); ++i) {
            if (!this->at(i).equals(that.at(i), ignore_numeric_type)) {
                return false;
            }
        }
        return true;
    }
    return false;
}

std::string io_vector::to_string_impl(int indent) const
{
    std::ostringstream oss;
    oss << yaff_begin_map[Vector];
    const auto wid = static_cast<int>(1 + std::floor(std::log10(size())));
    for (size_t i = 0; i < m_vector.size(); ++i) {
        oss << '\n'
            << std::setw(indent + 2)
            << std::string()
            << std::setw(wid + 2)
            << std::right
            << ('[' + std::to_string(i) + ']')
            << " : "
            << m_vector[i]->to_string(indent + wid + 7);
    }
    if (!empty()) {
        oss << '\n'
            << std::setw(indent) << std::string();
    }
    oss << yaff_end_map[Vector];
    return oss.str();
}

} // namespace analysis
