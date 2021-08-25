/*
File        : $HeadURL: https://swdev.cld.analog.com/svn/projects/icdev/sandbox/users/pderouni/platform-upgrades/src/analysis/type_aliases.hpp $
Originator  : pderouni
Revision    : $Revision: 12382 $
Last Commit : $Date: 2020-04-06 11:21:16 -0400 (Mon, 06 Apr 2020) $
Last Editor : $Author: pderouni $
*/

#ifndef ICD_ANALYSIS_TYPE_ALIASES_HPP
#define ICD_ANALYSIS_TYPE_ALIASES_HPP

#include <complex>
#include <cstddef>
#include <cstdint>
#include <string>

namespace analysis {

/// @{ @name Common Types

/// @ingroup Types

using diff_t = std::ptrdiff_t; ///< Alias for std::ptrdiff_t
using size_t = std::size_t; ///< Alias for std::size_t
using real_t = double; ///< Real number
using cplx_t = std::complex<real_t>; ///< Complex number
using str_t = std::string; ///< Alias for std::string
/// @}

/// @{ @name Signed Integers

/// @ingroup Types

using int16_t = std::int_least16_t; ///< Signed 16-bit integer
using int32_t = std::int_least32_t; ///< Signed 32-bit integer
using int64_t = std::int_least64_t; ///< Signed 64-bit integer
/// @}

/// @{ @name Unsigned Integers

/// @ingroup Types

using uint16_t = std::uint_least16_t; ///< Unsigned 16-bit integer
using uint32_t = std::uint_least32_t; ///< Unsigned 16-bit integer
using uint64_t = std::uint_least64_t; ///< Unsigned 16-bit integer
/// @}

/// @{ @name IO Types

/// @ingroup Types

using io_bool_t = bool; ///< bool
using io_int_t = int64_t; ///< int64_t
using io_real_t = real_t; ///< real_t
using io_cplx_t = cplx_t; ///< cplx_t
using io_str_t = str_t; ///< str_t

/// @}

} // namespace analysis

#endif // ICD_ANALYSIS_TYPE_ALIASES_HPP
