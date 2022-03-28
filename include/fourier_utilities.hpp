#ifndef DCANALYSIS_IMPL_FOURIER_UTILITIES_HPP
#define DCANALYSIS_IMPL_FOURIER_UTILITIES_HPP

#include "enums.hpp"
#include "type_aliases.hpp"

namespace dcanalysis_impl {

    real_t alias(real_t fs, real_t freq, FreqAxisType axis_type);

    real_t coherent(size_t nfft, real_t fs, real_t freq);

    void fftshift(const real_t* in_data, size_t in_size, real_t* out_data, size_t out_size);

    void freq_axis(
        real_t* data,
        size_t size,
        size_t nfft,
        FreqAxisType axis_type,
        real_t fs,
        FreqAxisFormat axis_format
        );

    void ifftshift(const real_t* in_data, size_t in_size, real_t* out_data, size_t out_size);

    size_t freq_axis_size(size_t nfft, FreqAxisType axis_type);

} // namespace dcanalysis_impl

#endif // DCANALYSIS_IMPL_FOURIER_UTILITIES_HPP