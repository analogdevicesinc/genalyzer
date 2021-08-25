/*
File        : $HeadURL: https://swdev.cld.analog.com/svn/projects/icdev/sandbox/users/pderouni/platform-upgrades/src/analysis/io_data.hpp $
Originator  : pderouni
Revision    : $Revision: 11416 $
Last Commit : $Date: 2019-03-27 11:58:01 -0400 (Wed, 27 Mar 2019) $
Last Editor : $Author: pderouni $
*/

#ifndef ICD_ANALYSIS_IO_DATA_HPP
#define ICD_ANALYSIS_IO_DATA_HPP

#include "io_object.hpp"

namespace analysis {

/**
     * @ingroup Serialization
     * @brief The io_data class
     */
class ICD_ANALYSIS_DECL io_data final : public io_object {
public:
    using unique_ptr = std::unique_ptr<io_data>;

public: /// @{ @name Factories
    /**
         * @brief create
         * @return
         */
    static unique_ptr create()
    {
        return std::make_unique<io_data>();
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
    io_data() = default;

    io_data(const io_data& data) = default;

    /// @cond
    io_data(io_data&&) = delete;
    /// @endcond

    /// @}

public: /// @{ @name Destructor
    ~io_data() = default;

    /// @}

public: /// @cond
    io_data& operator=(const io_data&) = delete;

    io_data& operator=(io_data&&) = delete;

    /// @endcond

private: // io_object overrides
    void write_impl(std::ostream& stream) const override;

    base_ptr clone_impl() const override
    {
        return std::make_unique<io_data>(*this);
    }

    bool equals_impl(const io_object& obj, bool) const override;

    std::string to_string_impl(int indent) const override;

    IOType type_impl() const override
    {
        return Data;
    }

}; // class io_data

} // namespace analysis

#endif // ICD_ANALYSIS_IO_DATA_HPP
