/*
File        : $HeadURL: https://swdev.cld.analog.com/svn/projects/icdev/sandbox/users/pderouni/platform-upgrades/src/analysis/io_item.hpp $
Originator  : pderouni
Revision    : $Revision: 11416 $
Last Commit : $Date: 2019-03-27 11:58:01 -0400 (Wed, 27 Mar 2019) $
Last Editor : $Author: pderouni $
*/

#ifndef ICD_ANALYSIS_IO_ITEM_HPP
#define ICD_ANALYSIS_IO_ITEM_HPP

#include "io_object.hpp"

namespace analysis {

/**
     * @ingroup Serialization
     * @brief The io_item class template
     */
template <typename T>
class ICD_ANALYSIS_DECL io_item final : public io_object {
public:
    using unique_ptr = std::unique_ptr<io_item>;

public: /// @{ @name Factories
    /**
         * @brief create
         * @param[in] v
         * @return
         */
    static unique_ptr create(T v = {})
    {
        return std::make_unique<io_item>(v);
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
    explicit io_item(T v = {})
        : value(v)
    {
    }

    /// @cond
    io_item(const io_item&) = delete;

    io_item(io_item&&) = delete;
    /// @endcond

    /// @}

public: /// @{ @name Destructor
    ~io_item() = default;

    /// @}

public: /// @cond
    io_item& operator=(const io_item&) = delete;

    io_item& operator=(io_item&&) = delete;

    /// @endcond

public:
    T value;

private:
    static void validate(std::istream& stream, IOType type);

private: // io_object overrides
    void write_impl(std::ostream& stream) const override;

    base_ptr clone_impl() const override
    {
        return std::make_unique<io_item>(value);
    }

    bool equals_impl(const io_object& obj,
        bool ignore_numeric_type) const override;

    std::string to_string_impl(int) const override;

    IOType type_impl() const override;

}; // class io_item

/// @{ @name io_item Aliases
using io_bool = io_item<io_object::bool_type>;
using io_int = io_item<io_object::int_type>;
using io_real = io_item<io_object::real_type>;
using io_cplx = io_item<io_object::cplx_type>;
using io_str = io_item<io_object::str_type>;
/// @}

} // namespace analysis

#endif // ICD_ANALYSIS_IO_ITEM_HPP
