/*
File        : $HeadURL: https://swdev.cld.analog.com/svn/projects/icdev/sandbox/users/pderouni/platform-upgrades/src/analysis/array_math.hpp $
Originator  : pderouni
Revision    : $Revision: 12382 $
Last Commit : $Date: 2020-04-06 11:21:16 -0400 (Mon, 06 Apr 2020) $
Last Editor : $Author: pderouni $
*/

#ifndef ICD_ANALYSIS_UTILITIES_HPP
#define ICD_ANALYSIS_UTILITIES_HPP

#include "analysis.hpp"
#include "type_aliases.hpp"

#define ICD_API ICD_ANALYSIS_DECL

namespace analysis { /// @{ @ingroup ArrayMath

/**
     * @brief abs
     */
ICD_API void abs(const cplx_t* in_data, ///< [in]  complex input array pointer
    size_t in_size, ///< [in]  complex input array size
    real_t* out_data, ///< [out] magnitude output array pointer
    size_t out_size ///< [in]  magnitude output array size
);

/**
     * @brief angle
     */
ICD_API void angle(const cplx_t* in_data, ///< [in]  complex input array pointer
    size_t in_size, ///< [in]  complex input array size
    real_t* out_data, ///< [out] phase output array pointer
    size_t out_size ///< [in]  phase output array size
);

/**
     * @brief complex
     */
ICD_API void complex(const real_t* re_data, ///< [in]  real input array pointer
    size_t re_size, ///< [in]  real input array size
    const real_t* im_data, ///< [in]  imaginary input array pointer
    size_t im_size, ///< [in]  imaginary input array size
    cplx_t* out_data, ///< [out] complex output array pointer
    size_t out_size ///< [in]  complex output array size
);

/**
     * @brief db
     */
ICD_API void db(const cplx_t* in_data, ///< [in]  complex input array pointer
    size_t in_size, ///< [in]  complex input array size
    real_t* out_data, ///< [out] magnitude-dB output array pointer
    size_t out_size ///< [in]  magnitude-dB output array size
);

/**
     * @brief db10
     */
ICD_API void db10(const real_t* in_data, ///< [in]  magnitude-squared input array pointer
    size_t in_size, ///< [in]  magnitude-squared input array size
    real_t* out_data, ///< [out] magnitude-dB output array pointer
    size_t out_size ///< [in]  magnitude-dB output array size
);

/**
     * @brief db20
     */
ICD_API void db20(const real_t* in_data, ///< [in]  magnitude input array pointer
    size_t in_size, ///< [in]  magnitude input array size
    real_t* out_data, ///< [out] magnitude-dB output array pointer
    size_t out_size ///< [in]  magnitude-dB output array size
);

/**
     * @brief imag
     */
ICD_API void imag(const cplx_t* in_data, ///< [in]  complex input array pointer
    size_t in_size, ///< [in]  complex input array size
    real_t* out_data, ///< [out] imaginary output array pointer
    size_t out_size ///< [in]  imaginary output array size
);

/**
     * @brief norm
     */
ICD_API void norm(const cplx_t* in_data, ///< [in]  complex input array pointer
    size_t in_size, ///< [in]  complex input array size
    real_t* out_data, ///< [out] magnitude-squared output array pointer
    size_t out_size ///< [in]  magnitude-squared output array size
);

/**
     * @brief polar
     */
ICD_API void polar(const real_t* m_data, ///< [in]  magnitude input array pointer
    size_t m_size, ///< [in]  magnitude input array size
    const real_t* p_data, ///< [in]  phase input array pointer
    size_t p_size, ///< [in]  phase input array size
    cplx_t* out_data, ///< [out] complex output array pointer
    size_t out_size ///< [in]  complex output array size
);

/**
     * @brief real
     */
ICD_API void real(const cplx_t* in_data, ///< [in]  complex input array pointer
    size_t in_size, ///< [in]  complex input array size
    real_t* out_data, ///< [out] real output array pointer
    size_t out_size ///< [in]  real output array size
);

/// @} ArrayMath

} // namespace analysis

#undef ICD_API

#endif // ICD_ANALYSIS_UTILITIES_HPP
