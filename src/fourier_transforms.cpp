#include "fourier_transforms.hpp"

#include "constants.hpp"
#include "exceptions.hpp"
#include "utils.hpp"

#include <fftw3.h>

#include <algorithm>
#include <cmath>
#include <numeric>
#include <vector>

namespace dcanalysis_impl {

    namespace { // Window Function Constants

        const real_t blackman_harris_kx =  1.9688861870585801;
        const real_t blackman_harris_k0 =  0.35875;
        const real_t blackman_harris_k1 = -0.48829;
        const real_t blackman_harris_k2 =  0.14128;
        const real_t blackman_harris_k3 = -0.01168;
        const real_t hann_kx =  1.6329922791756648;
        const real_t hann_k0 =  0.5;
        const real_t hann_k1 = -0.5;

    } // namespace anonymous

    namespace { // Window-Only Functions

        // For Complex FFT
        void blackman_harris(
            const real_t* i_data,
            const real_t* q_data,
            real_t* out_data,
            size_t in_stride,
            size_t navg,
            size_t nfft
            )
        {
            const real_t scalar = blackman_harris_kx;
            const real_t k1 = k_2pi / static_cast<real_t>(nfft);
            const real_t k2 = k1 * 2;
            const real_t k3 = k1 * 3;
            const size_t in_row_stride = nfft * in_stride;
            const size_t out_row_stride = nfft * 2;
            size_t i = 0;
            real_t x = 0.0;
            if (1 == navg) {
                for (size_t j = 0; j < out_row_stride; j += 2) {
                    const real_t w = blackman_harris_k0
                                   + blackman_harris_k1 * std::cos(k1 * x)
                                   + blackman_harris_k2 * std::cos(k2 * x)
                                   + blackman_harris_k3 * std::cos(k3 * x);
                    out_data[j]   = w * scalar * i_data[i];
                    out_data[j+1] = w * scalar * q_data[i];
                    i += in_stride;
                    x += 1.0;
                }
            } else if (2 == navg) {
                const real_t* i2_data = i_data + in_row_stride;
                const real_t* q2_data = q_data + in_row_stride;
                real_t* out2_data = out_data + out_row_stride;
                for (size_t j = 0; j < out_row_stride; j += 2) {
                    const real_t w = blackman_harris_k0
                                   + blackman_harris_k1 * std::cos(k1 * x)
                                   + blackman_harris_k2 * std::cos(k2 * x)
                                   + blackman_harris_k3 * std::cos(k3 * x);
                    out_data[j]    = w * scalar * i_data[i];
                    out_data[j+1]  = w * scalar * q_data[i];
                    out2_data[j]   = w * scalar * i2_data[i];
                    out2_data[j+1] = w * scalar * q2_data[i];
                    i += in_stride;
                    x += 1.0;
                }
            } else { // 2 < navg
                for (size_t j = 0; j < out_row_stride; j += 2) {
                    const real_t w = blackman_harris_k0
                                   + blackman_harris_k1 * std::cos(k1 * x)
                                   + blackman_harris_k2 * std::cos(k2 * x)
                                   + blackman_harris_k3 * std::cos(k3 * x);
                    const real_t* pi = i_data;
                    const real_t* pq = q_data;
                    real_t* po = out_data;
                    for (size_t k = 0; k < navg; ++k) {
                        po[j]   = w * scalar * pi[i];
                        po[j+1] = w * scalar * pq[i];
                        pi += in_row_stride;
                        pq += in_row_stride;
                        po += out_row_stride;
                    }
                    i += in_stride;
                    x += 1.0;
                }
            }
        }

        // For Real FFT
        void blackman_harris(
            const real_t* in_data,
            real_t* out_data,
            size_t navg,
            size_t nfft,
            RfftScale scale
            )
        {
            const real_t scalar = blackman_harris_kx * ((RfftScale::DbfsSin == scale) ? k_sqrt2 : 1.0);
            const real_t k1 = k_2pi / static_cast<real_t>(nfft);
            const real_t k2 = k1 * 2;
            const real_t k3 = k1 * 3;
            const size_t out_stride = (nfft / 2 + 1) * 2;
            real_t x = 0.0;
            if (1 == navg) {
                for (size_t i = 0; i < nfft; ++i) {
                    const real_t w = blackman_harris_k0
                                   + blackman_harris_k1 * std::cos(k1 * x)
                                   + blackman_harris_k2 * std::cos(k2 * x)
                                   + blackman_harris_k3 * std::cos(k3 * x);
                    out_data[i] = w * scalar * in_data[i];
                    x += 1.0;
                }
            } else if (2 == navg) {
                const real_t* in2_data = in_data + nfft;
                real_t* out2_data = out_data + out_stride;
                for (size_t i = 0; i < nfft; ++i) {
                    const real_t w = blackman_harris_k0
                                   + blackman_harris_k1 * std::cos(k1 * x)
                                   + blackman_harris_k2 * std::cos(k2 * x)
                                   + blackman_harris_k3 * std::cos(k3 * x);
                    out_data[i]  = w * scalar * in_data[i];
                    out2_data[i] = w * scalar * in2_data[i];
                    x += 1.0;
                }
            } else { // 2 < navg
                for (size_t i = 0; i < nfft; ++i) {
                    const real_t w = blackman_harris_k0
                                   + blackman_harris_k1 * std::cos(k1 * x)
                                   + blackman_harris_k2 * std::cos(k2 * x)
                                   + blackman_harris_k3 * std::cos(k3 * x);
                    const real_t* pi = in_data;
                    real_t* po = out_data;
                    for (size_t j = 0; j < navg; ++j) {
                        po[i] = w * scalar * pi[i];
                        pi += nfft;
                        po += out_stride;
                    }
                    x += 1.0;
                }
            }
        }

        // For Complex FFT
        void hann(
            const real_t* i_data,
            const real_t* q_data,
            real_t* out_data,
            size_t in_stride,
            size_t navg,
            size_t nfft
            )
        {
            const real_t scalar = hann_kx;
            const real_t k1 = k_2pi / static_cast<real_t>(nfft);
            const size_t in_row_stride = nfft * in_stride;
            const size_t out_row_stride = nfft * 2;
            size_t i = 0;
            real_t x = 0.0;
            if (1 == navg) {
                for (size_t j = 0; j < out_row_stride; j += 2) {
                    const real_t w = hann_k0 + hann_k1 * std::cos(k1 * x);
                    out_data[j]   = w * scalar * i_data[i];
                    out_data[j+1] = w * scalar * q_data[i];
                    i += in_stride;
                    x += 1.0;
                }
            } else if (2 == navg) {
                const real_t* i2_data = i_data + nfft * in_stride;
                const real_t* q2_data = q_data + nfft * in_stride;
                real_t* out2_data = out_data + out_row_stride;
                for (size_t j = 0; j < out_row_stride; j += 2) {
                    const real_t w = hann_k0 + hann_k1 * std::cos(k1 * x);
                    out_data[j]    = w * scalar * i_data[i];
                    out_data[j+1]  = w * scalar * q_data[i];
                    out2_data[j]   = w * scalar * i2_data[i];
                    out2_data[j+1] = w * scalar * q2_data[i];
                    i += in_stride;
                    x += 1.0;
                }
            } else { // 2 < navg
                for (size_t j = 0; j < out_row_stride; j += 2) {
                    const real_t w = hann_k0 + hann_k1 * std::cos(k1 * x);
                    const real_t* pi = i_data;
                    const real_t* pq = q_data;
                    real_t* po = out_data;
                    for (size_t k = 0; k < navg; ++k) {
                        po[j]   = w * scalar * pi[i];
                        po[j+1] = w * scalar * pq[i];
                        pi += in_row_stride;
                        pq += in_row_stride;
                        po += out_row_stride;
                    }
                    i += in_stride;
                    x += 1.0;
                }
            }
        }

        // For Real FFT
        void hann(
            const real_t* in_data,
            real_t* out_data,
            size_t navg,
            size_t nfft,
            RfftScale scale
            )
        {
            const real_t scalar = hann_kx * ((RfftScale::DbfsSin == scale) ? k_sqrt2 : 1.0);
            const real_t k1 = k_2pi / static_cast<real_t>(nfft);
            const size_t out_stride = (nfft / 2 + 1) * 2;
            real_t x = 0.0;
            if (1 == navg) {
                for (size_t i = 0; i < nfft; ++i) {
                    const real_t w = hann_k0 + hann_k1 * std::cos(k1 * x);
                    out_data[i] = w * scalar * in_data[i];
                    x += 1.0;
                }
            } else if (2 == navg) {
                const real_t* in2_data = in_data + nfft;
                real_t* out2_data = out_data + out_stride;
                for (size_t i = 0; i < nfft; ++i) {
                    const real_t w = hann_k0 + hann_k1 * std::cos(k1 * x);
                    out_data[i]  = w * scalar * in_data[i];
                    out2_data[i] = w * scalar * in2_data[i];
                    x += 1.0;
                }
            } else { // 2 < navg
                for (size_t i = 0; i < nfft; ++i) {
                    const real_t w = hann_k0 + hann_k1 * std::cos(k1 * x);
                    const real_t* pi = in_data;
                    real_t* po = out_data;
                    for (size_t j = 0; j < navg; ++j) {
                        po[i] = w * scalar * pi[i];
                        pi += nfft;
                        po += out_stride;
                    }
                    x += 1.0;
                }
            }
        }

        // For Complex FFT
        void no_window(
            const real_t* i_data,
            const real_t* q_data,
            real_t* out_data,
            size_t in_stride,
            size_t navg,
            size_t nfft
            )
        {
            const size_t out_size = navg * nfft * 2;
            size_t i = 0;
            for (size_t j = 0; j < out_size; j += 2) {
                out_data[j]   = i_data[i];
                out_data[j+1] = q_data[i];
                i += in_stride;
            }
        }

        // For Real FFT
        void no_window(
            const real_t* in_data,
            real_t* out_data,
            size_t navg,
            size_t nfft,
            RfftScale scale
            )
        {
            const real_t scalar = (RfftScale::DbfsSin == scale) ? k_sqrt2 : 1.0;
            const size_t out_stride = (nfft / 2 + 1) * 2;
            for (size_t j = 0; j < navg; ++j) {
                for (size_t i = 0; i < nfft; ++i) {
                    out_data[i] = scalar * in_data[i];
                }
                in_data += nfft;
                out_data += out_stride;
            }
        }

    } // namespace anonymous

    namespace { // Normalize-Window Functions

        // For Complex FFT
        template<typename T>
        void norm_blackman_harris(
            const T* i_data,
            const T* q_data,
            real_t* out_data,
            size_t in_stride,
            int n,
            size_t navg,
            size_t nfft,
            CodeFormat format
            )
        {
            const real_t scalar = blackman_harris_kx * 2.0 / (1 << n);
            const real_t offset = (CodeFormat::OffsetBinary == format) ? -blackman_harris_kx : 0.0;
            const real_t k1 = k_2pi / static_cast<real_t>(nfft);
            const real_t k2 = k1 * 2;
            const real_t k3 = k1 * 3;
            const size_t in_row_stride = nfft * in_stride;
            const size_t out_row_stride = nfft * 2;
            size_t i = 0;
            real_t x = 0.0;
            if (1 == navg) {
                for (size_t j = 0; j < out_row_stride; j += 2) {
                    const real_t w = blackman_harris_k0
                                   + blackman_harris_k1 * std::cos(k1 * x)
                                   + blackman_harris_k2 * std::cos(k2 * x)
                                   + blackman_harris_k3 * std::cos(k3 * x);
                    out_data[j]   = w * std::fma(scalar, static_cast<real_t>(i_data[i]), offset);
                    out_data[j+1] = w * std::fma(scalar, static_cast<real_t>(q_data[i]), offset);
                    i += in_stride;
                    x += 1.0;
                }
            } else if (2 == navg) {
                const T* i2_data = i_data + in_row_stride;
                const T* q2_data = q_data + in_row_stride;
                real_t* out2_data = out_data + out_row_stride;
                for (size_t j = 0; j < out_row_stride; j += 2) {
                    const real_t w = blackman_harris_k0
                                   + blackman_harris_k1 * std::cos(k1 * x)
                                   + blackman_harris_k2 * std::cos(k2 * x)
                                   + blackman_harris_k3 * std::cos(k3 * x);
                    out_data[j]    = w * std::fma(scalar, static_cast<real_t>(i_data[i]),  offset);
                    out_data[j+1]  = w * std::fma(scalar, static_cast<real_t>(q_data[i]),  offset);
                    out2_data[j]   = w * std::fma(scalar, static_cast<real_t>(i2_data[i]), offset);
                    out2_data[j+1] = w * std::fma(scalar, static_cast<real_t>(q2_data[i]), offset);
                    i += in_stride;
                    x += 1.0;
                }
            } else { // 2 < navg
                for (size_t j = 0; j < out_row_stride; j += 2) {
                    const real_t w = blackman_harris_k0
                                   + blackman_harris_k1 * std::cos(k1 * x)
                                   + blackman_harris_k2 * std::cos(k2 * x)
                                   + blackman_harris_k3 * std::cos(k3 * x);
                    const T* pi = i_data;
                    const T* pq = q_data;
                    real_t* po = out_data;
                    for (size_t k = 0; k < navg; ++k) {
                        po[j]   = w * std::fma(scalar, static_cast<real_t>(pi[i]), offset);
                        po[j+1] = w * std::fma(scalar, static_cast<real_t>(pq[i]), offset);
                        pi += in_row_stride;
                        pq += in_row_stride;
                        po += out_row_stride;
                    }
                    i += in_stride;
                    x += 1.0;
                }
            }
        }

        // For Real FFT
        template<typename T>
        void norm_blackman_harris(
            const T* in_data,
            real_t* out_data,
            int n,
            size_t navg,
            size_t nfft,
            CodeFormat format,
            RfftScale scale
            )
        {
            real_t s = blackman_harris_kx * ((RfftScale::DbfsSin == scale) ? k_sqrt2 : 1.0);
            const real_t scalar = s * 2.0 / (1 << n);
            const real_t offset = (CodeFormat::OffsetBinary == format) ? -s : 0.0;
            const real_t k1 = k_2pi / static_cast<real_t>(nfft);
            const real_t k2 = k1 * 2;
            const real_t k3 = k1 * 3;
            const size_t out_stride = (nfft / 2 + 1) * 2;
            real_t x = 0.0;
            if (1 == navg) {
                for (size_t i = 0; i < nfft; ++i) {
                    const real_t w = blackman_harris_k0
                                   + blackman_harris_k1 * std::cos(k1 * x)
                                   + blackman_harris_k2 * std::cos(k2 * x)
                                   + blackman_harris_k3 * std::cos(k3 * x);
                    out_data[i] = w * std::fma(scalar, static_cast<real_t>(in_data[i]), offset);
                    x += 1.0;
                }
            } else if (2 == navg) {
                const T* in2_data = in_data + nfft;
                real_t* out2_data = out_data + out_stride;
                for (size_t i = 0; i < nfft; ++i) {
                    const real_t w = blackman_harris_k0
                                   + blackman_harris_k1 * std::cos(k1 * x)
                                   + blackman_harris_k2 * std::cos(k2 * x)
                                   + blackman_harris_k3 * std::cos(k3 * x);
                    out_data[i]  = w * std::fma(scalar, static_cast<real_t>(in_data[i]),  offset);
                    out2_data[i] = w * std::fma(scalar, static_cast<real_t>(in2_data[i]), offset);
                    x += 1.0;
                }
            } else { // 2 < navg
                for (size_t i = 0; i < nfft; ++i) {
                    const real_t w = blackman_harris_k0
                                   + blackman_harris_k1 * std::cos(k1 * x)
                                   + blackman_harris_k2 * std::cos(k2 * x)
                                   + blackman_harris_k3 * std::cos(k3 * x);
                    const T* pi = in_data;
                    real_t* po = out_data;
                    for (size_t j = 0; j < navg; ++j) {
                        po[i] = w * std::fma(scalar, static_cast<real_t>(pi[i]), offset);
                        pi += nfft;
                        po += out_stride;
                    }
                    x += 1.0;
                }
            }
        }

        // For Complex FFT
        template<typename T>
        void norm_hann(
            const T* i_data,
            const T* q_data,
            real_t* out_data,
            size_t in_stride,
            int n,
            size_t navg,
            size_t nfft,
            CodeFormat format
            )
        {
            const real_t scalar = hann_kx * 2.0 / (1 << n);
            const real_t offset = (CodeFormat::OffsetBinary == format) ? -hann_kx : 0.0;
            const real_t k1 = k_2pi / static_cast<real_t>(nfft);
            const size_t in_row_stride = nfft * in_stride;
            const size_t out_row_stride = nfft * 2;
            size_t i = 0;
            real_t x = 0.0;
            if (1 == navg) {
                for (size_t j = 0; j < out_row_stride; j += 2) {
                    const real_t w = hann_k0 + hann_k1 * std::cos(k1 * x);
                    out_data[j]   = w * std::fma(scalar, static_cast<real_t>(i_data[i]), offset);
                    out_data[j+1] = w * std::fma(scalar, static_cast<real_t>(q_data[i]), offset);
                    i += in_stride;
                    x += 1.0;
                }
            } else if (2 == navg) {
                const T* i2_data = i_data + nfft * in_stride;
                const T* q2_data = q_data + nfft * in_stride;
                real_t* out2_data = out_data + out_row_stride;
                for (size_t j = 0; j < out_row_stride; j += 2) {
                    const real_t w = hann_k0 + hann_k1 * std::cos(k1 * x);
                    out_data[j]    = w * std::fma(scalar, static_cast<real_t>(i_data[i]),  offset);
                    out_data[j+1]  = w * std::fma(scalar, static_cast<real_t>(q_data[i]),  offset);
                    out2_data[j]   = w * std::fma(scalar, static_cast<real_t>(i2_data[i]), offset);
                    out2_data[j+1] = w * std::fma(scalar, static_cast<real_t>(q2_data[i]), offset);
                    i += in_stride;
                    x += 1.0;
                }
            } else { // 2 < navg
                for (size_t j = 0; j < out_row_stride; j += 2) {
                    const real_t w = hann_k0 + hann_k1 * std::cos(k1 * x);
                    const T* pi = i_data;
                    const T* pq = q_data;
                    real_t* po = out_data;
                    for (size_t k = 0; k < navg; ++k) {
                        po[j]   = w * std::fma(scalar, static_cast<real_t>(pi[i]), offset);
                        po[j+1] = w * std::fma(scalar, static_cast<real_t>(pq[i]), offset);
                        pi += in_row_stride;
                        pq += in_row_stride;
                        po += out_row_stride;
                    }
                    i += in_stride;
                    x += 1.0;
                }
            }
        }

        // For Real FFT
        template<typename T>
        void norm_hann(
            const T* in_data,
            real_t* out_data,
            int n,
            size_t navg,
            size_t nfft,
            CodeFormat format,
            RfftScale scale
            )
        {
            real_t s = hann_kx * ((RfftScale::DbfsSin == scale) ? k_sqrt2 : 1.0);
            const real_t scalar = s * 2.0 / (1 << n);
            const real_t offset = (CodeFormat::OffsetBinary == format) ? -s : 0.0;
            const real_t k1 = k_2pi / static_cast<real_t>(nfft);
            const size_t out_stride = (nfft / 2 + 1) * 2;
            real_t x = 0.0;
            if (1 == navg) {
                for (size_t i = 0; i < nfft; ++i) {
                    const real_t w = hann_k0 + hann_k1 * std::cos(k1 * x);
                    out_data[i] = w * std::fma(scalar, static_cast<real_t>(in_data[i]), offset);
                    x += 1.0;
                }
            } else if (2 == navg) {
                const T* in2_data = in_data + nfft;
                real_t* out2_data = out_data + out_stride;
                for (size_t i = 0; i < nfft; ++i) {
                    const real_t w = hann_k0 + hann_k1 * std::cos(k1 * x);
                    out_data[i]  = w * std::fma(scalar, static_cast<real_t>(in_data[i]),  offset);
                    out2_data[i] = w * std::fma(scalar, static_cast<real_t>(in2_data[i]), offset);
                    x += 1.0;
                }
            } else { // 2 < navg
                for (size_t i = 0; i < nfft; ++i) {
                    const real_t w = hann_k0 + hann_k1 * std::cos(k1 * x);
                    const T* pi = in_data;
                    real_t* po = out_data;
                    for (size_t j = 0; j < navg; ++j) {
                        po[i] = w * std::fma(scalar, static_cast<real_t>(pi[i]), offset);
                        pi += nfft;
                        po += out_stride;
                    }
                    x += 1.0;
                }
            }
        }

        // For Complex FFT
        template<typename T>
        void norm_no_window(
            const T* i_data,
            const T* q_data,
            real_t* out_data,
            size_t in_stride,
            int n,
            size_t navg,
            size_t nfft,
            CodeFormat format
            )
        {
            const real_t scalar = 2.0 / (1 << n);
            const real_t offset = (CodeFormat::OffsetBinary == format) ? -1.0 : 0.0;
            const size_t out_size = navg * nfft * 2;
            size_t i = 0;
            for (size_t j = 0; j < out_size; j += 2) {
                out_data[j]   = std::fma(scalar, static_cast<real_t>(i_data[i]), offset);
                out_data[j+1] = std::fma(scalar, static_cast<real_t>(q_data[i]), offset);
                i += in_stride;
            }
        }

        // For Real FFT
        template<typename T>
        void norm_no_window(
            const T* in_data,
            real_t* out_data,
            int n,
            size_t navg,
            size_t nfft,
            CodeFormat format,
            RfftScale scale
            )
        {
            real_t s = (RfftScale::DbfsSin == scale) ? k_sqrt2 : 1.0;
            const real_t scalar = s * 2.0 / (1 << n);
            const real_t offset = (CodeFormat::OffsetBinary == format) ? -s : 0.0;
            const size_t out_stride = (nfft / 2 + 1) * 2;
            for (size_t j = 0; j < navg; ++j) {
                for (size_t i = 0; i < nfft; ++i) {
                    out_data[i] = std::fma(scalar, static_cast<real_t>(in_data[i]), offset);
                }
                in_data += nfft;
                out_data += out_stride;
            }
        }

    } // namespace anonymous

    namespace { // FFTW Functions

        void exec_fftw(real_t* data, size_t navg, size_t nfft)
        {
            diff_t navg_ = static_cast<diff_t>(navg);
            diff_t nfft_ = static_cast<diff_t>(nfft);
            // FFT size: FFTW "rank" and "dims"
            int rank = 1;
            fftw_iodim64 dims {nfft_, 2, 2};
            // FFT averaging: FFTW "howmany_rank" and "howmany_dims"
            int howmany_rank = 1;
            fftw_iodim64 howmany_dims = {navg_, nfft_ * 2, nfft_ * 2};
            // FFTW plan setup and execution
            fftw_plan plan = fftw_plan_guru64_split_dft(
                rank,
                &dims,
                howmany_rank,
                &howmany_dims,
                data,           // I input
                data + 1,       // Q input
                data,           // Re output
                data + 1,       // Im output
                FFTW_ESTIMATE);
            if (nullptr == plan) {
                throw runtime_error("FFTW Plan is NULL");
            }
            fftw_execute(plan);
            fftw_destroy_plan(plan);
        }

        void exec_rfftw(real_t* data, size_t navg, size_t nfft)
        {
            diff_t navg_ = static_cast<diff_t>(navg);
            diff_t nfft_ = static_cast<diff_t>(nfft);
            diff_t out_stride = nfft_ / 2 + 1;
            // FFT size: FFTW "rank" and "dims"
            int rank = 1;
            fftw_iodim64 dims {nfft_, 1, 1};
            // FFT averaging: FFTW "howmany_rank" and "howmany_dims"
            int howmany_rank = 1;
            fftw_iodim64 howmany_dims {navg_, out_stride * 2, out_stride};
            // Plan setup and execution
            fftw_complex* out_data = reinterpret_cast<fftw_complex*>(data);
            fftw_plan plan = fftw_plan_guru64_dft_r2c(
                rank,
                &dims,
                howmany_rank,
                &howmany_dims,
                data,
                out_data,
                FFTW_ESTIMATE);
            if (nullptr == plan) {
                throw runtime_error("FFTW Plan is NULL");
            }
            fftw_execute(plan);
            fftw_destroy_plan(plan);
        }

    } // namespace anonymous

    namespace { // Scaling and Averaging Functions

        void reduce_and_scale_fft(real_t* fftw_data, real_t* out_data, size_t navg, size_t nfft)
        {
            cplx_t* cfftw_data = reinterpret_cast<cplx_t*>(fftw_data);
            cplx_t* cout_data = reinterpret_cast<cplx_t*>(out_data);
            for (size_t i = 0; i < nfft; ++i) {
                cout_data[i] = {std::norm(cfftw_data[i]), std::arg(cfftw_data[i])};
            }
            for (size_t j = 1; j < navg; ++j) {
                cfftw_data += nfft;
                for (size_t i = 0; i < nfft; ++i) {
                    cout_data[i] += cplx_t(std::norm(cfftw_data[i]), std::arg(cfftw_data[i]));
                }
            }
            const real_t avg_scalar = 1.0 / static_cast<real_t>(navg);
            const real_t fft_scalar = 1.0 / static_cast<real_t>(nfft);
            for (size_t i = 0; i < nfft; ++i) {
                cplx_t& x = cout_data[i];
                x *= avg_scalar;
                x = std::polar(std::sqrt(x.real()) * fft_scalar, x.imag());
            }
        }

        void reduce_and_scale_rfft(
            real_t* fftw_data, real_t* out_data, size_t navg, size_t nfft, RfftScale scale)
        {
            cplx_t* cfftw_data = reinterpret_cast<cplx_t*>(fftw_data);
            cplx_t* cout_data = reinterpret_cast<cplx_t*>(out_data);
            const size_t cout_size = nfft / 2 + 1;
            for (size_t i = 0; i < cout_size; ++i) {
                cout_data[i] = {std::norm(cfftw_data[i]), std::arg(cfftw_data[i])};
            }
            for (size_t j = 1; j < navg; ++j) {
                cfftw_data += cout_size;
                for (size_t i = 0; i < cout_size; ++i) {
                    cout_data[i] += cplx_t(std::norm(cfftw_data[i]), std::arg(cfftw_data[i]));
                }
            }
            const real_t avg_scalar = 1.0 / static_cast<real_t>(navg);
            real_t s = (RfftScale::Native == scale) ? 1.0 : k_sqrt2;
            const real_t fft_scalar = s / static_cast<real_t>(nfft);
            for (size_t i = 0; i < cout_size; ++i) {
                cplx_t& x = cout_data[i];
                x *= avg_scalar;
                x = std::polar(std::sqrt(x.real()) * fft_scalar, x.imag());
            }
            if (RfftScale::Native != scale) {
                cout_data[0] /= k_sqrt2;
                if (1 < nfft && is_even(nfft)) {
                    cout_data[cout_size - 1] /= k_sqrt2;
                }
            }
        }

        void scale_fft(real_t* data, size_t nfft)
        {
            const size_t size = 2 * nfft;
            const real_t scalar = 1.0 / static_cast<real_t>(nfft);
            for (size_t i = 0; i < size; ++i) {
                data[i] *= scalar;
            }
        }

        void scale_rfft(real_t* data, size_t nfft, RfftScale scale)
        {
            const size_t size = (nfft / 2 + 1) * 2;
            real_t s = (RfftScale::Native == scale) ? 1.0 : k_sqrt2;
            const real_t scalar = s / static_cast<real_t>(nfft);
            for (size_t i = 0; i < size; ++i) {
                data[i] *= scalar;
            }
            if (RfftScale::Native != scale) {
                data[0] /= k_sqrt2;
                data[1] /= k_sqrt2;
                if (1 < nfft && is_even(nfft)) {
                    data[size - 1] /= k_sqrt2;
                    data[size - 2] /= k_sqrt2;
                }
            }
        }

    } // namespace anonymous

    void fft(
        const real_t* i_data,
        size_t i_size,
        const real_t* q_data,
        size_t q_size,
        real_t* out_data,
        size_t out_size,
        size_t navg,
        size_t nfft,
        Window window
        )
    {
        size_t in_stride = 1;
        if (0 == q_size) {
            // Interleaved I/Q
            check_array("", "input array", i_data, i_size, true);
            i_size /= 2;
            q_size = i_size;
            q_data = i_data + 1;
            in_stride = 2;
        } else {
            // Split I/Q
            check_array_pair("", "I array", i_data, i_size, "Q array", q_data, q_size);
        }
        check_array("", "output array", out_data, out_size);
        size_t out_size_expected = fft_size(i_size, q_size, navg, nfft); // may modify navg and nfft
        assert_eq("", "output array size", out_size, "expected", out_size_expected);
        // If not averaging (1 == navg), use out_data directly.  Otherwise, allocate temporary
        // array to store result of normalization/windowing and FFT.  For example,
        //   navg = 4, nfft = 16
        //   => i_size = 64
        //      q_size = 64
        //      out_size = 16 * 2 = 32
        //      tmp.size = navg * out_size = 128
        std::vector<real_t> tmp ((1 == navg) ? 0 : 2 * i_size);
        real_t* fftw_data = (1 == navg) ? out_data : tmp.data();
        switch (window)
        {
            case Window::BlackmanHarris:
                blackman_harris(i_data, q_data, fftw_data, in_stride, navg, nfft);
                break;
            case Window::Hann:
                hann(i_data, q_data, fftw_data, in_stride, navg, nfft);
                break;
            case Window::NoWindow:
                no_window(i_data, q_data, fftw_data, in_stride, navg, nfft);
                break;
            default:
                throw runtime_error("unsupported window");
        }
        exec_fftw(fftw_data, navg, nfft);
        if (1 == navg) {
            scale_fft(out_data, nfft);
        } else {
            reduce_and_scale_fft(fftw_data, out_data, navg, nfft);
        }
    }

    template<typename T>
    void fft(
        const T* i_data,
        size_t i_size,
        const T* q_data,
        size_t q_size,
        real_t* out_data,
        size_t out_size,
        int n,
        size_t navg,
        size_t nfft,
        Window window,
        CodeFormat format
        )
    {
        size_t in_stride = 1;
        if (0 == q_size) {
            // Interleaved I/Q
            check_array("", "input array", i_data, i_size, true);
            i_size /= 2;
            q_size = i_size;
            q_data = i_data + 1;
            in_stride = 2;
        } else {
            // Split I/Q
            check_array_pair("", "I array", i_data, i_size, "Q array", q_data, q_size);
        }
        check_array("", "output array", out_data, out_size);
        size_t out_size_expected = fft_size(i_size, q_size, navg, nfft); // may modify navg and nfft
        assert_eq("", "output array size", out_size, "expected", out_size_expected);
        check_code_width("", n);
        // If not averaging (1 == navg), use out_data directly.  Otherwise, allocate temporary
        // array to store result of normalization/windowing and FFT.  For example,
        //   navg = 4, nfft = 16
        //   => i_size = 64
        //      q_size = 64
        //      out_size = 16 * 2 = 32
        //      tmp.size = navg * out_size = 128
        std::vector<real_t> tmp ((1 == navg) ? 0 : 2 * i_size);
        real_t* fftw_data = (1 == navg) ? out_data : tmp.data();
        switch (window)
        {
            case Window::BlackmanHarris:
                norm_blackman_harris(i_data, q_data, fftw_data, in_stride, n, navg, nfft, format);
                break;
            case Window::Hann:
                norm_hann(i_data, q_data, fftw_data, in_stride, n, navg, nfft, format);
                break;
            case Window::NoWindow:
                norm_no_window(i_data, q_data, fftw_data, in_stride, n, navg, nfft, format);
                break;
            default:
                throw runtime_error("unsupported window");
        }
        exec_fftw(fftw_data, navg, nfft);
        if (1 == navg) {
            scale_fft(out_data, nfft);
        } else {
            reduce_and_scale_fft(fftw_data, out_data, navg, nfft);
        }
    }

    template void fft(const int16_t*, size_t, const int16_t*, size_t, real_t*, size_t, int, size_t, size_t, Window, CodeFormat);
    template void fft(const int32_t*, size_t, const int32_t*, size_t, real_t*, size_t, int, size_t, size_t, Window, CodeFormat);
    template void fft(const int64_t*, size_t, const int64_t*, size_t, real_t*, size_t, int, size_t, size_t, Window, CodeFormat);

    void rfft(
        const real_t* in_data,
        size_t in_size,
        real_t* out_data,
        size_t out_size,
        size_t navg,
        size_t nfft,
        Window window,
        RfftScale scale
        )
    {
        check_array("", "input array", in_data, in_size);
        check_array("", "output array", out_data, out_size);
        size_t out_size_expected = rfft_size(in_size, navg, nfft);
        assert_eq("", "output array size", out_size, "expected", out_size_expected);
        // If not averaging (1 == navg), use out_data directly.  Otherwise, allocate temporary
        // array to store result of windowing and FFT.  For example,
        //   navg = 4, nfft = 16
        //   => in_size = 64
        //      out_size = (16/2 + 1) * 2 = 18
        //      tmp.size = navg * out_size = 72
        std::vector<real_t> tmp ((1 == navg) ? 0 : navg * out_size);
        real_t* fftw_data = (1 == navg) ? out_data : tmp.data();
        switch (window)
        {
            case Window::BlackmanHarris:
                blackman_harris(in_data, fftw_data, navg, nfft, scale);
                break;
            case Window::Hann:
                hann(in_data, fftw_data, navg, nfft, scale);
                break;
            case Window::NoWindow:
                no_window(in_data, fftw_data, navg, nfft, scale);
                break;
            default:
                throw runtime_error("unsupported window");
        }
        exec_rfftw(fftw_data, navg, nfft);
        if (1 == navg) {
            scale_rfft(out_data, nfft, scale);
        } else {
            reduce_and_scale_rfft(fftw_data, out_data, navg, nfft, scale);
        }
    }

    template<typename T>
    void rfft(
        const T* in_data,
        size_t in_size,
        real_t* out_data,
        size_t out_size,
        int n,
        size_t navg,
        size_t nfft,
        Window window,
        CodeFormat format,
        RfftScale scale
        )
    {
        check_array("", "input array", in_data, in_size);
        check_array("", "output array", out_data, out_size);
        size_t out_size_expected = rfft_size(in_size, navg, nfft);
        assert_eq("", "output array size", out_size, "expected", out_size_expected);
        check_code_width("", n);
        // If not averaging (1 == navg), use out_data directly.  Otherwise, allocate temporary
        // array to store result of normalization/windowing and FFT.  For example,
        //   navg = 4, nfft = 16
        //   => in_size = 64
        //      out_size = (16/2 + 1) * 2 = 18
        //      tmp.size = navg * out_size = 72
        std::vector<real_t> tmp ((1 == navg) ? 0 : navg * out_size);
        real_t* fftw_data = (1 == navg) ? out_data : tmp.data();
        switch (window)
        {
            case Window::BlackmanHarris:
                norm_blackman_harris(in_data, fftw_data, n, navg, nfft, format, scale);
                break;
            case Window::Hann:
                norm_hann(in_data, fftw_data, n, navg, nfft, format, scale);
                break;
            case Window::NoWindow:
                norm_no_window(in_data, fftw_data, n, navg, nfft, format, scale);
                break;
            default:
                throw runtime_error("unsupported window");
        }
        exec_rfftw(fftw_data, navg, nfft);
        if (1 == navg) {
            scale_rfft(out_data, nfft, scale);
        } else {
            reduce_and_scale_rfft(fftw_data, out_data, navg, nfft, scale);
        }
    }

    template void rfft(const int16_t*, size_t, real_t*, size_t, int, size_t, size_t, Window, CodeFormat, RfftScale);
    template void rfft(const int32_t*, size_t, real_t*, size_t, int, size_t, size_t, Window, CodeFormat, RfftScale);
    template void rfft(const int64_t*, size_t, real_t*, size_t, int, size_t, size_t, Window, CodeFormat, RfftScale);

} // namespace dcanalysis_impl

namespace dcanalysis_impl {

    namespace {

        //  fft: in_size is the size of either the I or Q waveform (not the sum)
        // rfft: in_size is the size of the waveform
        void resolve_navg_and_nfft(const size_t in_size, size_t& navg, size_t& nfft)
        {
            assert_gt0("", "input size", in_size);
            if (k_abs_max_fft_navg < navg) {
                throw runtime_error("navg (" + std::to_string(navg)
                    + ") exceeds limit (" + std::to_string(k_abs_max_fft_navg) + ")");
            }
            if (0 == navg && 0 == nfft) {
                navg = 1;
                nfft = in_size;
            } else if (0 == navg) {
                navg = in_size / nfft;
                if (0 == navg) {
                    throw runtime_error("derived navg == 0");
                }
                if (k_abs_max_fft_navg < navg) {
                    throw runtime_error("derived navg (" + std::to_string(navg)
                        + ") exceeds limit (" + std::to_string(k_abs_max_fft_navg) + ")");
                }
            } else if (0 == nfft) {
                nfft = in_size / navg;
            }
            size_t in_size_expected = navg * nfft;
            try {
                assert_eq("", "input size", in_size, "expected", in_size_expected);
            } catch (const std::exception& e) {
                throw runtime_error(str_t(e.what()) + "\nExpected input size = navg * nfft = "
                    + std::to_string(navg) + " * " + std::to_string(nfft) + " = "
                    + std::to_string(in_size_expected) + "\nGot input size = "
                    + std::to_string(in_size));
            }
        }
        
    } // namespace anonymous

    size_t fft_size(size_t i_size, size_t q_size, size_t& navg, size_t& nfft)
    {
        if (0 == q_size) {
            if (is_odd(i_size)) {
                throw runtime_error("size of interleaved array must be even");
            }
            i_size /= 2;
        } else {
            assert_eq("", "I size", i_size, "Q size", q_size);
        }
        resolve_navg_and_nfft(i_size, navg, nfft);
        size_t size = nfft * 2;
        return size;
    }

    size_t rfft_size(size_t in_size, size_t& navg, size_t& nfft)
    {
        resolve_navg_and_nfft(in_size, navg, nfft);
        size_t size = (nfft / 2 + 1) * 2; // integer division intentional
        return size;
    }

} // namespace dcanalysis_impl