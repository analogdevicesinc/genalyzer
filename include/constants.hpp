/*
File        : $HeadURL: https://swdev.cld.analog.com/svn/projects/icdev/sandbox/users/pderouni/platform-upgrades/src/analysis/constants.hpp $
Originator  : pderouni
Revision    : $Revision: 12511 $
Last Commit : $Date: 2020-05-20 09:32:34 -0400 (Wed, 20 May 2020) $
Last Editor : $Author: pderouni $
*/

#ifndef ICD_ANALYSIS_CONSTANTS_HPP
#define ICD_ANALYSIS_CONSTANTS_HPP

#include "enums.hpp"
#include "type_aliases.hpp"
#include <limits>

namespace analysis {

/// @{ @name Mathematical Constants
// In C++17, can use exact hex literal

/// @ingroup Constants

/// @cond
const real_t k_one = 1.0;
const real_t k_two = 2.0;
/// @endcond

const real_t k_sqrt2 = std::sqrt(k_two); ///< &radic;2
const real_t k_pi4 = std::atan(k_one); ///< &pi;/4
const real_t k_pi2 = k_two * k_pi4; ///< &pi;/2
const real_t k_pi = k_two * k_pi2; ///< &pi;
const real_t k_2pi = k_two * k_pi; ///< 2&pi;
/// @}

/// @{ @name Code Format

/// @ingroup Constants

const CodeFormat def_code_format = CodeFormat::TwosComplement;

/// @}

/// @{ @name Converter Resolution

/// @ingroup Constants

const int max_res = 30;
const int min_res = 1;
/// @}

/// @{ @name Full-Scale Range

/// @ingroup Constants

const real_t def_fsr = 2.0;
/// @}

/// @{ @name Number of Points

/// @ingroup Constants

const diff_t max_npts = 1073741824; ///< 2<sup>30</sup>
const diff_t min_npts = 1;
/// @}

/// @{ @name PMF Type

/// @ingroup Constants

const PmfType def_pmf_type = PmfType::Tone;

/// @}

/// @{ @name Sample Frequency

/// @ingroup Constants

const real_t def_fs = 1.0;
const real_t max_fs = 1e+15;
const real_t min_fs = 1e-15;
/// @}

/// @{ @name Sample Period

/// @ingroup Constants

const real_t def_ts = 1.0;
const real_t max_ts = 1e+15;
const real_t min_ts = 1e-15;
/// @}

/// @{ @name Other Numeric Constants

/// @ingroup Constants

/// @cond
const auto real_t_digits10 = std::numeric_limits<real_t>::digits10;
const auto real_t_max_digits10 = std::numeric_limits<real_t>::max_digits10;
static_assert(15 == real_t_digits10, "real_t_digits10 != 15");
/// @endcond

const real_t max_ms_value = 1e+30;
const real_t min_ms_value = 1e-30;
const real_t max_rms_value = 1e+15;
const real_t min_rms_value = 1e-15;
const real_t max_db_value = +300.0;
const real_t min_db_value = -300.0;

/// @}

} // namespace analysis

#endif // ICD_ANALYSIS_CONSTANTS_HPP
