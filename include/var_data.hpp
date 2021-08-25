/*
File        : $HeadURL$
Originator  : pderouni
Revision    : $Revision$
Last Commit : $Date$
Last Editor : $Author$
*/

#ifndef ICD_ANALYSIS_VAR_DATA_HPP
#define ICD_ANALYSIS_VAR_DATA_HPP

#include "abstract_object.hpp"
#include "type_aliases.hpp"
#include <map>
#include <vector>

namespace analysis {

const std::map<VarType, std::string> var_type_map{
    { VarType::Bool, "Bool" },
    { VarType::Int, "Int" },
    { VarType::Real, "Real" },
    { VarType::Cplx, "Cplx" },
    { VarType::Str, "Str" },
    { VarType::Map, "Map" },
    { VarType::Vector, "Vector" }
};

class io_object;
class var_map;
class var_vector;

/**
     * @brief The var_data class
     */
class ICD_ANALYSIS_DECL var_data : public abstract_object {
    friend class var_map; // needs access to save_var
    friend class var_vector; // needs access to save_var

public:
    /// Pointer to shared var_data
    using data_ptr = std::shared_ptr<var_data>;

    /// Pointer to shared var_map
    using map_ptr = std::shared_ptr<var_map>;

    /// Pointer to shared var_vector
    using vector_ptr = std::shared_ptr<var_vector>;

    using bool_type = io_bool_t;
    using int_type = io_int_t;
    using real_type = io_real_t;
    using cplx_type = io_cplx_t;
    using str_type = io_str_t;

public: /// @{ @name Constructors
    var_data() = default;

    var_data(const var_data&) = default;

    /// @}

public: /// @{ @name Destructor
    virtual ~var_data() = default;

    /// @}

public: /// @{ @name Other Member Functions
    /**
         * @brief clone_var
         * @return
         */
    data_ptr clone_var() const
    {
        return clone_var_impl();
    }

    /**
         * @brief var_type
         * @return
         */
    VarType var_type() const
    {
        return var_type_impl();
    }

    /// @}

protected:
    using io_ptr = std::unique_ptr<io_object>;

private:
    virtual data_ptr clone_var_impl() const = 0;

    virtual io_ptr save_var() const = 0;

    virtual VarType var_type_impl() const = 0;

}; // class var_data

} // namespace analysis

#endif // ICD_ANALYSIS_VAR_DATA_HPP
