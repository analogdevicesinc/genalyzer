/*
File        : $HeadURL: https://swdev.cld.analog.com/svn/projects/icdev/sandbox/users/pderouni/platform-upgrades/src/analysis/dft.cpp $
Originator  : pderouni
Revision    : $Revision: 12540 $
Last Commit : $Date: 2020-05-22 09:17:35 -0400 (Fri, 22 May 2020) $
Last Editor : $Author: pderouni $
*/

#include "dft.hpp"
#include "checks.hpp"
#include "constants.hpp"
#include "enum_maps.hpp"
#include <fftw3.h>
#include <numeric>

/*
 * - Check for alignment and copy if not aligned (?)
 * - Need to understand FFTW's potential destruction of input data while
 *   creating plan (FFTW's docs say ESTIMATE will not touch input data)
 * - Revisit wisdom; it would be nice to take advantage of this
 * - FFTW build
 * - Benchmarks
 *     - in-place vs out-of-place
 *     - aligned vs not aligned (SIMD vs no SIMD)
 *     - threads, OpenMP
 */

namespace analysis {

namespace {

    bool is_even(real_t n)
    {
        return 0.0 == std::fmod(n, 2.0);
    }

} // namespace anonymous

} // namespace analysis

namespace analysis { // Window Functions for Complex FFT

void bh4(const real_t* in_i,
    const real_t* in_q,
    const diff_t in_col_stride,
    real_t* out_i,
    real_t* out_q,
    const diff_t out_col_stride,
    int avg,
    diff_t size)
{
    const real_t k0 = 1.9688861870585801;
    const real_t k1 = k_2pi / size;
    const real_t k2 = k1 * 2;
    const real_t k3 = k1 * 3;
    const diff_t in_row_stride = size * in_col_stride;
    const diff_t out_row_stride = size * out_col_stride;
    diff_t j = 0;
    diff_t k = 0;
    if (1 == avg) {
        for (diff_t i = 0; i < size; ++i) {
            real_t w_factor = k0 * (0.35875 - 0.48829 * std::cos(k1 * i) + 0.14128 * std::cos(k2 * i) - 0.01168 * std::cos(k3 * i));
            out_i[k] = in_i[j] * w_factor;
            out_q[k] = in_q[j] * w_factor;
            j += in_col_stride;
            k += out_col_stride;
        }
    } else if (2 == avg) {
        const real_t* in2_i = in_i + in_row_stride;
        const real_t* in2_q = in_q + in_row_stride;
        real_t* out2_i = out_i + out_row_stride;
        real_t* out2_q = out_q + out_row_stride;
        for (diff_t i = 0; i < size; ++i) {
            real_t w_factor = k0 * (0.35875 - 0.48829 * std::cos(k1 * i) + 0.14128 * std::cos(k2 * i) - 0.01168 * std::cos(k3 * i));
            out_i[k] = in_i[j] * w_factor;
            out_q[k] = in_q[j] * w_factor;
            out2_i[k] = in2_i[j] * w_factor;
            out2_q[k] = in2_q[j] * w_factor;
            j += in_col_stride;
            k += out_col_stride;
        }
    } else { // 2 < avg
        for (diff_t i = 0; i < size; ++i) {
            real_t w_factor = k0 * (0.35875 - 0.48829 * std::cos(k1 * i) + 0.14128 * std::cos(k2 * i) - 0.01168 * std::cos(k3 * i));
            diff_t jj = j;
            diff_t kk = k;
            for (diff_t z = 0; z < avg; ++z) {
                out_i[kk] = in_i[jj] * w_factor;
                out_q[kk] = in_q[jj] * w_factor;
                jj += in_row_stride;
                kk += out_row_stride;
            }
            j += in_col_stride;
            k += out_col_stride;
        }
    }
}

void hann(const real_t* in_i,
    const real_t* in_q,
    const diff_t in_col_stride,
    real_t* out_i,
    real_t* out_q,
    const diff_t out_col_stride,
    int avg,
    diff_t size)
{
    const real_t k0 = 1.6329922791756648;
    const real_t k1 = k_2pi / size;
    const diff_t in_row_stride = size * in_col_stride;
    const diff_t out_row_stride = size * out_col_stride;
    diff_t j = 0;
    diff_t k = 0;
    if (1 == avg) {
        for (diff_t i = 0; i < size; ++i) {
            real_t w_factor = k0 * (0.5 - 0.5 * std::cos(k1 * i));
            out_i[k] = in_i[j] * w_factor;
            out_q[k] = in_q[j] * w_factor;
            j += in_col_stride;
            k += out_col_stride;
        }
    } else if (2 == avg) {
        const real_t* in2_i = in_i + in_row_stride;
        const real_t* in2_q = in_q + in_row_stride;
        real_t* out2_i = out_i + out_row_stride;
        real_t* out2_q = out_q + out_row_stride;
        for (diff_t i = 0; i < size; ++i) {
            real_t w_factor = k0 * (0.5 - 0.5 * std::cos(k1 * i));
            out_i[k] = in_i[j] * w_factor;
            out_q[k] = in_q[j] * w_factor;
            out2_i[k] = in2_i[j] * w_factor;
            out2_q[k] = in2_q[j] * w_factor;
            j += in_col_stride;
            k += out_col_stride;
        }
    } else { // 2 < avg
        for (diff_t i = 0; i < size; ++i) {
            real_t w_factor = k0 * (0.5 - 0.5 * std::cos(k1 * i));
            diff_t jj = j;
            diff_t kk = k;
            for (diff_t z = 0; z < avg; ++z) {
                out_i[kk] = in_i[jj] * w_factor;
                out_q[kk] = in_q[jj] * w_factor;
                jj += in_row_stride;
                kk += out_row_stride;
            }
            j += in_col_stride;
            k += out_col_stride;
        }
    }
}

void rect(const real_t* in_i,
    const real_t* in_q,
    const diff_t in_col_stride,
    real_t* out_i,
    real_t* out_q,
    const diff_t out_col_stride,
    int avg,
    diff_t size)
{
    const diff_t data_size = avg * size;
    diff_t j = 0;
    diff_t k = 0;
    for (diff_t i = 0; i < data_size; ++i) {
        out_i[k] = in_i[j];
        out_q[k] = in_q[j];
        j += in_col_stride;
        k += out_col_stride;
    }
}

} // namespace analysis - Window Functions for Complex FFT

namespace analysis { // Normalize-Window Functions for Complex FFT

template <typename T>
void norm_bh4(const T* in_i,
    const T* in_q,
    const diff_t in_col_stride,
    real_t* out_i,
    real_t* out_q,
    const diff_t out_col_stride,
    int resolution,
    int avg,
    diff_t size,
    CodeFormat format)
{
    const real_t k0 = 1.9688861870585801;
    const real_t k1 = k_2pi / size;
    const real_t k2 = k1 * 2;
    const real_t k3 = k1 * 3;
    const real_t n_factor = 2.0 / (1 << resolution);
    const real_t f_offset = (CodeFormat::OffsetBinary == format) ? -1.0 : 0.0;
    const diff_t in_row_stride = size * in_col_stride;
    const diff_t out_row_stride = size * out_col_stride;
    diff_t j = 0;
    diff_t k = 0;
    if (1 == avg) {
        for (diff_t i = 0; i < size; ++i) {
            real_t w_factor = k0 * (0.35875 - 0.48829 * std::cos(k1 * i) + 0.14128 * std::cos(k2 * i) - 0.01168 * std::cos(k3 * i));
            out_i[k] = std::fma(in_i[j], n_factor, f_offset) * w_factor;
            out_q[k] = std::fma(in_q[j], n_factor, f_offset) * w_factor;
            j += in_col_stride;
            k += out_col_stride;
        }
    } else if (2 == avg) {
        const T* in2_i = in_i + in_row_stride;
        const T* in2_q = in_q + in_row_stride;
        real_t* out2_i = out_i + out_row_stride;
        real_t* out2_q = out_q + out_row_stride;
        for (diff_t i = 0; i < size; ++i) {
            real_t w_factor = k0 * (0.35875 - 0.48829 * std::cos(k1 * i) + 0.14128 * std::cos(k2 * i) - 0.01168 * std::cos(k3 * i));
            out_i[k] = std::fma(in_i[j], n_factor, f_offset) * w_factor;
            out_q[k] = std::fma(in_q[j], n_factor, f_offset) * w_factor;
            out2_i[k] = std::fma(in2_i[j], n_factor, f_offset) * w_factor;
            out2_q[k] = std::fma(in2_q[j], n_factor, f_offset) * w_factor;
            j += in_col_stride;
            k += out_col_stride;
        }
    } else { // 2 < avg
        for (diff_t i = 0; i < size; ++i) {
            real_t w_factor = k0 * (0.35875 - 0.48829 * std::cos(k1 * i) + 0.14128 * std::cos(k2 * i) - 0.01168 * std::cos(k3 * i));
            diff_t jj = j;
            diff_t kk = k;
            for (diff_t z = 0; z < avg; ++z) {
                out_i[kk] = std::fma(in_i[jj], n_factor, f_offset) * w_factor;
                out_q[kk] = std::fma(in_q[jj], n_factor, f_offset) * w_factor;
                jj += in_row_stride;
                kk += out_row_stride;
            }
            j += in_col_stride;
            k += out_col_stride;
        }
    }
}

template <typename T>
void norm_hann(const T* in_i,
    const T* in_q,
    const diff_t in_col_stride,
    real_t* out_i,
    real_t* out_q,
    const diff_t out_col_stride,
    int resolution,
    int avg,
    diff_t size,
    CodeFormat format)
{
    const real_t k0 = 1.6329922791756648;
    const real_t k1 = k_2pi / size;
    const real_t n_factor = 2.0 / (1 << resolution);
    const real_t f_offset = (CodeFormat::OffsetBinary == format) ? -1.0 : 0.0;
    const diff_t in_row_stride = size * in_col_stride;
    const diff_t out_row_stride = size * out_col_stride;
    diff_t j = 0;
    diff_t k = 0;
    if (1 == avg) {
        for (diff_t i = 0; i < size; ++i) {
            real_t w_factor = k0 * (0.5 - 0.5 * std::cos(k1 * i));
            out_i[k] = std::fma(in_i[j], n_factor, f_offset) * w_factor;
            out_q[k] = std::fma(in_q[j], n_factor, f_offset) * w_factor;
            j += in_col_stride;
            k += out_col_stride;
        }
    } else if (2 == avg) {
        const T* in2_i = in_i + in_row_stride;
        const T* in2_q = in_q + in_row_stride;
        real_t* out2_i = out_i + out_row_stride;
        real_t* out2_q = out_q + out_row_stride;
        for (diff_t i = 0; i < size; ++i) {
            real_t w_factor = k0 * (0.5 - 0.5 * std::cos(k1 * i));
            out_i[k] = std::fma(in_i[j], n_factor, f_offset) * w_factor;
            out_q[k] = std::fma(in_q[j], n_factor, f_offset) * w_factor;
            out2_i[k] = std::fma(in2_i[j], n_factor, f_offset) * w_factor;
            out2_q[k] = std::fma(in2_q[j], n_factor, f_offset) * w_factor;
            j += in_col_stride;
            k += out_col_stride;
        }
    } else { // 2 < avg
        for (diff_t i = 0; i < size; ++i) {
            real_t w_factor = k0 * (0.5 - 0.5 * std::cos(k1 * i));
            diff_t jj = j;
            diff_t kk = k;
            for (diff_t z = 0; z < avg; ++z) {
                out_i[kk] = std::fma(in_i[jj], n_factor, f_offset) * w_factor;
                out_q[kk] = std::fma(in_q[jj], n_factor, f_offset) * w_factor;
                jj += in_row_stride;
                kk += out_row_stride;
            }
            j += in_col_stride;
            k += out_col_stride;
        }
    }
}

template <typename T>
void norm_rect(const T* in_i,
    const T* in_q,
    const diff_t in_col_stride,
    real_t* out_i,
    real_t* out_q,
    const diff_t out_col_stride,
    int resolution,
    int avg,
    diff_t size,
    CodeFormat format)
{
    const real_t n_factor = 2.0 / (1 << resolution);
    const real_t f_offset = (CodeFormat::OffsetBinary == format) ? -1.0 : 0.0;
    const diff_t data_size = avg * size;
    diff_t j = 0;
    diff_t k = 0;
    for (diff_t i = 0; i < data_size; ++i) {
        out_i[k] = std::fma(in_i[j], n_factor, f_offset);
        out_q[k] = std::fma(in_q[j], n_factor, f_offset);
        j += in_col_stride;
        k += out_col_stride;
    }
}

} // namespace analysis - Normalize-Window Functions for Complex FFT

namespace analysis { // Window Functions for Real FFT

void bh4(const real_t* in, real_t* out, int avg, diff_t size)
{
    const real_t k0 = 1.9688861870585801;
    const real_t k1 = k_2pi / size;
    const real_t k2 = k1 * 2;
    const real_t k3 = k1 * 3;
    const diff_t out_row_stride = (size / 2 + 1) * 2;
    if (1 == avg) {
        for (diff_t i = 0; i < size; ++i) {
            real_t w_factor = k0 * (0.35875 - 0.48829 * std::cos(k1 * i) + 0.14128 * std::cos(k2 * i) - 0.01168 * std::cos(k3 * i));
            out[i] = in[i] * w_factor;
        }
    } else if (2 == avg) {
        const real_t* in2 = in + size;
        real_t* out2 = out + out_row_stride;
        for (diff_t i = 0; i < size; ++i) {
            real_t w_factor = k0 * (0.35875 - 0.48829 * std::cos(k1 * i) + 0.14128 * std::cos(k2 * i) - 0.01168 * std::cos(k3 * i));
            out[i] = in[i] * w_factor;
            out2[i] = in2[i] * w_factor;
        }
    } else { // 2 < avg
        for (diff_t i = 0; i < size; ++i) {
            real_t w_factor = k0 * (0.35875 - 0.48829 * std::cos(k1 * i) + 0.14128 * std::cos(k2 * i) - 0.01168 * std::cos(k3 * i));
            const real_t* inx = in;
            real_t* outx = out;
            for (diff_t z = 0; z < avg; ++z) {
                outx[i] = inx[i] * w_factor;
                inx += size;
                outx += out_row_stride;
            }
        }
    }
}

void hann(const real_t* in, real_t* out, int avg, diff_t size)
{
    const real_t k0 = 1.6329922791756648;
    const real_t k1 = k_2pi / size;
    const diff_t out_row_stride = (size / 2 + 1) * 2;
    if (1 == avg) {
        for (diff_t i = 0; i < size; ++i) {
            real_t w_factor = k0 * (0.5 - 0.5 * std::cos(k1 * i));
            out[i] = in[i] * w_factor;
        }
    } else if (2 == avg) {
        const real_t* in2 = in + size;
        real_t* out2 = out + out_row_stride;
        for (diff_t i = 0; i < size; ++i) {
            real_t w_factor = k0 * (0.5 - 0.5 * std::cos(k1 * i));
            out[i] = in[i] * w_factor;
            out2[i] = in2[i] * w_factor;
        }
    } else { // 2 < avg
        for (diff_t i = 0; i < size; ++i) {
            real_t w_factor = k0 * (0.5 - 0.5 * std::cos(k1 * i));
            const real_t* inx = in;
            real_t* outx = out;
            for (diff_t z = 0; z < avg; ++z) {
                outx[i] = inx[i] * w_factor;
                inx += size;
                outx += out_row_stride;
            }
        }
    }
}

void rect(const real_t* in, real_t* out, int avg, diff_t size)
{
    const diff_t out_row_stride = (size / 2 + 1) * 2;
    const real_t* inx = in;
    real_t* outx = out;
    for (diff_t z = 0; z < avg; ++z) {
        for (diff_t i = 0; i < size; ++i) {
            outx[i] = inx[i];
        }
        inx += size;
        outx += out_row_stride;
    }
}

} // namespace analysis - Window Functions for Real FFT

namespace analysis { // Normalize-Window Functions for Real FFT

template <typename T>
void norm_bh4(const T* in,
    real_t* out,
    int resolution,
    int avg,
    diff_t size,
    CodeFormat format)
{
    const real_t k0 = 1.9688861870585801;
    const real_t k1 = k_2pi / size;
    const real_t k2 = k1 * 2;
    const real_t k3 = k1 * 3;
    const real_t n_factor = 2.0 / (1 << resolution);
    const real_t f_offset = (CodeFormat::OffsetBinary == format) ? -1.0 : 0.0;
    const diff_t out_row_stride = (size / 2 + 1) * 2;
    if (1 == avg) {
        for (diff_t i = 0; i < size; ++i) {
            real_t w_factor = k0 * (0.35875 - 0.48829 * std::cos(k1 * i) + 0.14128 * std::cos(k2 * i) - 0.01168 * std::cos(k3 * i));
            out[i] = std::fma(in[i], n_factor, f_offset) * w_factor;
        }
    } else if (2 == avg) {
        const T* in2 = in + size;
        real_t* out2 = out + out_row_stride;
        for (diff_t i = 0; i < size; ++i) {
            real_t w_factor = k0 * (0.35875 - 0.48829 * std::cos(k1 * i) + 0.14128 * std::cos(k2 * i) - 0.01168 * std::cos(k3 * i));
            out[i] = std::fma(in[i], n_factor, f_offset) * w_factor;
            out2[i] = std::fma(in2[i], n_factor, f_offset) * w_factor;
        }
    } else { // 2 < avg
        for (diff_t i = 0; i < size; ++i) {
            real_t w_factor = k0 * (0.35875 - 0.48829 * std::cos(k1 * i) + 0.14128 * std::cos(k2 * i) - 0.01168 * std::cos(k3 * i));
            const T* inx = in;
            real_t* outx = out;
            for (diff_t z = 0; z < avg; ++z) {
                outx[i] = std::fma(inx[i], n_factor, f_offset) * w_factor;
                inx += size;
                outx += out_row_stride;
            }
        }
    }
}

template <typename T>
void norm_hann(const T* in,
    real_t* out,
    int resolution,
    int avg,
    diff_t size,
    CodeFormat format)
{
    const real_t k0 = 1.6329922791756648;
    const real_t k1 = k_2pi / size;
    const real_t n_factor = 2.0 / (1 << resolution);
    const real_t f_offset = (CodeFormat::OffsetBinary == format) ? -1.0 : 0.0;
    const diff_t out_row_stride = (size / 2 + 1) * 2;
    if (1 == avg) {
        for (diff_t i = 0; i < size; ++i) {
            real_t w_factor = k0 * (0.5 - 0.5 * std::cos(k1 * i));
            out[i] = std::fma(in[i], n_factor, f_offset) * w_factor;
        }
    } else if (2 == avg) {
        const T* in2 = in + size;
        real_t* out2 = out + out_row_stride;
        for (diff_t i = 0; i < size; ++i) {
            real_t w_factor = k0 * (0.5 - 0.5 * std::cos(k1 * i));
            out[i] = std::fma(in[i], n_factor, f_offset) * w_factor;
            out2[i] = std::fma(in2[i], n_factor, f_offset) * w_factor;
        }
    } else { // 2 < avg
        for (diff_t i = 0; i < size; ++i) {
            real_t w_factor = k0 * (0.5 - 0.5 * std::cos(k1 * i));
            const T* inx = in;
            real_t* outx = out;
            for (diff_t z = 0; z < avg; ++z) {
                outx[i] = std::fma(inx[i], n_factor, f_offset) * w_factor;
                inx += size;
                outx += out_row_stride;
            }
        }
    }
}

template <typename T>
void norm_rect(const T* in,
    real_t* out,
    int resolution,
    int avg,
    diff_t size,
    CodeFormat format)
{
    const real_t n_factor = 2.0 / (1 << resolution);
    const real_t f_offset = (CodeFormat::OffsetBinary == format) ? -1.0 : 0.0;
    const diff_t out_row_stride = (size / 2 + 1) * 2;
    const T* inx = in;
    real_t* outx = out;
    for (diff_t z = 0; z < avg; ++z) {
        for (diff_t i = 0; i < size; ++i) {
            outx[i] = std::fma(inx[i], n_factor, f_offset);
        }
        inx += size;
        outx += out_row_stride;
    }
}

} // namespace analysis - Normalize-Window Functions for Real FFT

namespace analysis { // FFTW Functions

// Complex FFT: interleaved data
void exec_fft_il(cplx_t* data, // I/Q -> Re/Im
    diff_t avg,
    diff_t size)
{
    // FFT size: rank, dims
    int rank = 1;
    fftw_iodim64 dim{ size, 1, 1 };
    // FFT averaging: howmany_rank, howmany_dims
    int howmany_rank = 1;
    fftw_iodim64 howmany_dim{ avg, size, size };
    // Plan setup and execution
    fftw_complex* fftw_data = reinterpret_cast<fftw_complex*>(data);
    fftw_plan plan = fftw_plan_guru64_dft(rank,
        &dim,
        howmany_rank,
        &howmany_dim,
        fftw_data,
        fftw_data,
        FFTW_FORWARD,
        FFTW_ESTIMATE);
    if (nullptr == plan) {
        throw base::exception("FFTW Plan is NULL");
    }
    fftw_execute(plan);
    fftw_destroy_plan(plan);
}

// Complex FFT: split data
void exec_fft_sp(real_t* data_ir, // I -> Re
    real_t* data_qi, // Q -> Im
    diff_t avg,
    diff_t size)
{
    // FFT size: rank, dims
    int rank = 1;
    fftw_iodim64 dim{ size, 1, 1 };
    // FFT averaging: howmany_rank, howmany_dims
    int howmany_rank = 1;
    fftw_iodim64 howmany_dim{ avg, size, size };
    // Plan setup and execution
    fftw_plan plan = fftw_plan_guru64_split_dft(rank,
        &dim,
        howmany_rank,
        &howmany_dim,
        data_ir,
        data_qi,
        data_ir,
        data_qi,
        FFTW_ESTIMATE);
    if (nullptr == plan) {
        throw base::exception("FFTW Plan is NULL");
    }
    fftw_execute(plan);
    fftw_destroy_plan(plan);
}

// Real FFT
void exec_rfft(cplx_t* data, // Real -> Re/Im
    diff_t avg,
    diff_t fft_size)
{
    diff_t data_size = fft_size / 2 + 1;
    // FFT size: rank, dims
    int rank = 1;
    fftw_iodim64 dim{ fft_size, 1, 1 };
    // FFT averaging: howmany_rank, howmany_dims
    int howmany_rank = 1;
    fftw_iodim64 howmany_dim{ avg, data_size * 2, data_size };
    // Plan setup and execution
    real_t* fftw_in = reinterpret_cast<real_t*>(data);
    fftw_complex* fftw_out = reinterpret_cast<fftw_complex*>(data);
    fftw_plan plan = fftw_plan_guru64_dft_r2c(rank,
        &dim,
        howmany_rank,
        &howmany_dim,
        fftw_in,
        fftw_out,
        FFTW_ESTIMATE);
    if (nullptr == plan) {
        throw base::exception("FFTW Plan is NULL");
    }
    fftw_execute(plan);
    fftw_destroy_plan(plan);
}

} // namespace analysis - FFTW Functions

namespace analysis { // Post FFTW Functions

void cplx_to_polar(cplx_t* data, diff_t size)
{
    for (diff_t i = 0; i < size; ++i) {
        data[i] = { std::norm(data[i]), std::arg(data[i]) };
    }
}

void c2p_and_scale_fft(real_t* data_m,
    real_t* data_p,
    int avg,
    diff_t size)
{
    const diff_t data_size = avg * size;
    const cplx_t fft_size(static_cast<real_t>(size));
    for (diff_t i = 0; i < data_size; ++i) {
        cplx_t tmp(data_m[i], data_p[i]);
        data_m[i] = std::norm(tmp / fft_size);
        data_p[i] = std::arg(tmp);
    }
}

template <typename T>
void colwise_sum(T* in, T* out, int avg, diff_t size)
{
    std::copy(in, in + size, out);
    if (1 == avg) {
        return;
    }
    for (int row = 1; row < avg; ++row) {
        T* prow = in + size * row;
        for (diff_t i = 0; i < size; ++i) {
            out[i] += prow[i];
        }
    }
}

void average(cplx_t* in, cplx_t* out, int avg, diff_t size)
{
    colwise_sum(in, out, avg, size);
    const cplx_t x(1.0 / static_cast<real_t>(avg), 0.0);
    for (diff_t i = 0; i < size; ++i) {
        out[i] *= x;
    }
}

void average(real_t* in, real_t* out, int avg, diff_t size)
{
    colwise_sum(in, out, avg, size);
    const real_t x = 1.0 / static_cast<real_t>(avg);
    for (diff_t i = 0; i < size; ++i) {
        out[i] *= x;
    }
}

void p2c_and_scale_fft(cplx_t* data, diff_t size)
{
    const real_t rsize = static_cast<real_t>(size);
    for (diff_t i = 0; i < size; ++i) {
        cplx_t& d = data[i];
        d = std::polar(std::sqrt(d.real()) / rsize, d.imag());
    }
}

void p2c_and_scale_rfft(cplx_t* data, diff_t fft_size)
{
    const diff_t data_size = fft_size / 2 + 1;
    const cplx_t csqrt2(static_cast<real_t>(k_sqrt2));
    const real_t scalar = 2.0 / static_cast<real_t>(fft_size);
    for (diff_t i = 0; i < data_size; ++i) {
        cplx_t& d = data[i];
        d = std::polar(std::sqrt(d.real()) * scalar, d.imag());
    }
    data[0] /= csqrt2;
    if (1 < fft_size && 0 == fft_size % 2) {
        data[data_size - 1] /= csqrt2;
    }
}

void scale_fft(cplx_t* data, diff_t size)
{
    const cplx_t csize(static_cast<real_t>(size));
    for (diff_t i = 0; i < size; ++i) {
        data[i] /= csize;
    }
}

void scale_rfft(cplx_t* data, diff_t fft_size)
{
    const diff_t data_size = fft_size / 2 + 1;
    const cplx_t csqrt2(static_cast<real_t>(k_sqrt2));
    const cplx_t scalar(2.0 / static_cast<real_t>(fft_size));
    for (diff_t i = 0; i < data_size; ++i) {
        data[i] *= scalar;
    }
    data[0] /= csqrt2;
    if (1 < fft_size && 0 == fft_size % 2) {
        data[data_size - 1] /= csqrt2;
    }
}

} // namespace analysis - Post FFTW Functions

namespace analysis { // Discrete Fourier Transform

// Quantized IQ -> Complex
template <typename T>
void fft(const T* in_i,
    size_t in_i_size,
    const T* in_q,
    size_t in_q_size,
    diff_t in_stride,
    cplx_t* out,
    size_t out_size,
    int res,
    int avg,
    diff_t fft_size,
    WindowType window,
    CodeFormat format)
{
    check_array_pair(in_i, in_i_size, "in-phase input array",
        in_q, in_q_size, "quadrature input array");
    check_array(out, out_size, "output array");
    size_t out_size_exp = resolve_fft_size(in_i_size, avg, fft_size, false);
    assert_eq(out_size, "output array size", out_size_exp, "FFTSize");
    check_resolution<T>(res);
    bool allocate = (1 < avg);
    cplx_vector tmp(allocate ? in_i_size : 0);
    cplx_t* exec_io = allocate ? tmp.data() : out;
    real_t* nw_i = reinterpret_cast<real_t*>(exec_io);
    real_t* nw_q = nw_i + 1;
    switch (window) {
    case WindowType::BlackmanHarris:
        norm_bh4(in_i, in_q, in_stride, nw_i, nw_q, 2,
            res, avg, fft_size, format);
        break;
    case WindowType::Hann:
        norm_hann(in_i, in_q, in_stride, nw_i, nw_q, 2,
            res, avg, fft_size, format);
        break;
    case WindowType::Rect:
        norm_rect(in_i, in_q, in_stride, nw_i, nw_q, 2,
            res, avg, fft_size, format);
        break;
    default:
        throw base::exception("Unsupported window type: "
            + window_type_map[window]);
    }
    exec_fft_il(exec_io, avg, fft_size);
    if (1 == avg) {
        scale_fft(out, fft_size);
    } else {
        cplx_to_polar(exec_io, avg * fft_size);
        average(exec_io, out, avg, fft_size);
        p2c_and_scale_fft(out, fft_size);
    }
}

// Quantized IQ -> M/P
template <typename T>
void fft_mp(const T* in_i,
    size_t in_i_size,
    const T* in_q,
    size_t in_q_size,
    diff_t in_stride,
    real_t* out_m,
    size_t out_m_size,
    real_t* out_p,
    size_t out_p_size,
    int res,
    int avg,
    diff_t fft_size,
    WindowType window,
    CodeFormat format)
{
    check_array_pair(in_i, in_i_size, "in-phase input array",
        in_q, in_q_size, "quadrature input array");
    check_array_pair(out_m, out_m_size, "magnitude output array",
        out_p, out_p_size, "phase output array");
    size_t out_size_exp = resolve_fft_size(in_i_size, avg, fft_size, false);
    assert_eq(out_m_size, "magnitude output array size",
        out_size_exp, "FFTSize");
    check_resolution<T>(res);
    bool allocate = (1 < avg);
    real_vector tmp_im(allocate ? in_i_size : 0); // im = in-phase / magnitude
    real_vector tmp_qp(allocate ? in_i_size : 0); // qp = quadrature / phase
    real_t* exec_im = allocate ? tmp_im.data() : out_m;
    real_t* exec_qp = allocate ? tmp_qp.data() : out_p;
    switch (window) {
    case WindowType::BlackmanHarris:
        norm_bh4(in_i, in_q, in_stride, exec_im, exec_qp, 1,
            res, avg, fft_size, format);
        break;
    case WindowType::Hann:
        norm_hann(in_i, in_q, in_stride, exec_im, exec_qp, 1,
            res, avg, fft_size, format);
        break;
    case WindowType::Rect:
        norm_rect(in_i, in_q, in_stride, exec_im, exec_qp, 1,
            res, avg, fft_size, format);
        break;
    default:
        throw base::exception("Unsupported window type: "
            + window_type_map[window]);
    }
    exec_fft_sp(exec_im, exec_qp, avg, fft_size);
    c2p_and_scale_fft(exec_im, exec_qp, avg, fft_size);
    if (1 < avg) {
        average(exec_im, out_m, avg, fft_size);
        average(exec_qp, out_p, avg, fft_size);
    }
}

// Normalized IQ -> Complex
void fft_norm(const cplx_t* in,
    size_t in_size,
    cplx_t* out,
    size_t out_size,
    int avg,
    diff_t fft_size,
    WindowType window)
{
    check_array(in, in_size, "input array");
    check_array(out, out_size, "output array");
    size_t out_size_exp = resolve_fft_size(in_size, avg, fft_size, false);
    assert_eq(out_size, "output array size", out_size_exp, "FFTSize");
    bool allocate = (1 < avg);
    cplx_vector tmp(allocate ? in_size : 0);
    cplx_t* exec_io = allocate ? tmp.data() : out;
    const real_t* in_i = reinterpret_cast<const real_t*>(in);
    const real_t* in_q = in_i + 1;
    real_t* w_i = reinterpret_cast<real_t*>(exec_io);
    real_t* w_q = w_i + 1;
    switch (window) {
    case WindowType::BlackmanHarris:
        bh4(in_i, in_q, 2, w_i, w_q, 2, avg, fft_size);
        break;
    case WindowType::Hann:
        hann(in_i, in_q, 2, w_i, w_q, 2, avg, fft_size);
        break;
    case WindowType::Rect:
        rect(in_i, in_q, 2, w_i, w_q, 2, avg, fft_size);
        break;
    default:
        throw base::exception("Unsupported window type: "
            + window_type_map[window]);
    }
    exec_fft_il(exec_io, avg, fft_size);
    if (1 == avg) {
        scale_fft(out, fft_size);
    } else {
        cplx_to_polar(exec_io, avg * fft_size);
        average(exec_io, out, avg, fft_size);
        p2c_and_scale_fft(out, fft_size);
    }
}

// Quantized Real -> Complex
template <typename T>
void rfft(const T* in,
    size_t in_size,
    cplx_t* out,
    size_t out_size,
    int res,
    int avg,
    diff_t fft_size,
    WindowType window,
    CodeFormat format)
{
    check_array(in, in_size, "input array");
    check_array(out, out_size, "output array");
    check_resolution<T>(res);
    size_t out_size_exp = resolve_rfft_size(in_size, avg, fft_size);
    assert_eq(out_size, "output array size",
        out_size_exp, "FFTSize / 2 + 1");
    bool allocate = (1 < avg);
    cplx_vector tmp(allocate ? static_cast<size_t>(avg) * out_size : 0);
    cplx_t* exec_io = allocate ? tmp.data() : out;
    real_t* nw = reinterpret_cast<real_t*>(exec_io);
    switch (window) {
    case WindowType::BlackmanHarris:
        norm_bh4(in, nw, res, avg, fft_size, format);
        break;
    case WindowType::Hann:
        norm_hann(in, nw, res, avg, fft_size, format);
        break;
    case WindowType::Rect:
        norm_rect(in, nw, res, avg, fft_size, format);
        break;
    default:
        throw base::exception("Unsupported window type: "
            + window_type_map[window]);
    }
    exec_rfft(exec_io, avg, fft_size);
    if (1 == avg) {
        scale_rfft(out, fft_size);
    } else {
        cplx_to_polar(exec_io, static_cast<diff_t>(tmp.size()));
        average(exec_io, out, avg, static_cast<diff_t>(out_size));
        p2c_and_scale_rfft(out, fft_size);
    }
}

// Quantized Real -> M/P
template <typename T>
void rfft_mp(const T* in,
    size_t in_size,
    real_t* out_m,
    size_t out_m_size,
    real_t* out_p,
    size_t out_p_size,
    int res,
    int avg,
    diff_t fft_size,
    WindowType window,
    CodeFormat format)
{
    check_array(in, in_size, "input array");
    check_array_pair(out_m, out_m_size, "magnitude output array",
        out_p, out_p_size, "phase output array");
    size_t out_size_exp = resolve_rfft_size(in_size, avg, fft_size);
    assert_eq(out_m_size, "magnitude output array size",
        out_size_exp, "FFTSize / 2 + 1");
    cplx_vector ri(out_size_exp); // ri = real / imaginary
    rfft(in, in_size,
        ri.data(), out_size_exp,
        res, avg, fft_size, window, format);
    for (size_t i = 0; i < out_size_exp; ++i) {
        out_m[i] = std::norm(ri[i]);
        out_p[i] = std::arg(ri[i]);
    }
}

// Normalized Real -> Complex
void rfft_norm(const real_t* in,
    size_t in_size,
    cplx_t* out,
    size_t out_size,
    int avg,
    diff_t fft_size,
    WindowType window)
{
    check_array(in, in_size, "input array");
    check_array(out, out_size, "output array");
    size_t out_size_exp = resolve_rfft_size(in_size, avg, fft_size);
    assert_eq(out_size, "output array size",
        out_size_exp, "FFTSize / 2 + 1");
    bool allocate = (1 < avg);
    cplx_vector tmp(allocate ? static_cast<size_t>(avg) * out_size : 0);
    cplx_t* exec_io = allocate ? tmp.data() : out;
    real_t* w = reinterpret_cast<real_t*>(exec_io);
    switch (window) {
    case WindowType::BlackmanHarris:
        bh4(in, w, avg, fft_size);
        break;
    case WindowType::Hann:
        hann(in, w, avg, fft_size);
        break;
    case WindowType::Rect:
        rect(in, w, avg, fft_size);
        break;
    default:
        throw base::exception("Unsupported window type: "
            + window_type_map[window]);
    }
    exec_rfft(exec_io, avg, fft_size);
    if (1 == avg) {
        scale_rfft(out, fft_size);
    } else {
        cplx_to_polar(exec_io, static_cast<diff_t>(tmp.size()));
        average(exec_io, out, avg, static_cast<diff_t>(out_size));
        p2c_and_scale_rfft(out, fft_size);
    }
}

} // namespace analysis

namespace analysis { // Other functions

size_t resolve_fft_size(size_t in_size,
    int& avg,
    diff_t& fft_size,
    bool interleaved)
{
    // Get the "one-sided" input size.  For real input, this equals
    // in_size.  For complex input (interleaved), this equals in_size
    // divided by two.
    size_t in_size_1s_unsigned = in_size;
    if (interleaved) {
        if (0 != in_size % 2) {
            throw base::exception("Input array size, "
                + std::to_string(in_size)
                + ", must be even for interleaved data");
        }
        in_size_1s_unsigned /= 2;
    }
    // Check input size against absolute max.  FFTW, even with 64-bit
    // interface, still uses ptrdiff_t, which is signed.  This check is not
    // so much for practical cases, but to catch erroneous input.
    const diff_t diff_t_max = std::numeric_limits<diff_t>::max();
    if (0 == in_size_1s_unsigned || static_cast<size_t>(diff_t_max) < in_size_1s_unsigned) {
        throw base::exception("One-sided input array size, "
            + std::to_string(in_size_1s_unsigned)
            + ", out of range [ 1, "
            + std::to_string(diff_t_max) + " ]");
    }
    diff_t in_size_1s = static_cast<diff_t>(in_size_1s_unsigned);
    // Check averaging number against absolute max, albeit somewhat
    // arbitrary.
    if (256 < avg) {
        throw base::exception("FFT averaging number exceeds limit: 256 < "
            + std::to_string(avg));
    }
    // Proceed with resolving avg and fft_size:
    if (avg < 1 && fft_size < 1) {
        avg = 1;
        fft_size = in_size_1s;
    } else if (avg < 1) {
        diff_t _avg = in_size_1s / fft_size;
        if (256 < _avg) {
            throw base::exception("Derived FFT averaging number exceeds "
                                  "limit: 256 < "
                + std::to_string(_avg));
        }
        avg = static_cast<int>(_avg);
    } else if (fft_size < 1) {
        fft_size = in_size_1s / avg;
    } else {
        // This check prevents overflow in the final check
        diff_t fft_size_max = diff_t_max / avg;
        if (fft_size_max < fft_size) {
            throw base::exception("FFTSize exceeds MaxFFTSize / Avg: "
                + std::to_string(diff_t_max) + " / "
                + std::to_string(avg) + " < "
                + std::to_string(fft_size));
        }
    }
    assert_eq(static_cast<size_t>(in_size_1s),
        "one-sided input array size",
        static_cast<size_t>(avg * fft_size),
        "Avg * FFTSize");
    return static_cast<size_t>(fft_size);
}

size_t resolve_rfft_size(size_t in_size, int& avg, diff_t& fft_size)
{
    return resolve_fft_size(in_size, avg, fft_size, false) / 2 + 1;
}

real_t coherent_freq(size_t npts, real_t fs, real_t freq)
{
    assert_gt0(npts, "NPTS");
    check_fs(fs);
    if (1 == npts) {
        return freq;
    }
    real_t fbin = fs / npts;
    // if npts is a power of 2, return nearest odd integer
    if (is_pow2(npts)) {
        real_t cycles = std::floor(std::fabs(freq) / fbin);
        if (is_even(cycles)) {
            cycles += 1.0;
        }
        return std::copysign(cycles * fbin, freq);
    }
    // Find the nearest integer number of cycles that is prime relative to
    // npts, that is, the nearest integer for which the GCD is 1.

    // And until this is implemented, just return the nearest integer:
    return std::round(freq / fbin) * fbin;

    //        real_t cycles = std::abs(freq) / fbin;
    //        if (1.5 > cycles) {
    //            return std::copysign(fbin, freq);
    //        }
    //        else {
    //            auto npts_real = static_cast<real_t>(npts);
    //            real_t nnpts = std::floor(cycles / npts_real);
    //            cycles = std::fmod(cycles, npts_real);
    //            auto c0 = static_cast<diff_t>(std::round(cycles));
    //            // if not coprime, search for nearest coprime
    //            if (1 != boost::math::gcd(c0, npts)) {
    //                // if rounded up, try down first, otherwise up
    //                diff_t dir = (static_cast<real_t>(c0) > cycles) ? -1 : 1;
    //                while (true) {
    //                    if (1 == boost::math::gcd(c0 + x, npts)) {
    //                        break;
    //                    } else if (1 == boost::math::gcd(c0 - x, npts)) {
    //                        break;
    //                    }
    //                }
    //            }
    //            cycles = c0 + nnpts * npts_real;
    //        }
    //        return std::copysign(cycles * fbin, freq);
}

std::vector<real_t> coherent_freq(size_t npts,
    real_t fs,
    std::vector<real_t> freq)
{
    for (auto& f : freq) {
        f = coherent_freq(npts, fs, f);
    }
    return freq;
}

real_t fft_alias(real_t fs, real_t freq, bool shift)
{
    check_fs(fs);
    freq -= std::floor(freq / fs) * fs;
    // note the <= : for shifted alias, +fs/2 aliases to -fs/2
    return (shift && fs <= 2 * freq) ? freq - fs : freq;
}

real_vector fft_axis(size_t npts, FFTAxisFormat fmt, real_t fs, bool shift)
{
    assert_gt0(npts, "NPTS");
    real_vector axis(npts);
    real_t start = shift ? -std::floor(npts / 2) : 0.0;
    std::iota(axis.begin(), axis.end(), start);
    real_t step = 1.0;
    if (FFTAxisFormat::Freq == fmt) {
        step = fs / npts;
    } else if (FFTAxisFormat::Norm == fmt) {
        step /= npts;
    }
    for (real_t& x : axis) {
        x *= step;
    }
    return axis;
}

diff_t fft_bin(size_t npts, real_t fs, real_t freq, bool shift)
{
    assert_gt0(npts, "NPTS");
    auto n = static_cast<diff_t>(npts);
    freq = fft_alias(fs, freq, false); // [-fs/2, fs/2)
    auto bin = static_cast<diff_t>(std::floor(freq / (fs / n) + 0.5));
    if (shift) {
        return ((n + 1) / 2 <= bin) ? bin - n : bin;
    } else {
        return bin % n; // mod is only for case: bin == npts
    }
}

size_t fft_index(size_t npts, real_t fs, real_t freq, bool shift)
{
    diff_t bin = fft_bin(npts, fs, freq, shift);
    auto n = static_cast<diff_t>(npts);
    return static_cast<size_t>(shift ? bin + n / 2 : bin);
}

template <typename T>
void fft_shift(const T* in_data,
    size_t in_size,
    T* out_data,
    size_t out_size)
{
    check_array_pair(in_data, in_size, "input array",
        out_data, out_size, "output array");
    size_t offset = in_size / 2;
    if (1 == offset % 2) {
        offset += 1;
    }
    if (in_data == out_data) {
        std::rotate(out_data, out_data + offset, out_data + out_size);
    } else {
        std::rotate_copy(in_data, in_data + offset, in_data + in_size,
            out_data);
    }
}

template <typename T>
void ifft_shift(const T* in_data,
    size_t in_size,
    T* out_data,
    size_t out_size)
{
    check_array_pair(in_data, in_size, "input array",
        out_data, out_size, "output array");
    size_t offset = in_size / 2;
    if (in_data == out_data) {
        std::rotate(out_data, out_data + offset, out_data + out_size);
    } else {
        std::rotate_copy(in_data, in_data + offset, in_data + in_size,
            out_data);
    }
}

real_t rfft_alias(real_t fs, real_t freq)
{
    check_fs(fs);
    freq -= std::floor(freq / fs) * fs;
    return (fs < 2 * freq) ? fs - freq : freq;
}

real_vector rfft_axis(size_t npts, FFTAxisFormat fmt, real_t fs)
{
    assert_gt0(npts, "NPTS");
    real_vector axis(npts / 2 + 1);
    std::iota(axis.begin(), axis.end(), 0.0);
    real_t step = 1.0;
    if (FFTAxisFormat::Freq == fmt) {
        step = fs / npts;
    } else if (FFTAxisFormat::Norm == fmt) {
        step /= npts;
    }
    for (real_t& x : axis) {
        x *= step;
    }
    return axis;
}

diff_t rfft_bin(size_t npts, real_t fs, real_t freq)
{
    assert_gt0(npts, "NPTS");
    auto n = static_cast<diff_t>(npts);
    freq = fft_alias(fs, freq, false); // [-fs/2, fs/2)
    auto bin = static_cast<diff_t>(std::floor(freq / (fs / n) + 0.5));
    return (n / 2 < bin) ? n - bin : bin;
}

size_t rfft_index(size_t npts, real_t fs, real_t freq)
{
    return static_cast<size_t>(rfft_bin(npts, fs, freq));
}

} // namespace analysis - FFT Utilities

namespace analysis { // Allocating Functions

namespace {

    //        static const int init_threads = fftw_init_threads();

    //        static const int num_threads = 8;
}

namespace utils {

    void check_size(size_t size)
    {
        if (size < min_npts || max_npts < size) {
            throw base::exception("size out of range [ "
                + std::to_string(min_npts) + ", "
                + std::to_string(max_npts) + " ]");
        }
    }

    void check_sizes(size_t size1, size_t size2)
    {
        if (size1 != size2) {
            throw base::exception("Sizes do not match");
        }
    }

    bool is_odd(size_t n)
    {
        return 1 == n % 2;
    }
}

cplx_vector fft(const cplx_vector& tdata)
{
    const size_t npts = tdata.size();
    utils::check_size(npts);
    cplx_vector fdata(npts);
    cplx_t* p_tdata = const_cast<cplx_t*>(tdata.data());
    cplx_t* p_fdata = fdata.data();
    fftw_complex* p_tfftw = reinterpret_cast<fftw_complex*>(p_tdata);
    fftw_complex* p_ffftw = reinterpret_cast<fftw_complex*>(p_fdata);
    auto plan = fftw_plan_dft_1d((int)npts, p_tfftw, p_ffftw,
        FFTW_FORWARD, FFTW_ESTIMATE);
    if (nullptr == plan) {
        throw base::exception("FFTW planner failed.");
    }
    fftw_execute(plan);
    fftw_destroy_plan(plan);
    const real_t scale_factor = 1.0 / npts;
    for (size_t i = 0; i < npts; ++i) {
        fdata[i] *= scale_factor;
    }
    return fdata;
}

cplx_vector fft(const real_vector_pair& data)
{
    const real_vector& idata = data.first;
    const real_vector& qdata = data.second;
    const size_t npts = idata.size();
    utils::check_sizes(npts, qdata.size());
    utils::check_size(npts);
    cplx_vector iqdata(npts);
    for (size_t i = 0; i < npts; ++i) {
        iqdata[i] = { idata[i], qdata[i] };
    }
    return fft(iqdata);
}

cplx_vector fft(const real_vector& data)
{
    const size_t npts = data.size();
    utils::check_size(npts);
    cplx_vector iqdata(npts);
    for (size_t i = 0; i < npts; ++i) {
        iqdata[i] = { data[i], 0.0 };
    }
    return fft(iqdata);
}

real_vector_pair cplx_split(const cplx_vector& data)
{
    size_t npts = data.size();
    real_vector_pair out{ real_vector(npts), real_vector(npts) };
    real_vector& re = out.first;
    real_vector& im = out.second;
    for (size_t i = 0; i < npts; ++i) {
        re[i] = data[i].real();
        im[i] = data[i].imag();
    }
    return out;
}

real_vector_pair ifft(const cplx_vector& data)
{
    size_t npts = data.size();
    utils::check_size(npts);
    cplx_vector cwvf(npts);
    auto npts_int = static_cast<int>(npts);
    auto pdata = const_cast<cplx_t*>(data.data());
    auto pfft = reinterpret_cast<fftw_complex*>(pdata);
    auto pwvf = reinterpret_cast<fftw_complex*>(cwvf.data());
    auto plan = fftw_plan_dft_1d(npts_int, pfft, pwvf,
        FFTW_BACKWARD, FFTW_ESTIMATE);
    if (nullptr == plan) {
        throw base::exception("FFTW planner error");
    }
    fftw_execute(plan);
    fftw_destroy_plan(plan);
    return cplx_split(cwvf);
}

real_vector irfft(cplx_vector data, size_t npts)
{
    size_t size = data.size();
    if ((2 * size - 1) != npts) { // if not the only possible odd
        npts = 2 * (size - 1); // then assume even
    }
    utils::check_size(npts);
    real_vector wvf(npts);
    auto npts_int = static_cast<int>(npts);
    auto pfft = reinterpret_cast<fftw_complex*>(data.data());
    auto pwvf = wvf.data();
    auto plan = fftw_plan_dft_c2r_1d(npts_int, pfft, pwvf, FFTW_ESTIMATE);
    if (nullptr == plan) {
        throw base::exception("FFTW planner error");
    }
    // Undo the scaling done in rfft, except for scaling by npts.
    data.front() /= k_sqrt2;
    for (size_t i = 1; i < size - 1; ++i) {
        data[i] /= 2.0;
    }
    if (1 < npts_int) {
        data.back() /= k_sqrt2;
        if (utils::is_odd(npts)) {
            data.back() /= k_sqrt2;
        }
    }
    fftw_execute(plan);
    fftw_destroy_plan(plan);
    return wvf;
}

cplx_vector rfft(const real_vector& data)
{
    //        if (0 != init_threads) {
    //            fftw_plan_with_nthreads(num_threads);
    //        }
    const size_t npts = data.size();
    utils::check_size(npts);
    const size_t size = npts / 2 + 1; // integer division intentional
    cplx_vector _fft(size);
    const int npts_int = static_cast<int>(npts);
    real_t* pwvf = const_cast<real_t*>(data.data());
    fftw_complex* pfft = reinterpret_cast<fftw_complex*>(_fft.data());
    auto plan = fftw_plan_dft_r2c_1d(npts_int, pwvf, pfft, FFTW_ESTIMATE);
    if (nullptr == plan) {
        throw base::exception("FFTW planner error");
    }
    fftw_execute(plan);
    fftw_destroy_plan(plan);
    //        if (0 != init_threads) {
    //            fftw_cleanup_threads();
    //        }
    _fft.front() *= k_sqrt2 / npts_int;
    for (size_t i = 1; i < size - 1; ++i) { // skip [0] and [-1]
        _fft[i] *= 2.0 / npts_int;
    }
    if (1 < npts_int) {
        _fft.back() *= k_sqrt2 / npts_int;
        // Last bin for even npts is fs/2, for which there is no folding.
        // Last bin for odd npts is not fs/2, hence *= sqrt2.
        if (utils::is_odd(npts)) {
            _fft.back() *= k_sqrt2;
        }
    }
    return _fft;
}

cplx_vector sfft(const cplx_vector& data)
{
    cplx_vector _fft = fft(data);
    fft_shift(_fft.data(), _fft.size(), _fft.data(), _fft.size());
    return _fft;
}

cplx_vector sfft(const real_vector& data)
{
    cplx_vector _fft = fft(data);
    fft_shift(_fft.data(), _fft.size(), _fft.data(), _fft.size());
    return _fft;
}

} // namespace analysis - Allocating Functions

namespace analysis { // Template Instantiations

// Next 3: Quantized IQ -> Complex
template ICD_ANALYSIS_DECL void fft(const int16_t*, size_t, const int16_t*, size_t, diff_t,
    cplx_t*, size_t,
    int, int, diff_t, WindowType, CodeFormat);

template ICD_ANALYSIS_DECL void fft(const int32_t*, size_t, const int32_t*, size_t, diff_t,
    cplx_t*, size_t,
    int, int, diff_t, WindowType, CodeFormat);

template ICD_ANALYSIS_DECL void fft(const int64_t*, size_t, const int64_t*, size_t, diff_t,
    cplx_t*, size_t,
    int, int, diff_t, WindowType, CodeFormat);

// Next 3: Quantized IQ -> M/P
template ICD_ANALYSIS_DECL void fft_mp(const int16_t*, size_t, const int16_t*, size_t, diff_t,
    real_t*, size_t, real_t*, size_t,
    int, int, diff_t, WindowType, CodeFormat);

template ICD_ANALYSIS_DECL void fft_mp(const int32_t*, size_t, const int32_t*, size_t, diff_t,
    real_t*, size_t, real_t*, size_t,
    int, int, diff_t, WindowType, CodeFormat);

template ICD_ANALYSIS_DECL void fft_mp(const int64_t*, size_t, const int64_t*, size_t, diff_t,
    real_t*, size_t, real_t*, size_t,
    int, int, diff_t, WindowType, CodeFormat);

// Next 3: Quantized Real -> Complex
template ICD_ANALYSIS_DECL void rfft(const int16_t*, size_t,
    cplx_t*, size_t,
    int, int, diff_t, WindowType, CodeFormat);

template ICD_ANALYSIS_DECL void rfft(const int32_t*, size_t,
    cplx_t*, size_t,
    int, int, diff_t, WindowType, CodeFormat);

template ICD_ANALYSIS_DECL void rfft(const int64_t*, size_t,
    cplx_t*, size_t,
    int, int, diff_t, WindowType, CodeFormat);

// Next 3: Quantized Real -> M/P
template ICD_ANALYSIS_DECL void rfft_mp(const int16_t*, size_t,
    real_t*, size_t, real_t*, size_t,
    int, int, diff_t, WindowType, CodeFormat);

template ICD_ANALYSIS_DECL void rfft_mp(const int32_t*, size_t,
    real_t*, size_t, real_t*, size_t,
    int, int, diff_t, WindowType, CodeFormat);

template ICD_ANALYSIS_DECL void rfft_mp(const int64_t*, size_t,
    real_t*, size_t, real_t*, size_t,
    int, int, diff_t, WindowType, CodeFormat);

template ICD_ANALYSIS_DECL void fft_shift(const cplx_t*, size_t, cplx_t*, size_t);

template ICD_ANALYSIS_DECL void fft_shift(const real_t*, size_t, real_t*, size_t);

template ICD_ANALYSIS_DECL void ifft_shift(const cplx_t*, size_t, cplx_t*, size_t);

template ICD_ANALYSIS_DECL void ifft_shift(const real_t*, size_t, real_t*, size_t);

} // namespace analysis - Template Instantiations
