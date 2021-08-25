/*
File        : $HeadURL: https://swdev.cld.analog.com/svn/projects/icdev/sandbox/users/pderouni/platform-upgrades/src/analysis/io_object.hpp $
Originator  : pderouni
Revision    : $Revision: 12274 $
Last Commit : $Date: 2020-02-19 06:47:51 -0500 (Wed, 19 Feb 2020) $
Last Editor : $Author: pderouni $
*/

#ifndef ICD_ANALYSIS_IO_OBJECT_HPP
#define ICD_ANALYSIS_IO_OBJECT_HPP

#include "analysis.hpp"
#include "enum_map.hpp"
#include "type_aliases.hpp"

namespace analysis {

class io_data;
class io_map;
class io_null;
class io_vector;

template <typename T>
class io_item;

/**
     * @ingroup Serialization
     * @brief The io_object class
     */
class ICD_ANALYSIS_DECL io_object {
public:
    using base_ptr = std::unique_ptr<io_object>;
    using bool_type = io_bool_t;
    using int_type = io_int_t;
    using real_type = io_real_t;
    using cplx_type = io_cplx_t;
    using str_type = io_str_t;

public:
    enum IOType {
        Bool,
        Cplx,
        Data,
        Int,
        Map,
        Null,
        Real,
        Str,
        Vector
    };

public: /// @{ @name Deserialization
    /**
         * @brief load
         * @param[in] filename
         * @return
         */
    static base_ptr load(const std::string& filename);

    /**
         * @brief read
         * @param[in] stream
         * @return
         */
    static base_ptr read(std::istream& stream);

    /// @}

public: /// @{ @name Constructors
    io_object() = default;

    io_object(const io_object&) = default;

    /// @cond
    io_object(io_object&&) = delete;
    /// @endcond

    /// @}

public: /// @{ @name Destructor
    virtual ~io_object() = default;

    /// @}

public: /// @{ @name Serialization
    /**
         * @brief save
         * @param[in] filename
         * @param[in] type
         * @param[in] opt
         */
    void save(std::string filename, FileType type = FileType::Xml) const;

    /**
         * @brief write
         * @param[in] stream
         */
    void write(std::ostream& stream) const
    {
        write_impl(stream);
    }

    /// @}

public: /// @{ @name Type Casting
    bool_type as_bool() const;

    cplx_type as_cplx() const;

    const io_data& as_data() const;

    int_type as_int() const;

    const io_map& as_map() const;

    io_null as_null() const;

    real_type as_real() const;

    const str_type& as_str() const;

    const io_vector& as_vector() const;

    /// @}

public: /// @{ @name Other Member Functions
    /**
         * @brief clone
         * @return
         */
    base_ptr clone() const
    {
        return clone_impl();
    }

    /**
         * @brief equals
         * @param[in] obj
         * @param[in] ignore_numeric_type
         * @return
         */
    bool equals(const io_object& obj,
        bool ignore_numeric_type = false) const
    {
        return equals_impl(obj, ignore_numeric_type);
    }

    /**
         * @brief to_string
         * @param[in] indent
         * @return
         */
    std::string to_string(int indent = 0) const
    {
        return to_string_impl(indent);
    }

    /**
         * @brief type
         * @return
         */
    IOType type() const
    {
        return type_impl();
    }

    /**
         * @brief type_str
         * @return
         */
    const std::string& type_str() const
    {
        return io_type_map[type()];
    }

    /// @}

protected:
    static IOType peek_obj_type(std::istream& stream);

protected:
    static const enum_map<IOType> io_type_map;

    static const enum_map<IOType> yaff_begin_map;

    static const enum_map<IOType> yaff_end_map;

    static const char yaff_assoc_char = ':';

    static const char yaff_sep_char = ',';

private:
    virtual void write_impl(std::ostream& stream) const = 0;

    virtual base_ptr clone_impl() const = 0;

    virtual bool equals_impl(const io_object& obj,
        bool ignore_numeric_type) const = 0;

    virtual std::string to_string_impl(int indent) const = 0;

    virtual IOType type_impl() const = 0;

}; // class io_object

/**
     * @brief operator ==
     * @param lhs
     * @param rhs
     * @return
     * @relates io_object
     */
inline bool operator==(const io_object& lhs, const io_object& rhs)
{
    return lhs.equals(rhs);
}

/**
     * @brief operator !=
     * @param lhs
     * @param rhs
     * @return
     * @relates io_object
     */
inline bool operator!=(const io_object& lhs, const io_object& rhs)
{
    return !(lhs == rhs);
}

} // namespace analysis

#endif // ICD_ANALYSIS_IO_OBJECT_HPP
