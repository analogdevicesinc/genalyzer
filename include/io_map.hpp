/*
File        : $HeadURL: https://swdev.cld.analog.com/svn/projects/icdev/sandbox/users/pderouni/platform-upgrades/src/analysis/io_map.hpp $
Originator  : pderouni
Revision    : $Revision: 12078 $
Last Commit : $Date: 2019-11-20 10:59:23 -0500 (Wed, 20 Nov 2019) $
Last Editor : $Author: pderouni $
*/

#ifndef ICD_ANALYSIS_IO_MAP_HPP
#define ICD_ANALYSIS_IO_MAP_HPP

#include "io_object.hpp"

namespace analysis {

/**
     * @ingroup Serialization
     * @brief The io_map class
     */
class ICD_ANALYSIS_DECL io_map final : public io_object {
public:
    using unique_ptr = std::unique_ptr<io_map>;
    using key_type = std::string;
    using map = std::map<key_type, base_ptr>;
    using vector = std::vector<key_type>;
    using const_iterator = vector::const_iterator;

public: /// @{ @name Factories
    /**
         * @brief create
         * @return
         */
    static unique_ptr create()
    {
        return std::make_unique<io_map>();
    }

    /// @}

public: /// @{ @name Deserialization
    /**
         * @brief read
         * @param stream
         * @return
         */
    static unique_ptr read(std::istream& stream);

    /// @}

public: /// @{ @name Constructors
    io_map()
        : m_keys{}
        , m_map{}
        , m_max_key_length{ 0 }
    {
    }

    io_map(const io_map& map)
        : io_object{}
        , m_keys(map.m_keys)
        , m_map{}
        , m_max_key_length{ map.m_max_key_length }
    {
        for (const auto& k : m_keys) {
            m_map[k] = map.at(k).clone();
        }
    }

    io_map(io_map&& map)
        : m_keys(std::move(map.m_keys))
        , m_map(std::move(map.m_map))
        , m_max_key_length{ map.m_max_key_length }
    {
        map.clear();
    }

    /// @}

public: /// @{ @name Destructor
    ~io_map() = default;

    /// @}

public: /// @cond
    io_map& operator=(const io_map&) = delete;

    io_map& operator=(io_map&&) = delete;

    /// @endcond

public: /// @{ @name Element Access
    /**
         * @brief at
         * @param key
         * @return
         */
    const io_object& at(const key_type& key) const
    {
        return *m_map.at(key);
    }

    /// @}

public: /// @{ @name Iterators
    const_iterator begin() const
    {
        return m_keys.begin();
    }

    const_iterator end() const
    {
        return m_keys.end();
    }

    /// @}

public: /// @{ @name Capacity
    /**
         * @brief Indicates whether the container is empty or not.
         * @return true if the container is empty, false otherwise
         */
    bool empty() const
    {
        return m_keys.empty();
    }

    /**
         * @brief Returns the number of elements in the container.
         * @return The number of elements in the container
         */
    diff_t size() const
    {
        return static_cast<diff_t>(m_keys.size());
    }

    /// @}

public: /// @{ @name Modifiers
    /**
         * @brief clear
         */
    void clear()
    {
        m_keys.clear();
        m_map.clear();
        m_max_key_length = 0;
    }

    /**
         * @brief emplace
         * @param[in] key
         * @param[in] v
         * @return
         */
    template <typename T>
    bool emplace(const key_type& key, T v);

    /**
         * @brief emplace
         * @param[in] key
         * @param[in] v
         * @return
         */
    bool emplace(const key_type& key, const str_type& v);

    /**
         * @brief emplace
         * @param[in] key
         * @param[in] v
         * @return
         */
    bool emplace(const key_type& key, const char* v);

    /**
         * @brief insert
         * @param[in] key
         * @param[in] obj
         * @return
         */
    bool insert(const key_type& key, base_ptr obj);

    /// @}

public: /// @{ @name Lookup
    /**
         * @brief contains
         * @param key
         * @return
         */
    bool contains(const key_type& key) const
    {
        return m_map.end() != m_map.find(key);
    }

private:
    int max_key_length() const;

private: // io_object overrides
    void write_impl(std::ostream& stream) const override;

    base_ptr clone_impl() const override
    {
        return std::make_unique<io_map>(*this);
    }

    bool equals_impl(const io_object& obj,
        bool ignore_numeric_type) const override;

    std::string to_string_impl(int indent) const override;

    IOType type_impl() const override
    {
        return Map;
    }

private:
    static const std::string json_id;

private:
    vector m_keys;
    map m_map;
    int m_max_key_length;

}; // class io_map

} // namespace analysis

#endif // ICD_ANALYSIS_IO_MAP_HPP
