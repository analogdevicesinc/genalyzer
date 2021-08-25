/*
File        : $HeadURL: https://swdev.cld.analog.com/svn/projects/icdev/sandbox/users/pderouni/platform-upgrades/src/analysis/analysis.hpp $
Originator  : pderouni
Revision    : $Revision: 12382 $
Last Commit : $Date: 2020-04-06 11:21:16 -0400 (Mon, 06 Apr 2020) $
Last Editor : $Author: pderouni $
*/

#ifndef ICD_ANALYSIS_ANALYSIS_HPP
#define ICD_ANALYSIS_ANALYSIS_HPP

#include <exception>
#include <sstream>
#include <string>

#ifdef _WIN32
#ifdef EXPORT_API
#define ICD_ANALYSIS_DECL __declspec(dllexport)
#else
#define ICD_ANALYSIS_DECL __declspec(dllimport)
#endif
#else
#define ICD_ANALYSIS_DECL
#endif

/*
 * Header files in the Analysis subsystem should include this header rather
 * than <base/exception.hpp> and <global/global.hpp> directly.  This file also
 * contains the Doxygen @mainpage.  It should not be included in the Doxyfile
 * for other ICD subsystems.
 */

/// @defgroup Constants Constants
/// @defgroup Enumerations Enumerations
/// @defgroup Functions Functions
/// @defgroup Types Types
/** @defgroup Serialization Serialization
 *  @details The Analysis subsystem's serialization class hierarchy provides
 *  facilities for serialization, comparison, and string representation of
 *  heterogeneous data structures.
 */
/// @defgroup Utilities Utilities

/// @defgroup ArrayMath Array Math
/// @ingroup Functions
/// @defgroup CodeDensity Code Density
/// @ingroup Functions
/// @defgroup DFT Discrete Fourier Transform
/// @ingroup Functions
/// @defgroup FormattedData Formatted Data
/// @ingroup Functions
/// @defgroup Processes Processes
/// @ingroup Functions
/// @defgroup Waveforms Waveforms
/// @ingroup Functions

/**
 * @mainpage
 * @details The Analysis subsystem provides the analysis and signal processing
 * functionality of the ICD platform.  This functionality is exposed through
 * the Analysis Commands subsystem.
 *
 * **Dependencies**
 *   - [FFTW 3.3.5]  (http://fftw.org) (libfftw3-3.dll)
 */

/// The **analysis** namespace represents the ICD Analysis subsystem
namespace analysis {
}

namespace base {

/**
     * @class exception
     * @brief Base class for all system exceptions.
     */
class exception : public std::exception {
public:
    /**
         * @brief Constructor.
         */
    exception()
        : m_default_error("Base Exception.")
    {
    }

    /**
         * @brief Overloaded constructor.
         * @param[in] error Error string.
         */
    exception(const std::string& error)
        : exception()
    {
        *this << " \"" << error.c_str() << "\".";
    }

    /**
         * @brief Copy constructor.
         * @param[in] e An exception object.
         */
    exception(const exception& e)
        : m_default_error(e.m_default_error)
        , m_error((e.what() != e.m_default_error) ? e.what() : e.m_error)
    {
    }

public:
    /**
         * @brief Destructor.
         * @note Required to override default destructor in order to maintain
         *       the same throw specifier specified by std::exception.
         */
    virtual ~exception() noexcept {}

    /**
         * @brief Gives a desription of the exception that was thrown.
         * @returns const char* Exception description.
         */
    virtual const char* what() const noexcept override
    {
        if (m_error.empty()) {
            return m_default_error.c_str();
        }
        return m_error.c_str();
    }

public:
    /**
         * @brief Appends the string <i>str</i> to the error string.
         * @param[in] str String.
         * @returns Reference to <i>this</i>.
         */
    exception& operator<<(const char* str)
    {
        std::ostringstream error_stream;
        error_stream << m_error;
        if (!m_error.empty() && m_error.back() != ' ') {
            error_stream << ' ';
        }
        error_stream << str;
        m_error = error_stream.str();
        return *this;
    }

    /**
         * @brief Appends the string <i>str</i> to the error string.
         * @param[in] str String.
         */
    //void append(const char* str)
    //{
    //    this->operator <<(str);
    //}

protected:
    /// @brief Default error string. Deriving classes can set this to the
    ///        default error pertaining to the respective exception. This
    ///        is used by <em>what()</em> in case the compiled error is empty.
    std::string m_default_error;

private:
    /// @brief Compiled error string.
    std::string m_error;

}; // class exception

} // namespace base

#endif // ICD_ANALYSIS_ANALYSIS_HPP
