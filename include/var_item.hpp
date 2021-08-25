/*
File        : $HeadURL: https://swdev.cld.analog.com/svn/projects/icdev/sandbox/users/pderouni/platform-upgrades/src/analysis/var_item.hpp $
Originator  : pderouni
Revision    : $Revision: 11416 $
Last Commit : $Date: 2019-03-27 11:58:01 -0400 (Wed, 27 Mar 2019) $
Last Editor : $Author: pderouni $
*/

#ifndef ICD_ANALYSIS_VAR_ITEM_HPP
#define ICD_ANALYSIS_VAR_ITEM_HPP

#include "var_data.hpp"

namespace analysis {

/**
     * @brief The var_item class template
     * @tparam T
     */
template <typename T>
class var_item final : public var_data {
public:
    static std::unique_ptr<var_item> create(T v)
    {
        return std::make_unique<var_item>(v);
    }

public: /// @{ @name Constructors
    explicit var_item(T v)
        : value(v)
    {
    }

    /// @cond
    var_item(const var_item&) = delete;
    var_item(var_item&&) = delete;
    /// @endcond

    /// @}

public: /// @{ @name Destructor
    ~var_item() = default;

    /// @}

public: /// @{ @name Assignment
    /// @cond
    var_item& operator=(const var_item&) = delete;
    var_item& operator=(var_item&&) = delete;
    /// @endcond

    /// @}

public:
    T value;

protected: // abstract_object overrides
    void save_state(io_map&) const override
    {
        throw base::exception("VarItem cannot be saved by itself");
    }

private: // abstract_object overrides
    object_ptr clone_impl() const override
    {
        return std::make_unique<var_item>(value);
    }

    ObjectType object_type_impl() const override
    {
        return ObjectType::VarItem;
    }

private: // var_data overrides
    data_ptr clone_var_impl() const override
    {
        return std::make_shared<var_item>(value);
    }

    io_ptr save_var() const override;

    VarType var_type_impl() const override;

}; // class var_item

/// @{ @name var_item Aliases
using var_bool = var_item<var_data::bool_type>;
using var_int = var_item<var_data::int_type>;
using var_real = var_item<var_data::real_type>;
using var_cplx = var_item<var_data::cplx_type>;
using var_str = var_item<var_data::str_type>;
/// @}

} // namespace analysis

#endif // ICD_ANALYSIS_VAR_ITEM_HPP
