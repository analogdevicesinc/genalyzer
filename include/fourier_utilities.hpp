// Copyright (C) 2024-2026 Analog Devices, Inc.
//
// SPDX short identifier: ADIBSD OR GPL-2.0-or-later
#ifndef GENALYZER_IMPL_FOURIER_UTILITIES_HPP
#define GENALYZER_IMPL_FOURIER_UTILITIES_HPP

#include "enums.hpp"
#include "type_aliases.hpp"

namespace genalyzer_impl {

/**
 * @brief Compute the aliased frequency for a given sample rate.
 *
 * Maps the input frequency into the appropriate Nyquist zone based on the
 * axis type: DcCenter maps to [-fs/2, fs/2), DcLeft maps to [0, fs), and
 * Real maps to [0, fs/2].
 *
 * @param fs        Sample rate in Hz.
 * @param freq      Input frequency in Hz.
 * @param axis_type Frequency axis type determining the alias range.
 * @return The aliased frequency in Hz.
 */
real_t alias(real_t fs, real_t freq, FreqAxisType axis_type);

/**
 * @brief Compute the nearest coherent frequency for a given FFT size.
 *
 * Coherent sampling ensures the signal frequency lands exactly on an FFT bin,
 * eliminating spectral leakage. For power-of-2 FFT sizes, the returned
 * frequency corresponds to an odd number of cycles within the record.
 *
 * @param nfft FFT size.
 * @param fs   Sample rate in Hz.
 * @param freq Desired frequency in Hz.
 * @return The nearest coherent frequency in Hz.
 */
real_t coherent(size_t nfft, real_t fs, real_t freq);

/**
 * @brief Shift the zero-frequency component to the center of the array.
 *
 * Performs a circular shift by N/2 (rounded up for odd sizes), converting
 * a DC-left spectrum to a DC-center spectrum. Supports in-place operation
 * (i.e., @p in_data and @p out_data may point to the same buffer).
 *
 * @param in_data  Pointer to input array.
 * @param in_size  Number of elements in @p in_data.
 * @param out_data Pointer to output array.
 * @param out_size Number of elements in @p out_data (must equal @p in_size).
 */
void fftshift(const real_t *in_data, size_t in_size, real_t *out_data,
		size_t out_size);

/**
 * @brief Generate a frequency axis array for plotting FFT results.
 *
 * Supports DC-center [-fs/2, fs/2), DC-left [0, fs), and real [0, fs/2]
 * layouts. Output values can be expressed in bins, Hz, or normalized frequency.
 *
 * @param data        Pointer to output array for frequency axis values.
 * @param size        Number of elements in @p data (use freq_axis_size()).
 * @param nfft        FFT size.
 * @param axis_type   Frequency axis layout type.
 * @param fs          Sample rate in Hz.
 * @param axis_format Output format (bins, Hz, or normalized).
 */
void freq_axis(real_t *data, size_t size, size_t nfft, FreqAxisType axis_type,
		real_t fs, FreqAxisFormat axis_format);

/**
 * @brief Inverse of fftshift: shift the zero-frequency component back to
 * the left of the array.
 *
 * Performs a circular shift of N/2 (rounded down), converting a DC-center
 * spectrum to a DC-left spectrum. Supports in-place operation.
 *
 * @param in_data  Pointer to input array.
 * @param in_size  Number of elements in @p in_data.
 * @param out_data Pointer to output array.
 * @param out_size Number of elements in @p out_data (must equal @p in_size).
 */
void ifftshift(const real_t *in_data, size_t in_size, real_t *out_data,
		size_t out_size);

/**
 * @brief Return the number of elements in a frequency axis array.
 *
 * For the Real axis type, returns nfft/2 + 1. For complex axis types
 * (DcCenter, DcLeft), returns nfft.
 *
 * @param nfft      FFT size.
 * @param axis_type Frequency axis type.
 * @return Number of elements in the frequency axis array.
 */
size_t freq_axis_size(size_t nfft, FreqAxisType axis_type);

} // namespace genalyzer_impl

#endif // GENALYZER_IMPL_FOURIER_UTILITIES_HPP