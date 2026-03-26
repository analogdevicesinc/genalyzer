// Copyright (C) 2024-2025 Analog Devices, Inc.
//
// SPDX short identifier: ADIBSD OR GPL-2.0-or-later
#ifndef GENALYZER_IMPL_ARRAY_OPS_HPP
#define GENALYZER_IMPL_ARRAY_OPS_HPP

#include "type_aliases.hpp"

namespace genalyzer_impl {

/**
 * @brief Compute the absolute value (magnitude) of each complex number.
 *
 * Input is an interleaved real/imaginary array of size 2N. Output is a real
 * array of size N containing the magnitude of each complex element.
 *
 * @param in_data  Pointer to interleaved complex input data (re, im, re, im, ...).
 * @param in_size  Number of elements in @p in_data (must be even).
 * @param out_data Pointer to output array for magnitudes.
 * @param out_size Number of elements in @p out_data (must be in_size / 2).
 */
void abs(const real_t *in_data, size_t in_size, real_t *out_data,
		size_t out_size);

/**
 * @brief Compute the phase angle (argument) of each complex number.
 *
 * Input is an interleaved real/imaginary array of size 2N. Output is a real
 * array of size N containing the phase angle in radians of each complex element.
 *
 * @param in_data  Pointer to interleaved complex input data.
 * @param in_size  Number of elements in @p in_data (must be even).
 * @param out_data Pointer to output array for phase angles (radians).
 * @param out_size Number of elements in @p out_data (must be in_size / 2).
 */
void angle(const real_t *in_data, size_t in_size, real_t *out_data,
		size_t out_size);

/**
 * @brief Convert each complex number to decibels.
 *
 * Computes 10*log10(re^2 + im^2) for each complex element with a lower bound
 * to avoid -infinity. Input is interleaved real/imaginary, output is real.
 *
 * @param in_data  Pointer to interleaved complex input data.
 * @param in_size  Number of elements in @p in_data (must be even).
 * @param out_data Pointer to output array for dB values.
 * @param out_size Number of elements in @p out_data (must be in_size / 2).
 */
void db(const real_t *in_data, size_t in_size, real_t *out_data,
		size_t out_size);

/**
 * @brief Convert each real value to decibels using 10*log10(x).
 *
 * A lower bound is applied to avoid -infinity for zero or negative values.
 * Input and output are real arrays of the same size.
 *
 * @param in_data  Pointer to input real data.
 * @param in_size  Number of elements in @p in_data.
 * @param out_data Pointer to output array for dB values.
 * @param out_size Number of elements in @p out_data (must equal @p in_size).
 */
void db10(const real_t *in_data, size_t in_size, real_t *out_data,
		size_t out_size);

/**
 * @brief Convert each real value to decibels using 20*log10(x).
 *
 * A lower bound is applied to avoid -infinity for zero or negative values.
 * Input and output are real arrays of the same size.
 *
 * @param in_data  Pointer to input real data.
 * @param in_size  Number of elements in @p in_data.
 * @param out_data Pointer to output array for dB values.
 * @param out_size Number of elements in @p out_data (must equal @p in_size).
 */
void db20(const real_t *in_data, size_t in_size, real_t *out_data,
		size_t out_size);

/**
 * @brief Compute the squared magnitude (norm) of each complex number.
 *
 * Computes re^2 + im^2 for each complex element. Input is interleaved
 * real/imaginary of size 2N, output is real of size N.
 *
 * @param in_data  Pointer to interleaved complex input data.
 * @param in_size  Number of elements in @p in_data (must be even).
 * @param out_data Pointer to output array for squared magnitudes.
 * @param out_size Number of elements in @p out_data (must be in_size / 2).
 */
void norm(const real_t *in_data, size_t in_size, real_t *out_data,
		size_t out_size);

} // namespace genalyzer_impl

#endif // GENALYZER_IMPL_ARRAY_OPS_HPP