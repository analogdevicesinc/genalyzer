/*
File        : $HeadURL: https://swdev.cld.analog.com/svn/projects/icdev/sandbox/users/pderouni/platform-upgrades/src/analysis/var_vector.hpp $
Originator  : pderouni
Revision    : $Revision: 11416 $
Last Commit : $Date: 2019-03-27 11:58:01 -0400 (Wed, 27 Mar 2019) $
Last Editor : $Author: pderouni $
*/

#ifndef ICD_ANALYSIS_VAR_VECTOR_HPP
#define ICD_ANALYSIS_VAR_VECTOR_HPP

#include "var_data.hpp"

namespace analysis {

class io_vector;
class var_map;

/**
     * @brief The var_vector class is a heterogeneous vector-like container.
     */
class ICD_ANALYSIS_DECL var_vector final : public var_data {
    friend class abstract_object; // needs access to load_state
    friend class var_map; // needs access to load_state

public:
    using unique_ptr = std::unique_ptr<var_vector>;

public: /// @{ @name Factories
    /**
         * @brief create
         * @return
         */
    static unique_ptr create()
    {
        return std::make_unique<var_vector>();
    }

    /**
         * @brief create
         * @param vv
         * @return
         */
    static unique_ptr create(var_vector vv)
    {
        return std::make_unique<var_vector>(std::move(vv));
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
    var_vector()
        : m_vars{}
    {
    }

    var_vector(const var_vector& vv)
        : var_data{}
        , m_vars{}
    {
        for (const data_ptr& vp : vv.m_vars) {
            m_vars.push_back(vp->clone_var());
        }
    }

    var_vector(var_vector&& vv)
        : m_vars(std::move(vv.m_vars))
    {
        vv.clear();
    }

    /// @}

public: /// @{ @name Destructor
    ~var_vector() = default;

    /// @}

public: /// @{ @name Assignment
    var_vector& operator=(const var_vector& vv)
    {
        if (&vv != this) {
            var_vector the_copy(vv);
            std::swap(the_copy, *this);
        }
        return *this;
    }

    var_vector& operator=(var_vector&& vv)
    {
        std::swap(m_vars, vv.m_vars);
        return *this;
    }

    /// @}

public: /// @{ @name Element Access
    bool_type as_bool(diff_t i) const;

    int_type as_int(diff_t i) const;

    real_type as_real(diff_t i) const;

    cplx_type as_cplx(diff_t i) const;

    const str_type& as_str(diff_t i) const;

    const var_map& as_map(diff_t i) const;

    const var_vector& as_vector(diff_t i) const;

    map_ptr as_shared_map(diff_t i);

    vector_ptr as_shared_vector(diff_t i);

    /// @}

public: /// @{ @name Capacity
    /**
         * @brief empty
         * @return
         */
    bool empty() const
    {
        return m_vars.empty();
    }

    /**
         * @brief size
         * @return
         */
    diff_t size() const
    {
        return static_cast<diff_t>(m_vars.size());
    }

    /// @}

public: /// @{ @name Modifiers
    /**
         * @brief clear
         */
    void clear()
    {
        m_vars.clear();
    }

    void push_back(bool_type v);

    void push_back(int_type v);

    void push_back(real_type v);

    void push_back(cplx_type v);

    void push_back(str_type v);

    void push_back(const char* v);

    void push_back(var_map v);

    void push_back(var_vector v);

    void push_back(data_ptr v);

    /// @}

public: /// @{ @name Other Member Functions
    /**
         * @brief type
         * @param i
         * @return
         */
    VarType type(diff_t i) const
    {
        return m_vars.at(static_cast<size_t>(i))->var_type();
    }

    /// @}

protected: // abstract_object overrides
    void save_state(io_map& state) const override;

private: // abstract_object overrides
    object_ptr clone_impl() const override
    {
        return std::make_unique<var_vector>(*this);
    }

    ObjectType object_type_impl() const override
    {
        return ObjectType::VarVector;
    }

private: // var_data overrides
    data_ptr clone_var_impl() const override
    {
        return std::make_shared<var_vector>(*this);
    }

    io_ptr save_var() const override;

    VarType var_type_impl() const override
    {
        return VarType::Vector;
    }

private:
    static unique_ptr load_state(const io_vector& data);

private:
    void if_wrong_type_throw(diff_t i, VarType vtype) const
    {
        VarType ktype = type(i);
        if (ktype != vtype) {
            throw base::exception("Bad var_data cast: cannot cast "
                + var_type_map.at(ktype) + " to "
                + var_type_map.at(vtype));
        }
    }

private:
    std::vector<data_ptr> m_vars;

}; // class var_vector

} // namespace analysis

#endif // ICD_ANALYSIS_VAR_VECTOR_HPP
