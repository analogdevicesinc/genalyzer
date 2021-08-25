/*
File        : $HeadURL: https://swdev.cld.analog.com/svn/projects/icdev/sandbox/users/pderouni/platform-upgrades/src/analysis/enum_map.hpp $
Originator  : pderouni
Revision    : $Revision: 11594 $
Last Commit : $Date: 2019-05-20 08:38:37 -0400 (Mon, 20 May 2019) $
Last Editor : $Author: pderouni $
*/

#ifndef ICD_ANALYSIS_ENUM_MAP_HPP
#define ICD_ANALYSIS_ENUM_MAP_HPP

#include "analysis.hpp"
#include "text_utils.hpp"
#include <iomanip>
#include <map>
#include <vector>

namespace analysis {

/**
     * @ingroup Utilities
     * @brief A class template that bidirectionally maps enumerators and
     * enumerator names.
     * @tparam E
     * @details Immutable, insertion-ordered, case-insensitive...
     */
template <typename E>
class enum_map {
    static_assert(std::is_enum<E>::value, "Requires enum type.");

public:
    using enum_type = E;
    using name_type = std::string;
    using vector = std::vector<E>;
    using const_iterator = typename vector::const_iterator;
    using underlying_type = typename std::underlying_type<E>::type;

public: /// @{ @name Constructors
    /**
         * @param enum_name
         * @param list
         */
    enum_map(const char* enum_name,
        std::initializer_list<std::pair<E, name_type>> list)
        : m_enum_name(enum_name)
        , m_enums{}
        , m_map{}
        , m_rmap{}
        , m_max_length{ 0 }
    {
        if (m_enum_name.empty()) {
            throw base::exception("Must provide enumeration name");
        }
        for (const auto& kv : list) {
            auto e = kv.first;
            auto& n = kv.second;
            if (n.empty()) {
                throw base::exception("Must provide enumerator name");
            }
            bool inserted;
            std::tie(std::ignore, inserted) = m_map.emplace(e, n);
            if (!inserted) {
                throw base::exception("Enumerator already exists.");
            }
            std::tie(std::ignore, inserted) = m_rmap.emplace(n, e);
            if (!inserted) {
                throw base::exception("Enumerator name already exists.");
            }
            m_enums.push_back(e);
            auto len = static_cast<int>(n.length());
            if (len > m_max_length) {
                m_max_length = len;
            }
        }
    }

    /// @cond
    enum_map() = delete;
    enum_map(const enum_map&) = delete;
    enum_map(enum_map&&) = delete;
    /// @endcond

    /// @}

public: /// @{ @name Destructor
    ~enum_map() = default;

    /// @}

public: /// @{ @name Assignment
    /// @cond
    enum_map& operator=(const enum_map&) = delete;
    enum_map& operator=(enum_map&&) = delete;
    /// @endcond

    /// @}

public: /// @{ @name Element Access
    /**
         * @brief Returns the name of enumerator e.
         * @param e
         * @return The name of enumerator e
         */
    const name_type& operator[](E e) const
    {
        return m_map.at(e);
    }

    /**
         * @brief Returns the enumerator associated with name n.
         * @param n
         * @return The enumerator associated with name n
         */
    E operator[](const name_type& n) const
    {
        return m_rmap.at(n);
    }

    /**
         * @brief Returns the enumerator at index i.
         * @param i
         * @return The enumerator at index i
         */
    E operator[](int i) const
    {
        return m_enums.at(static_cast<typename vector::size_type>(i));
    }

    /// @}

public: /// @{ @name Iterators
    /**
         * @brief Returns a constant iterator to the first element in the
         *        enumerator vector.
         * @return A constant iterator to the first element
         */
    const_iterator begin() const
    {
        return m_enums.begin();
    }

    /**
         * @brief Returns a constant iterator to the element following the last
         *        element in the enumerator vector.
         * @return A constant iterator to the element following the last
         *         element
         */
    const_iterator end() const
    {
        return m_enums.end();
    }

    /// @}

public: /// @{ @name Capacity
    /**
         * @brief Indicates whether the container is empty or not.
         * @return true if the container is empty, false otherwise
         */
    bool empty() const
    {
        return m_enums.empty();
    }

    /**
         * @brief Returns the number of elements in the container.
         * @return The number of elements in the container
         */
    int size() const
    {
        return static_cast<int>(m_enums.size());
    }

    /// @}

public: /// @{ @name Lookup
    /**
         * @brief contains
         * @param e
         * @return
         */
    bool contains(E e) const
    {
        return !(m_map.end() == m_map.find(e));
    }

    /**
         * @brief contains
         * @param n
         * @return
         */
    bool contains(const name_type& n) const
    {
        return !(m_rmap.end() == m_rmap.find(n));
    }

    /// @}

public: /// @{ @name Other Member Functions
    /**
         * @brief Returns the name given to the enumeration type.
         * @return The name given to the enumeration type
         */
    const std::string& enum_name() const
    {
        return m_enum_name;
    }

    /**
         * @brief Returns the qualified name of enumerator e.
         * @return The qualified name of enumerator e
         */
    std::string qual_name(E e) const
    {
        return m_enum_name + "::" + m_map.at(e);
    }

    /**
         * @brief Returns the length of the longest enumerator name.
         * @return The length of the longest enumerator name
         */
    int max_length() const
    {
        return m_max_length;
    }

    /**
         * @brief Returns a string representation of the instance.
         * @param verbose
         * @return A string representation of the instance
         */
    std::string to_string(bool verbose = false) const
    {
        std::ostringstream oss;
        oss << m_enum_name << " {";
        if (verbose) {
            int i = 0;
            oss << '\n';
            for (const auto& kv : m_map) {
                oss << std::setw(4) << std::right << i++ << " : "
                    << std::setw(m_max_length) << std::left << kv.second
                    << " = " << std::setw(3) << std::right
                    << value(kv.first) << '\n';
            }
        } else {
            for (const auto& kv : m_map) {
                oss << ' ' << kv.second << ',';
            }
            if (0 < size()) {
                oss.seekp(-1, std::ios_base::end);
                oss << ' ';
            }
        }
        oss << '}';
        return oss.str();
    }

    /**
         * @brief Returns the underlying value of enumerator e.
         * @param e
         * @return The underlying value of enumerator e
         */
    underlying_type value(E e) const
    {
        return static_cast<underlying_type>(e);
    }

    /// @}

private:
    using map = std::map<E, name_type>;
    using reverse_map = std::map<name_type, E,
        utils::caseless_string_compare>;

    std::string m_enum_name;
    vector m_enums;
    map m_map;
    reverse_map m_rmap;
    int m_max_length;

}; // class enum_map

} // namespace analysis

#endif // ICD_ANALYSIS_ENUM_MAP_HPP
