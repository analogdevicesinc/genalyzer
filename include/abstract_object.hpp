/*
File        : $HeadURL: https://swdev.cld.analog.com/svn/projects/icdev/sandbox/users/pderouni/platform-upgrades/src/analysis/abstract_object.hpp $
Originator  : pderouni
Revision    : $Revision: 12191 $
Last Commit : $Date: 2020-01-17 15:04:50 -0500 (Fri, 17 Jan 2020) $
Last Editor : $Author: pderouni $
*/

#ifndef ICD_ANALYSIS_ABSTRACT_OBJECT_HPP
#define ICD_ANALYSIS_ABSTRACT_OBJECT_HPP

#include "analysis.hpp"
#include "enums.hpp"
#include <memory>

namespace analysis {

class io_map;

/**
     * @brief The abstract_object class is the base class for objects that will
     * be managed by the Analysis Commands subsystem.
     */
class ICD_ANALYSIS_DECL abstract_object {
public:
    /// Pointer to uniquely-owned abstract_object
    using object_ptr = std::unique_ptr<abstract_object>;

public: /// @{ @name Deserialization
    /**
         * @brief load
         * @param filename
         * @return
         */
    static object_ptr load(const std::string& filename);

    /**
         * @brief is_fft_analysis_v1
         * @param filename
         * @return
         */
    static bool is_fft_analysis_v1(const std::string& filename);

    /// @}

public: /// @{ @name Constructors
    abstract_object() = default;

    abstract_object(const abstract_object&) = default;

    /// @}

public: /// @{ @name Destructor
    virtual ~abstract_object() = default;

    /// @}

public: /// @{ @name Serialization
    /**
         * @brief save
         * @param filename
         * @param type
         */
    void save(const std::string& filename,
        FileType type = FileType::Xml) const;

    /// @}

public: /// @{ @name Other Member Functions
    /**
         * @brief clone
         * @return
         */
    object_ptr clone() const
    {
        return clone_impl();
    }

    /**
         * @brief equals
         * @param obj
         * @return
         */
    bool equals(const abstract_object& obj) const;

    /**
         * @brief object_type
         * @return
         */
    ObjectType object_type() const
    {
        return object_type_impl();
    }

    /**
         * @brief to_string
         * @return
         */
    std::string to_string() const;

    /// @}

protected:
    static const std::string object_data_key;

protected:
    /**
         * @brief save_state
         * @param state
         */
    virtual void save_state(io_map& state) const;

private:
    virtual object_ptr clone_impl() const = 0;

    virtual ObjectType object_type_impl() const = 0;

}; // class abstract_object

/**
     * @brief operator ==
     * @param lhs
     * @param rhs
     * @return
     * @relates abstract_object
     */
inline bool operator==(const abstract_object& lhs,
    const abstract_object& rhs)
{
    return lhs.equals(rhs);
}

/**
     * @brief operator !=
     * @param lhs
     * @param rhs
     * @return
     * @relates abstract_object
     */
inline bool operator!=(const abstract_object& lhs,
    const abstract_object& rhs)
{
    return !(lhs == rhs);
}

} // namespace analysis

#endif // ICD_ANALYSIS_ABSTRACT_OBJECT_HPP
