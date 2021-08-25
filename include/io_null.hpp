/*
File        : $HeadURL: https://swdev.cld.analog.com/svn/projects/icdev/sandbox/users/pderouni/platform-upgrades/src/analysis/io_null.hpp $
Originator  : pderouni
Revision    : $Revision: 11416 $
Last Commit : $Date: 2019-03-27 11:58:01 -0400 (Wed, 27 Mar 2019) $
Last Editor : $Author: pderouni $
*/

#ifndef ICD_ANALYSIS_IO_NULL_HPP
#define ICD_ANALYSIS_IO_NULL_HPP

#include "io_object.hpp"

namespace analysis {

/**
     * @ingroup Serialization
     * @brief The io_null class
     */
class ICD_ANALYSIS_DECL io_null final : public io_object {
public:
    using unique_ptr = std::unique_ptr<io_null>;

public: /// @{ @name Factories
    /**
         * @brief create
         * @return
         */
    static unique_ptr create()
    {
        return std::make_unique<io_null>();
    }

    /// @}

public: /// @{ @name Deserialization
    /**
         * @brief read
         * @param[in] stream
         * @return
         */
    static unique_ptr read(std::istream& stream);

    /// @}

public: /// @{ @name Constructors
    io_null() = default;

    /// @cond
    io_null(const io_null&) = delete;
    /// @endcond

    io_null(io_null&&)
        : io_null{}
    {
    }

    /// @}

public: /// @{ @name Destructor
    ~io_null() = default;

    /// @}

public: /// @cond
    io_null& operator=(const io_null&) = delete;

    io_null& operator=(io_null&&) = delete;

    /// @endcond

private: // io_object overrides
    void write_impl(std::ostream& stream) const override;

    base_ptr clone_impl() const override
    {
        return create();
    }

    bool equals_impl(const io_object& obj, bool) const override
    {
        return Null == obj.type();
    }

    std::string to_string_impl(int indent) const override;

    IOType type_impl() const override
    {
        return Null;
    }

}; // class io_null

} // namespace analysis

#endif // ICD_ANALYSIS_IO_NULL_HPP
