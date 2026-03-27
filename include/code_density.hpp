// Copyright (C) 2024-2026 Analog Devices, Inc.
//
// SPDX short identifier: ADIBSD OR GPL-2.0-or-later
#ifndef GENALYZER_IMPL_CODE_DENSITY_HPP
#define GENALYZER_IMPL_CODE_DENSITY_HPP

#include "enums.hpp"
#include "type_aliases.hpp"

#include <map>

namespace genalyzer_impl {

/**
 * @brief Return the number of histogram bins for a given ADC resolution and code format.
 *
 * @param n      ADC resolution in bits.
 * @param format Code format (e.g., TwosComplement, OffsetBinary).
 * @return Number of codes (2^n).
 */
size_t code_density_size(int n, CodeFormat format);

/**
 * @brief Return the number of histogram bins for an explicit code range.
 *
 * @param min Minimum code value (inclusive).
 * @param max Maximum code value (inclusive).
 * @return Number of codes (max - min + 1).
 */
size_t code_densityx_size(int64_t min, int64_t max);

/**
 * @brief Generate a code axis array for an ADC with the given resolution.
 *
 * Values span the full code range determined by the code format (e.g.,
 * -2^(n-1) to 2^(n-1)-1 for two's complement).
 *
 * @param data   Pointer to output array for code axis values.
 * @param size   Number of elements in @p data (use code_density_size()).
 * @param n      ADC resolution in bits.
 * @param format Code format.
 */
void code_axis(real_t *data, size_t size, int n, CodeFormat format);

/**
 * @brief Generate a code axis array for an explicit code range [min, max].
 *
 * Output values are integers from @p min to @p max inclusive.
 *
 * @param data Pointer to output array for code axis values.
 * @param size Number of elements in @p data (use code_densityx_size()).
 * @param min  Minimum code value (inclusive).
 * @param max  Maximum code value (inclusive).
 */
void code_axisx(real_t *data, size_t size, int64_t min, int64_t max);

/**
 * @brief Compute Differential Nonlinearity (DNL) from a histogram.
 *
 * DNL measures the deviation of each code bin width from the ideal 1-LSB
 * step size. The computation method depends on the signal type (ramp or
 * sinusoidal tone).
 *
 * @param dnl_data  Pointer to output DNL array.
 * @param dnl_size  Number of elements in @p dnl_data.
 * @param hist_data Pointer to input histogram data.
 * @param hist_size Number of elements in @p hist_data.
 * @param type      Signal type used to acquire the histogram (Ramp or Tone).
 */
void dnl(real_t *dnl_data, size_t dnl_size, const uint64_t *hist_data,
		size_t hist_size, DnlSignal type);

/**
 * @brief Compute summary statistics from DNL data.
 *
 * Results include min, max, average, RMS, and indices of extrema and
 * non-missing codes.
 *
 * @param data Pointer to DNL data array.
 * @param size Number of elements in @p data.
 * @return Map of metric names to values.
 */
std::map<str_t, real_t> dnl_analysis(const real_t *data, size_t size);

/**
 * @brief Return the ordered list of result keys for dnl_analysis().
 *
 * @return Reference to a vector of key strings in canonical order.
 */
const std::vector<str_t> &dnl_analysis_ordered_keys();

/**
 * @brief Compute a histogram (code density) of a quantized waveform.
 *
 * Counts the number of occurrences of each code for the given ADC resolution
 * and code format.
 *
 * @tparam T        Integer sample type.
 * @param hist_data Pointer to histogram output array.
 * @param hist_size Number of elements in @p hist_data (use code_density_size()).
 * @param wf_data   Pointer to input waveform samples.
 * @param wf_size   Number of elements in @p wf_data.
 * @param n         ADC resolution in bits.
 * @param format    Code format.
 * @param preserve  If true, add counts to existing histogram data; if false,
 *                  initialize the histogram to zero first.
 */
template <typename T>
void hist(uint64_t *hist_data, size_t hist_size, const T *wf_data,
		size_t wf_size, int n, CodeFormat format, bool preserve);

/**
 * @brief Compute a histogram of a quantized waveform using explicit code bounds.
 *
 * Uses explicit min/max code values instead of resolution and code format.
 *
 * @tparam T        Integer sample type.
 * @param hist_data Pointer to histogram output array.
 * @param hist_size Number of elements in @p hist_data (use code_densityx_size()).
 * @param wf_data   Pointer to input waveform samples.
 * @param wf_size   Number of elements in @p wf_data.
 * @param min       Minimum code value (inclusive).
 * @param max       Maximum code value (inclusive).
 * @param preserve  If true, add counts to existing histogram data; if false,
 *                  initialize the histogram to zero first.
 */
template <typename T>
void histx(uint64_t *hist_data, size_t hist_size, const T *wf_data,
		size_t wf_size, int64_t min, int64_t max, bool preserve);

/**
 * @brief Compute summary statistics from histogram data.
 *
 * Results include sum, first/last non-zero bin indices, and non-zero range.
 *
 * @param data Pointer to histogram data array.
 * @param size Number of elements in @p data.
 * @return Map of metric names to values.
 */
std::map<str_t, real_t> hist_analysis(const uint64_t *data, size_t size);

/**
 * @brief Return the ordered list of result keys for hist_analysis().
 *
 * @return Reference to a vector of key strings in canonical order.
 */
const std::vector<str_t> &hist_analysis_ordered_keys();

/**
 * @brief Compute Integral Nonlinearity (INL) by cumulative summation of DNL.
 *
 * INL measures the deviation of the transfer function from an ideal straight
 * line. The line-fit parameter controls whether/how a best-fit or endpoint
 * line is removed from the result.
 *
 * @param inl_data Pointer to output INL array.
 * @param inl_size Number of elements in @p inl_data.
 * @param dnl_data Pointer to input DNL data.
 * @param dnl_size Number of elements in @p dnl_data.
 * @param fit      Line-fit method (BestFit, EndFit, or NoFit).
 */
void inl(real_t *inl_data, size_t inl_size, const real_t *dnl_data,
		size_t dnl_size, InlLineFit fit);

/**
 * @brief Compute summary statistics from INL data.
 *
 * Results include min, max, and their indices.
 *
 * @param data Pointer to INL data array.
 * @param size Number of elements in @p data.
 * @return Map of metric names to values.
 */
std::map<str_t, real_t> inl_analysis(const real_t *data, size_t size);

/**
 * @brief Return the ordered list of result keys for inl_analysis().
 *
 * @return Reference to a vector of key strings in canonical order.
 */
const std::vector<str_t> &inl_analysis_ordered_keys();

} // namespace genalyzer_impl

#endif // GENALYZER_IMPL_CODE_DENSITY_HPP