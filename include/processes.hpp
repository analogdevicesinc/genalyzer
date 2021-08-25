/*
File        : $HeadURL: https://swdev.cld.analog.com/svn/projects/icdev/sandbox/users/pderouni/platform-upgrades/src/analysis/processes.hpp $
Originator  : pderouni
Revision    : $Revision: 12382 $
Last Commit : $Date: 2020-04-06 11:21:16 -0400 (Mon, 06 Apr 2020) $
Last Editor : $Author: pderouni $
*/

#ifndef ICD_ANALYSIS_PROCESSES_HPP
#define ICD_ANALYSIS_PROCESSES_HPP

#include "analysis.hpp"
#include "enums.hpp"
#include "type_aliases.hpp"
#include "types.hpp"

#define ICD_API ICD_ANALYSIS_DECL

namespace analysis { /// @{ @ingroup Processes

/**
     * @brief normalize
     * @tparam T int16_t, int32_t, int64_t
     */
template <typename T>
void normalize(const T* in_data, ///< [in]  input array pointer
    size_t in_size, ///< [in]  input array size
    real_t* out_data, ///< [out] output array pointer
    size_t out_size, ///< [in]  output array size
    int res, ///< [in]  full-scale resolution
    CodeFormat format ///< [in]  code format
);

/**
     * @brief polyval
     */
ICD_API void polyval(const real_t* in_data, ///< [in]  input array pointer
    size_t in_size, ///< [in]  input array size
    real_t* out_data, ///< [out] output array pointer
    size_t out_size, ///< [in]  output array size
    std::vector<real_t> poco ///< [in]  polynomial coefficients
);

/**
     * @brief quantize
     * @tparam T int16_t, int32_t, int64_t
     */
template <typename T>
void quantize(const real_t* in_data, ///< [in]  input array pointer
    size_t in_size, ///< [in]  input array size
    T* out_data, ///< [out] output array pointer
    size_t out_size, ///< [in]  output array size
    real_t fsr, ///< [in]  full-scale range
    int res, ///< [in]  resolution
    real_t noise, ///< [in]  integrated noise (dBFS)
    CodeFormat format, ///< [in]  code format
    bool null_offset, ///< [in]  null offset
    int m, ///< [in]  number of interleaved channels
    const std::vector<real_t>& offset, ///< [in]  interleaved channel offsets
    const std::vector<real_t>& gerror ///< [in]  interleaved channel gain errors
);

/**
     * @brief window
     */
ICD_API void window(const real_t* in_data, ///< [in]  input array pointer
    size_t in_size, ///< [in]  input array size
    real_t* out_data, ///< [out] output array pointer
    size_t out_size, ///< [in]  output array size
    WindowType window ///< [in]  window type
);

/// @} Processes

} // namespace analysis

namespace analysis { // Allocating Functions

using real_vector_pair = std::pair<real_vector, real_vector>;

ICD_API real_vector downsample(const real_vector& data,
    int m,
    diff_t start,
    size_t size);

ICD_API real_vector_pair downsample(const real_vector_pair& data,
    int m,
    diff_t start,
    size_t size);

ICD_API void ftrans(real_vector& data, real_t fs, real_t freq);

ICD_API void ftrans(real_vector_pair& data, real_t fs, real_t freq);

ICD_API real_vector pshift(const real_vector& data, real_t phase);

ICD_API real_vector_pair pshift(const real_vector_pair& data,
    real_t phase);

} // namespace analysis - Allocating Functions

#undef ICD_API

#endif // ICD_ANALYSIS_PROCESSES_HPP
