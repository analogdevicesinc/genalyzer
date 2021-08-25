/*
File        : $HeadURL: https://swdev.cld.analog.com/svn/projects/icdev/sandbox/users/pderouni/platform-upgrades/src/analysis/io_vector.hpp $
Originator  : pderouni
Revision    : $Revision: 11416 $
Last Commit : $Date: 2019-03-27 11:58:01 -0400 (Wed, 27 Mar 2019) $
Last Editor : $Author: pderouni $
*/

#ifndef ICD_ANALYSIS_IO_VECTOR_HPP
#define ICD_ANALYSIS_IO_VECTOR_HPP

#include "io_object.hpp"

namespace analysis {

/**
     * @ingroup Serialization
     * @brief The io_vector class
     */
class ICD_ANALYSIS_DECL io_vector final : public io_object {
public:
    using unique_ptr = std::unique_ptr<io_vector>;
    using vector = std::vector<base_ptr>;
    using const_iterator = vector::const_iterator;

public: /// @{ @name Factories
    /**
         * @brief create
         * @return
         */
    static unique_ptr create()
    {
        return std::make_unique<io_vector>();
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
    io_vector() = default;

    io_vector(const io_vector& vec)
        : io_object{}
        , m_vector{}
    {
        for (const auto& obj : vec) {
            m_vector.push_back(obj->clone());
        }
    }

    io_vector(io_vector&& vec)
        : m_vector(std::move(vec.m_vector))
    {
        vec.clear();
    }

    /// @}

public: /// @{ @name Destructor
    ~io_vector() = default;

    /// @}

public: /// @cond
    io_vector& operator=(const io_vector&) = delete;

    io_vector& operator=(io_vector&&) = delete;

    /// @endcond

public: /// @{ @name Element Access
    const io_object& at(diff_t i) const
    {
        return *m_vector.at(static_cast<size_t>(i));
    }

    /// @}

public: /// @{ @name Iterators
    const_iterator begin() const
    {
        return m_vector.begin();
    }

    const_iterator end() const
    {
        return m_vector.end();
    }

    /// @}

public: /// @{ @name Capacity
    /**
         * @brief Indicates whether the container is empty or not.
         * @return true if the container is empty, false otherwise
         */
    bool empty() const
    {
        return m_vector.empty();
    }

    /**
         * @brief Returns the number of elements in the container.
         * @return The number of elements in the container
         */
    diff_t size() const
    {
        return static_cast<diff_t>(m_vector.size());
    }

    /// @}

public: /// @{ @name Modifiers
    /**
         * @brief clear
         */
    void clear()
    {
        m_vector.clear();
    }

    /**
         * @brief emplace_back
         * @param[in] v
         */
    template <typename T>
    void emplace_back(T v);

    /**
         * @brief emplace_back
         * @param[in] v
         */
    void emplace_back(const str_type& v);

    /**
         * @brief emplace_back
         * @param[in] v
         */
    void emplace_back(const char* v);

    /**
         * @brief push_back
         * @param[in] obj
         */
    void push_back(base_ptr obj)
    {
        m_vector.push_back(std::move(obj));
    }

    /// @}

private: // io_object overrides
    void write_impl(std::ostream& stream) const override;

    base_ptr clone_impl() const override
    {
        return std::make_unique<io_vector>(*this);
    }

    bool equals_impl(const io_object& obj,
        bool ignore_numeric_type) const override;

    std::string to_string_impl(int indent) const override;

    IOType type_impl() const override
    {
        return Vector;
    }

private:
    vector m_vector;

}; // class io_vector

} // namespace analysis

#endif // ICD_ANALYSIS_IO_VECTOR_HPP
