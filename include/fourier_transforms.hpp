// Copyright (C) 2024-2025 Analog Devices, Inc.
//
// SPDX short identifier: ADIBSD OR GPL-2.0-or-later
#ifndef GENALYZER_IMPL_FOURIER_TRANSFORMS_HPP
#define GENALYZER_IMPL_FOURIER_TRANSFORMS_HPP

#include "enums.hpp"
#include "type_aliases.hpp"

namespace genalyzer_impl {

/**
 * @brief Compute the complex FFT of normalized (floating-point) I/Q data.
 *
 * Supports averaging over multiple records. If @p q_size is 0, @p i_data
 * contains interleaved I/Q samples; otherwise @p i_data and @p q_data are
 * separate I and Q channels. Output is an interleaved real/imaginary complex
 * FFT result.
 *
 * @param i_data   Pointer to I data (or interleaved I/Q if @p q_size is 0).
 * @param i_size   Number of elements in @p i_data.
 * @param q_data   Pointer to Q data (may be nullptr if @p q_size is 0).
 * @param q_size   Number of elements in @p q_data (0 for interleaved input).
 * @param out_data Pointer to output array for interleaved complex FFT result.
 * @param out_size Number of elements in @p out_data (use fft_size() to compute).
 * @param navg     Number of records to average (0 for auto-detect).
 * @param nfft     FFT size (0 for auto-detect).
 * @param window   Window function to apply before the FFT.
 */
void fft(const real_t *i_data, size_t i_size, const real_t *q_data,
		size_t q_size, real_t *out_data, size_t out_size, size_t navg,
		size_t nfft, Window window);

/**
 * @brief Compute the complex FFT of quantized (integer) I/Q data.
 *
 * Template supports int16_t, int32_t, and int64_t. Data is internally
 * normalized to floating-point before the FFT computation.
 *
 * @tparam T       Integer sample type (int16_t, int32_t, or int64_t).
 * @param i_data   Pointer to I data (or interleaved I/Q if @p q_size is 0).
 * @param i_size   Number of elements in @p i_data.
 * @param q_data   Pointer to Q data (may be nullptr if @p q_size is 0).
 * @param q_size   Number of elements in @p q_data (0 for interleaved input).
 * @param out_data Pointer to output array for interleaved complex FFT result.
 * @param out_size Number of elements in @p out_data (use fft_size() to compute).
 * @param n        ADC resolution in bits.
 * @param navg     Number of records to average (0 for auto-detect).
 * @param nfft     FFT size (0 for auto-detect).
 * @param window   Window function to apply before the FFT.
 * @param format   Code format of the input samples.
 */
template <typename T>
void fft(const T *i_data, size_t i_size, const T *q_data, size_t q_size,
		real_t *out_data, size_t out_size, int n, size_t navg, size_t nfft,
		Window window, CodeFormat format);

/**
 * @brief Compute the required output array size for fft().
 *
 * Also resolves @p navg and @p nfft if they are set to 0 (auto-detect).
 *
 * @param i_size Number of elements in the I data array.
 * @param q_size Number of elements in the Q data array (0 for interleaved).
 * @param navg   Number of averages (input/output; resolved if 0).
 * @param nfft   FFT size (input/output; resolved if 0).
 * @return Required output array size (2 * nfft for interleaved complex output).
 */
size_t fft_size(size_t i_size, size_t q_size, size_t &navg, size_t &nfft);

/**
 * @brief Compute the real FFT (one-sided spectrum) of normalized data.
 *
 * Output is interleaved real/imaginary of size nfft+2 (i.e., nfft/2+1
 * complex bins). The scale parameter controls the dBFS convention.
 *
 * @param in_data  Pointer to real input data.
 * @param in_size  Number of elements in @p in_data.
 * @param out_data Pointer to output array for interleaved complex FFT result.
 * @param out_size Number of elements in @p out_data (use rfft_size() to compute).
 * @param navg     Number of records to average (0 for auto-detect).
 * @param nfft     FFT size (0 for auto-detect).
 * @param window   Window function to apply before the FFT.
 * @param scale    dBFS scaling convention.
 */
void rfft(const real_t *in_data, size_t in_size, real_t *out_data,
		size_t out_size, size_t navg, size_t nfft, Window window,
		RfftScale scale);

/**
 * @brief Compute the real FFT of quantized (integer) data.
 *
 * Template supports int16_t, int32_t, and int64_t. Data is internally
 * normalized before the FFT computation.
 *
 * @tparam T       Integer sample type (int16_t, int32_t, or int64_t).
 * @param in_data  Pointer to quantized input data.
 * @param in_size  Number of elements in @p in_data.
 * @param out_data Pointer to output array for interleaved complex FFT result.
 * @param out_size Number of elements in @p out_data (use rfft_size() to compute).
 * @param n        ADC resolution in bits.
 * @param navg     Number of records to average (0 for auto-detect).
 * @param nfft     FFT size (0 for auto-detect).
 * @param window   Window function to apply before the FFT.
 * @param format   Code format of the input samples.
 * @param scale    dBFS scaling convention.
 */
template <typename T>
void rfft(const T *in_data, size_t in_size, real_t *out_data, size_t out_size,
		int n, size_t navg, size_t nfft, Window window, CodeFormat format,
		RfftScale scale);

/**
 * @brief Compute the required output array size for rfft().
 *
 * Also resolves @p navg and @p nfft if they are set to 0 (auto-detect).
 *
 * @param in_size Number of elements in the input data array.
 * @param navg    Number of averages (input/output; resolved if 0).
 * @param nfft    FFT size (input/output; resolved if 0).
 * @return Required output array size (nfft + 2).
 */
size_t rfft_size(size_t in_size, size_t &navg, size_t &nfft);

} // namespace genalyzer_impl

#endif // GENALYZER_IMPL_FOURIER_TRANSFORMS_HPP