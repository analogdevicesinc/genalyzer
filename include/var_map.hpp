/*
File        : $HeadURL: https://swdev.cld.analog.com/svn/projects/icdev/sandbox/users/pderouni/platform-upgrades/src/analysis/var_map.hpp $
Originator  : pderouni
Revision    : $Revision: 12274 $
Last Commit : $Date: 2020-02-19 06:47:51 -0500 (Wed, 19 Feb 2020) $
Last Editor : $Author: pderouni $
*/

#ifndef ICD_ANALYSIS_VAR_MAP_HPP
#define ICD_ANALYSIS_VAR_MAP_HPP

#include "var_data.hpp"

namespace analysis {

class var_vector;

/**
     * @brief The var_map class is a heterogeneous map-like container.
     */
class ICD_ANALYSIS_DECL var_map final : public var_data {
    friend class abstract_object; // needs access to load_state
    friend class var_vector; // needs access to load_state

public:
    using unique_ptr = std::unique_ptr<var_map>;
    using str_vector = std::vector<str_t>;
    using const_iterator = str_vector::const_iterator;

public: /// @{ @name Factories
    /**
         * @brief create
         * @return
         */
    static unique_ptr create()
    {
        return std::make_unique<var_map>();
    }

    /**
         * @brief create
         * @param vm
         * @return
         */
    static unique_ptr create(var_map vm)
    {
        return std::make_unique<var_map>(std::move(vm));
    }

    /// @}

public: /// @{ @name Deserialization
    /**
         * @brief load
         * @param filename
         * @return
         */
    static unique_ptr load(const std::string& filename);

    /// @}

public: /// @{ @name Constructors
    var_map()
        : m_keys{}
        , m_vars{}
    {
    }

    var_map(const var_map& vm)
        : var_data{}
        , m_keys(vm.m_keys)
        , m_vars{}
    {
        for (const auto& k : m_keys) {
            m_vars[k] = vm.m_vars.at(k)->clone_var();
        }
    }

    var_map(var_map&& vm)
        : m_keys(std::move(vm.m_keys))
        , m_vars(std::move(vm.m_vars))
    {
        vm.clear();
    }

    /// @}

public: /// @{ @name Destructor
    ~var_map() = default;

    /// @}

public: /// @{ @name Assignment
    var_map& operator=(const var_map& vm)
    {
        if (&vm != this) {
            var_map the_copy(vm);
            std::swap(the_copy, *this);
        }
        return *this;
    }

    var_map& operator=(var_map&& vm)
    {
        std::swap(m_keys, vm.m_keys);
        std::swap(m_vars, vm.m_vars);
        return *this;
    }

    /// @}

public: /// @{ @name Element Access
    bool_type as_bool(const str_t& key) const;

    int_type as_int(const str_t& key) const;

    real_type as_real(const str_t& key) const;

    cplx_type as_cplx(const str_t& key) const;

    const str_type& as_str(const str_t& key) const;

    const var_map& as_map(const str_t& key) const;

    const var_vector& as_vector(const str_t& key) const;

    map_ptr as_shared_map(const str_t& key);

    vector_ptr as_shared_vector(const str_t& key);

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
         * @brief empty
         * @return
         */
    bool empty() const
    {
        return m_keys.empty();
    }

    /**
         * @brief size
         * @return
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
        m_vars.clear();
    }

    bool insert(const str_t& key, bool_type v);

    bool insert(const str_t& key, int_type v);

    bool insert(const str_t& key, real_type v);

    bool insert(const str_t& key, cplx_type v);

    bool insert(const str_t& key, str_type v);

    bool insert(const str_t& key, const char* v);

    bool insert(const str_t& key, var_map v);

    bool insert(const str_t& key, var_vector v);

    bool insert(const str_t& key, data_ptr v);

    /// @}

public: /// @{ @name Other Member Functions
    /**
         * @brief contains
         * @param key
         * @return
         */
    bool contains(const str_t& key) const
    {
        return (m_vars.end() != m_vars.find(key));
    }

    /**
         * @brief type
         * @param key
         * @return
         */
    VarType type(const str_t& key) const
    {
        return m_vars.at(key)->var_type();
    }

    /// @}

protected: // abstract_object overrides
    void save_state(io_map& state) const override;

private: // abstract_object overrides
    object_ptr clone_impl() const override
    {
        return std::make_unique<var_map>(*this);
    }

    ObjectType object_type_impl() const override
    {
        return ObjectType::VarMap;
    }

private: // var_data overrides
    data_ptr clone_var_impl() const override
    {
        return std::make_shared<var_map>(*this);
    }

    io_ptr save_var() const override;

    VarType var_type_impl() const override
    {
        return VarType::Map;
    }

private:
    static unique_ptr load_state(const io_map& data);

private:
    void if_wrong_type_throw(const str_t& key, VarType vtype) const
    {
        VarType ktype = type(key);
        if (ktype != vtype) {
            throw base::exception("Bad var_data cast: cannot cast "
                + var_type_map.at(ktype) + " to "
                + var_type_map.at(vtype));
        }
    }

private:
    str_vector m_keys;

    std::map<str_t, data_ptr> m_vars;

}; // class var_map

} // namespace analysis

#endif // ICD_ANALYSIS_VAR_MAP_HPP
