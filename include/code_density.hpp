/*
File        : $HeadURL: https://swdev.cld.analog.com/svn/projects/icdev/sandbox/users/pderouni/platform-upgrades/src/analysis/code_density.hpp $
Originator  : pderouni
Revision    : $Revision: 12382 $
Last Commit : $Date: 2020-04-06 11:21:16 -0400 (Mon, 06 Apr 2020) $
Last Editor : $Author: pderouni $
*/

#ifndef ICD_ANALYSIS_CODE_DENSITY_HPP
#define ICD_ANALYSIS_CODE_DENSITY_HPP

#include "analysis.hpp"
#include "enums.hpp"
#include "type_aliases.hpp"

#define ICD_API ICD_ANALYSIS_DECL

namespace analysis { /// @{ @ingroup CodeDensity

/**
     * @brief dnl Computes DNL from histogram data
     */
ICD_API void dnl(const int64_t* hits_data, ///< [in]  code hits array pointer
    size_t hits_size, ///< [in]  code hits array size
    real_t* dnl_data, ///< [out] DNL array pointer
    size_t dnl_size, ///< [in]  DNL array size
    PmfType pmf ///< [in]  probability mass function
);

/**
     * @brief histogram2 Computes histogram over the given range of codes
     * @tparam T int16_t, int32_t, int64_t
     */
template <typename T>
void histogram(const T* in_data, ///< [in]  input array pointer
    size_t in_size, ///< [in]  input array size
    int32_t* bins_data, ///< [out] code bins array pointer
    size_t bins_size, ///< [in]  code bins array size
    int64_t* hits_data, ///< [out] code hits array pointer
    size_t hits_size, ///< [in]  code hits array size
    int64_t min_code, ///< [in]  minimum code
    int64_t max_code ///< [in]  maximum code
);

/**
     * @brief inl, Computes INL from DNL data
     */
ICD_API void inl(const real_t* dnl_data, ///< [in]  DNL array pointer
    size_t dnl_size, ///< [in]  DNL array size
    real_t* inl_data, ///< [out] INL array pointer
    size_t inl_size, ///< [in]  INL array size
    bool fit ///< [in]  enable line fit
);

/// @} CodeDensity

} // namespace analysis

#undef ICD_API

#endif // ICD_ANALYSIS_CODE_DENSITY_HPP
