/*
File        : $HeadURL: https://swdev.cld.analog.com/svn/projects/icdev/sandbox/users/pderouni/platform-upgrades/src/analysis/dft.hpp $
Originator  : pderouni
Revision    : $Revision: 12382 $
Last Commit : $Date: 2020-04-06 11:21:16 -0400 (Mon, 06 Apr 2020) $
Last Editor : $Author: pderouni $
*/

#ifndef ICD_ANALYSIS_DFT_HPP
#define ICD_ANALYSIS_DFT_HPP

#include "analysis.hpp"
#include "enums.hpp"
#include "types.hpp"

#define ICD_API ICD_ANALYSIS_DECL

/*
 *
 *     dBFS vs dBFSsin or dBFSs
 *
 */

namespace analysis { /// @{ @ingroup DFT

ICD_API real_t coherent_freq(size_t npts, real_t fs, real_t freq);

ICD_API std::vector<real_t> coherent_freq(size_t npts,
    real_t fs,
    std::vector<real_t> freq);

/**
     * @brief fft Quantized IQ input to complex output
     * @tparam T int16_t, int32_t, int64_t
     */
template <typename T>
void fft(const T* in_i, ///< [in]  pointer to in-phase input array
    size_t in_i_size, ///< [in]  size of array in_i
    const T* in_q, ///< [in]  pointer to quadrature input array
    size_t in_q_size, ///< [in]  size of array in_q
    diff_t in_stride, ///< [in]  stride of input arrays
    cplx_t* out, ///< [out] pointer to output array
    size_t out_size, ///< [in]  size of array out
    int res, ///< [in]  code resolution
    int avg, ///< [in]  FFT averaging number
    diff_t fft_size, ///< [in]  FFT size
    WindowType window, ///< [in]  window type
    CodeFormat format ///< [in]  code format
);

ICD_API real_t fft_alias(real_t fs, real_t freq, bool shift);

ICD_API real_vector fft_axis(size_t npts,
    FFTAxisFormat fmt,
    real_t fs,
    bool shift);

ICD_API diff_t fft_bin(size_t npts, real_t fs, real_t freq, bool shift);

ICD_API size_t fft_index(size_t npts, real_t fs, real_t freq, bool shift);

/**
     * @brief fft Quantized IQ input to mean-square magnitude and phase
     *        output
     * @tparam T int16_t, int32_t, int64_t
     */
template <typename T>
void fft_mp(const T* in_i, ///< [in]  pointer to in-phase input array
    size_t in_i_size, ///< [in]  size of array in_i
    const T* in_q, ///< [in]  pointer to quadrature input array
    size_t in_q_size, ///< [in]  size of array in_q
    diff_t in_stride, ///< [in]  stride of input arrays
    real_t* out_m, ///< [out] pointer to magnitude output array
    size_t out_m_size, ///< [in]  size of array out_m
    real_t* out_p, ///< [out] pointer to phase output array
    size_t out_p_size, ///< [in]  size of array out_p
    int res, ///< [in]  code resolution
    int avg, ///< [in]  FFT averaging number
    diff_t fft_size, ///< [in]  FFT size
    WindowType window, ///< [in]  window type
    CodeFormat format ///< [in]  code format
);

/**
     * @brief fft_norm Normalized IQ input to complex output
     */
ICD_API void fft_norm(const cplx_t* in, ///< [in]  pointer to input array
    size_t in_size, ///< [in]  size of array in
    cplx_t* out, ///< [out] pointer to output array
    size_t out_size, ///< [in]  size of array out
    int avg, ///< [in]  FFT averaging number
    diff_t fft_size, ///< [in]  FFT size
    WindowType window ///< [in]  window type
);

/**
     * @brief fft_shift
     * @tparam T cplx_t, real_t
     */
template <typename T>
void fft_shift(const T* in_data, ///< [in]  input array pointer
    size_t in_size, ///< [in]  input array size
    T* out_data, ///< [out] output array pointer
    size_t out_size ///< [in]  output array size
);

/**
     * @brief fft_shift
     * @tparam T cplx_t, real_t
     */
template <typename T>
void ifft_shift(const T* in_data, ///< [in]  input array pointer
    size_t in_size, ///< [in]  input array size
    T* out_data, ///< [out] output array pointer
    size_t out_size ///< [in]  output array size
);

/**
     * @brief resolve_fft_size
     * @param [in]     in_size     Size of input array
     * @param [in,out] avg         FFT averaging number
     * @param [in,out] fft_size    FFT size
     * @param [in]     interleaved
     * @return Expected output array size
     */
ICD_API size_t resolve_fft_size(size_t in_size,
    int& avg,
    diff_t& fft_size,
    bool interleaved);

/**
     * @brief resolve_rfft_size
     * @param [in]     in_size  Size of input array
     * @param [in,out] avg      FFT averaging number
     * @param [in,out] fft_size FFT size
     * @return Expected output array size
     */
ICD_API size_t resolve_rfft_size(size_t in_size,
    int& avg,
    diff_t& fft_size);

/**
     * @brief rfft Quantized real input to complex output
     * @tparam T int16_t, int32_t, int64_t
     */
template <typename T>
void rfft(const T* in, ///< [in]  pointer to input array
    size_t in_size, ///< [in]  size of array in
    cplx_t* out, ///< [out] pointer to output array
    size_t out_size, ///< [in]  size of array out
    int res, ///< [in]  code resolution
    int avg, ///< [in]  FFT averaging number
    diff_t fft_size, ///< [in]  FFT size
    WindowType window, ///< [in]  window type
    CodeFormat format ///< [in]  code format
);

ICD_API real_t rfft_alias(real_t fs, real_t freq);

ICD_API real_vector rfft_axis(size_t npts, FFTAxisFormat fmt, real_t fs);

ICD_API diff_t rfft_bin(size_t npts, real_t fs, real_t freq);

ICD_API size_t rfft_index(size_t npts, real_t fs, real_t freq);

/**
     * @brief rfft Quantized real input to mean-square magnitude and phase
     *             output
     * @tparam T int16_t, int32_t, int64_t
     */
template <typename T>
void rfft_mp(const T* in, ///< [in]  pointer to input array
    size_t in_size, ///< [in]  size of array in
    real_t* out_m, ///< [out] pointer to magnitude output array
    size_t out_m_size, ///< [in]  size of array out_m
    real_t* out_p, ///< [out] pointer to phase output array
    size_t out_p_size, ///< [in]  size of array out_p
    int res, ///< [in]  code resolution
    int avg, ///< [in]  FFT averaging number
    diff_t fft_size, ///< [in]  FFT size
    WindowType window, ///< [in]  window type
    CodeFormat format ///< [in]  code format
);

/**
     * @brief rfft_norm Normalized real input to complex output
     */
ICD_API void rfft_norm(const real_t* in, ///< [in]  pointer to input array
    size_t in_size, ///< [in]  size of array in
    cplx_t* out, ///< [out] pointer to output array
    size_t out_size, ///< [in]  size of array out
    int avg, ///< [in]  FFT averaging number
    diff_t fft_size, ///< [in]  FFT size
    WindowType window ///< [in]  window type
);

/// @} DFT

} // namespace analysis

namespace analysis { // Allocating Functions, consider deprecated

using real_vector_pair = std::pair<real_vector, real_vector>;

ICD_API cplx_vector fft(const cplx_vector& data);

ICD_API cplx_vector fft(const real_vector_pair& data);

ICD_API cplx_vector fft(const real_vector& data);

ICD_API real_vector_pair ifft(const cplx_vector& data);

ICD_API real_vector irfft(cplx_vector data, size_t npts);

ICD_API cplx_vector rfft(const real_vector& data);

ICD_API cplx_vector sfft(const cplx_vector& data);

ICD_API cplx_vector sfft(const real_vector& data);

} // namespace analysis - Allocating Functions

#undef ICD_API

#endif // ICD_ANALYSIS_DFT_HPP
