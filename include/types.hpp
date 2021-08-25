/*
File        : $HeadURL: https://swdev.cld.analog.com/svn/projects/icdev/sandbox/users/pderouni/platform-upgrades/src/analysis/types.hpp $
Originator  : pderouni
Revision    : $Revision: 12382 $
Last Commit : $Date: 2020-04-06 11:21:16 -0400 (Mon, 06 Apr 2020) $
Last Editor : $Author: pderouni $
*/

#ifndef ICD_ANALYSIS_TYPES_HPP
#define ICD_ANALYSIS_TYPES_HPP

#include "type_aliases.hpp"
#include <vector>

namespace analysis {

/// @{ @name Aligned Numeric Container Aliases

/// @ingroup Types

using int16_vector = std::vector<int16_t>;
using int32_vector = std::vector<int32_t>;
using int64_vector = std::vector<int64_t>;
using real_vector = std::vector<real_t>; ///< Real-valued waveforms
using cplx_vector = std::vector<cplx_t>; ///< FFT data

/// @}

/// @{ @name Other Container Aliases

/// @ingroup Types

using str_vector = std::vector<str_t>;

/// @}

} // namespace analysis

#endif // ICD_ANALYSIS_TYPES_HPP
