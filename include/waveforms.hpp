/*
File        : $HeadURL: https://swdev.cld.analog.com/svn/projects/icdev/sandbox/users/pderouni/platform-upgrades/src/analysis/waveforms.hpp $
Originator  : pderouni
Revision    : $Revision: 12382 $
Last Commit : $Date: 2020-04-06 11:21:16 -0400 (Mon, 06 Apr 2020) $
Last Editor : $Author: pderouni $
*/

#ifndef ICD_ANALYSIS_WAVEFORMS_HPP
#define ICD_ANALYSIS_WAVEFORMS_HPP

#include "analysis.hpp"
#include "type_aliases.hpp"
#include <vector>

#define ICD_API ICD_ANALYSIS_DECL

namespace analysis { /// @{ @ingroup Waveforms

/**
     * @brief cos
     */
ICD_API void cos(real_t* data, ///< [out] array pointer
    size_t size, ///< [in]  array size
    real_t fs, ///< [in]  sample rate (S/s)
    real_t fsr, ///< [in]  full-scale range
    const std::vector<real_t>& ampl, ///< [in]  amplitude (dBFS)
    const std::vector<real_t>& freq, ///< [in]  frequency (Hz)
    const std::vector<real_t>& phase, ///< [in]  phase (rad)
    real_t td, ///< [in]  delay (s)
    real_t tj, ///< [in]  rms jitter (s)
    real_t offset ///< [in]  offset
);

/**
     * @brief noise
     */
ICD_API void noise(real_t* data, ///< [out] array pointer
    size_t size, ///< [in]  array size
    real_t fsr, ///< [in]  full-scale range
    real_t noise, ///< [in]  integrated noise (dBFS)
    real_t offset ///< [in]  offset
);

/**
     * @brief ramp
     */
ICD_API void ramp(real_t* data, ///< [out] array pointer
    size_t size, ///< [in]  array size
    real_t start, ///< [in]  start value
    real_t stop, ///< [in]  stop value
    real_t offset ///< [in]  offset
);

/**
     * @brief sin
     */
ICD_API void sin(real_t* data, ///< [out] array pointer
    size_t size, ///< [in]  array size
    real_t fs, ///< [in]  sample rate (S/s)
    real_t fsr, ///< [in]  full-scale range
    const std::vector<real_t>& ampl, ///< [in]  amplitude (dBFS)
    const std::vector<real_t>& freq, ///< [in]  frequency (Hz)
    const std::vector<real_t>& phase, ///< [in]  phase (rad)
    real_t td, ///< [in]  delay (s)
    real_t tj, ///< [in]  rms jitter (s)
    real_t offset ///< [in]  offset
);

/// @} Waveforms

} // namespace analysis

#undef ICD_API

#endif // ICD_ANALYSIS_WAVEFORMS_HPP
