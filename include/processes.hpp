// Copyright (C) 2024-2026 Analog Devices, Inc.
//
// SPDX short identifier: ADIBSD OR GPL-2.0-or-later
#ifndef GENALYZER_IMPL_PROCESSES_HPP
#define GENALYZER_IMPL_PROCESSES_HPP

#include "enums.hpp"
#include "type_aliases.hpp"

namespace genalyzer_impl {

/**
 * @brief Decimate a waveform by keeping every Nth sample.
 *
 * If @p interleaved is true, the input is treated as interleaved I/Q pairs
 * and decimation preserves pair alignment.
 *
 * @tparam T          Sample type.
 * @param in_data     Pointer to input data.
 * @param in_size     Number of elements in @p in_data.
 * @param out_data    Pointer to output (decimated) data.
 * @param out_size    Number of elements in @p out_data (use downsample_size()).
 * @param ratio       Decimation ratio (keep every @p ratio-th sample).
 * @param interleaved If true, treat data as interleaved I/Q pairs.
 */
template <typename T>
void downsample(const T *in_data, size_t in_size, T *out_data, size_t out_size,
		int ratio, bool interleaved);

/**
 * @brief Return the output array size for downsample().
 *
 * @param in_size     Number of elements in the input array.
 * @param ratio       Decimation ratio.
 * @param interleaved If true, input is interleaved I/Q pairs.
 * @return Required output array size.
 */
size_t downsample_size(size_t in_size, int ratio, bool interleaved);

/**
 * @brief Apply a frequency shift to normalized (floating-point) complex data.
 *
 * Multiplies the input by exp(j*2*pi*fshift*n/fs). If @p q_size is 0,
 * @p i_data contains interleaved I/Q; otherwise @p i_data and @p q_data
 * are separate I and Q channels. Output is always interleaved I/Q.
 *
 * @param i_data   Pointer to I data (or interleaved I/Q if @p q_size is 0).
 * @param i_size   Number of elements in @p i_data.
 * @param q_data   Pointer to Q data (may be nullptr if @p q_size is 0).
 * @param q_size   Number of elements in @p q_data (0 for interleaved input).
 * @param out_data Pointer to output array for interleaved I/Q result.
 * @param out_size Number of elements in @p out_data (use fshift_size()).
 * @param fs       Sample rate in Hz.
 * @param _fshift  Frequency shift in Hz.
 */
void fshift(const real_t *i_data, size_t i_size, const real_t *q_data,
		size_t q_size, real_t *out_data, size_t out_size, real_t fs,
		real_t _fshift);

/**
 * @brief Apply a frequency shift to quantized (integer) complex data.
 *
 * Results are rounded and clamped to the valid code range for the given
 * resolution and format.
 *
 * @tparam T       Integer sample type (int16_t, int32_t, or int64_t).
 * @param i_data   Pointer to I data (or interleaved I/Q if @p q_size is 0).
 * @param i_size   Number of elements in @p i_data.
 * @param q_data   Pointer to Q data (may be nullptr if @p q_size is 0).
 * @param q_size   Number of elements in @p q_data (0 for interleaved input).
 * @param out_data Pointer to output array for interleaved I/Q result.
 * @param out_size Number of elements in @p out_data (use fshift_size()).
 * @param n        ADC resolution in bits.
 * @param fs       Sample rate in Hz.
 * @param _fshift  Frequency shift in Hz.
 * @param format   Code format of the input and output samples.
 */
template <typename T>
void fshift(const T *i_data, size_t i_size, const T *q_data, size_t q_size,
		T *out_data, size_t out_size, int n, real_t fs, real_t _fshift,
		CodeFormat format);

/**
 * @brief Return the output array size for fshift().
 *
 * @param i_size Number of elements in the I data array.
 * @param q_size Number of elements in the Q data array (0 for interleaved).
 * @return Required output array size.
 */
size_t fshift_size(size_t i_size, size_t q_size);

/**
 * @brief Convert quantized integer samples to normalized floating-point values.
 *
 * Output values are in the range [-1, 1). For offset binary, the offset is
 * subtracted before scaling. For two's complement, values are scaled directly
 * by 2/2^n.
 *
 * @tparam T       Integer sample type.
 * @param in_data  Pointer to input quantized data.
 * @param in_size  Number of elements in @p in_data.
 * @param out_data Pointer to output normalized data.
 * @param out_size Number of elements in @p out_data (must equal @p in_size).
 * @param n        ADC resolution in bits.
 * @param format   Code format of the input samples.
 */
template <typename T>
void normalize(const T *in_data, size_t in_size, real_t *out_data,
		size_t out_size, int n, CodeFormat format);

/**
 * @brief Evaluate a polynomial at each point in the input array.
 *
 * Uses Horner's method. Coefficients are ordered as [c0, c1, c2, ...] where
 * y = c0 + c1*x + c2*x^2 + ... This is useful for modeling nonlinear
 * distortion.
 *
 * @param in_data  Pointer to input data (x values).
 * @param in_size  Number of elements in @p in_data.
 * @param out_data Pointer to output data (y values).
 * @param out_size Number of elements in @p out_data (must equal @p in_size).
 * @param c_data   Pointer to polynomial coefficients [c0, c1, c2, ...].
 * @param c_size   Number of coefficients.
 */
void polyval(const real_t *in_data, size_t in_size, real_t *out_data,
		size_t out_size, const real_t *c_data, size_t c_size);

/**
 * @brief Quantize floating-point samples to integer codes.
 *
 * Quantizes with the given full-scale range and resolution. Optionally adds
 * Gaussian quantization noise. Output codes are clamped to the valid range
 * for the specified resolution and format.
 *
 * @tparam T       Integer output type.
 * @param in_data  Pointer to input floating-point data.
 * @param in_size  Number of elements in @p in_data.
 * @param out_data Pointer to output quantized data.
 * @param out_size Number of elements in @p out_data (must equal @p in_size).
 * @param fsr      Full-scale range.
 * @param n        ADC resolution in bits.
 * @param noise    RMS level of additive Gaussian quantization noise (0 for none).
 * @param format   Code format for the output samples.
 */
template <typename T>
void quantize(const real_t *in_data, size_t in_size, T *out_data,
		size_t out_size, real_t fsr, int n, real_t noise,
		CodeFormat format);

} // namespace genalyzer_impl

#endif // GENALYZER_IMPL_PROCESSES_HPP