#ifndef GENALYZER_IMPL_CONSTANTS_HPP
#define GENALYZER_IMPL_CONSTANTS_HPP

#include "type_aliases.hpp"

#include <cmath>

namespace genalyzer_impl {

    const real_t k_half         = 0.5;
    const real_t k_one          = 1.0;
    const real_t k_two          = 2.0;
    const real_t k_sqrt2        = std::sqrt(k_two);
    const real_t k_inv_sqrt2    = std::sqrt(k_half);
    const real_t k_pi4          = std::atan(k_one);
    const real_t k_pi2          = k_two * k_pi4;
    const real_t k_pi           = k_two * k_pi2;
    const real_t k_2pi          = k_two * k_pi;

    const int k_abs_min_code_width = 1;
    const int k_abs_max_code_width = 30;

    const size_t k_abs_max_fft_navg = 256;

    const real_t k_abs_max_db  =  400.0;
    const real_t k_abs_min_db  = -400.0;
    const real_t k_abs_max_rms = std::pow(10.0, k_abs_max_db / 20.0);
    const real_t k_abs_min_rms = std::pow(10.0, k_abs_min_db / 20.0);
    const real_t k_abs_max_msq = k_abs_max_rms * k_abs_max_rms;
    const real_t k_abs_min_msq = k_abs_min_rms * k_abs_min_rms;

} // namespace genalyzer_impl

#endif // GENALYZER_IMPL_CONSTANTS_HPP
