// Copyright (C) 2024-2026 Analog Devices, Inc.
//
// SPDX short identifier: ADIBSD OR GPL-2.0-or-later
#ifndef GENALYZER_IMPL_WAVEFORMS_HPP
#define GENALYZER_IMPL_WAVEFORMS_HPP

#include "type_aliases.hpp"

#include <map>

namespace genalyzer_impl {

/**
 * @brief Generate a cosine waveform.
 *
 * Computes data[i] = ampl * cos(2*pi*freq*(i/fs + td) + phase). If @p tj
 * is greater than zero, Gaussian aperture jitter is added to the sampling
 * instants.
 *
 * @param data  Pointer to output array for waveform samples.
 * @param size  Number of samples to generate.
 * @param fs    Sample rate in Hz.
 * @param ampl  Amplitude.
 * @param freq  Signal frequency in Hz.
 * @param phase Phase offset in radians.
 * @param td    Time delay (offset) in seconds.
 * @param tj    RMS aperture jitter in seconds (0 for no jitter).
 */
void cos(real_t *data, size_t size, real_t fs, real_t ampl, real_t freq,
		real_t phase, real_t td, real_t tj);

/**
 * @brief Generate Gaussian (normally distributed) random samples.
 *
 * If @p sd is 0, all samples are set to the mean value.
 *
 * @param data Pointer to output array for random samples.
 * @param size Number of samples to generate.
 * @param mean Mean of the distribution.
 * @param sd   Standard deviation of the distribution (0 for constant output).
 */
void gaussian(real_t *data, size_t size, real_t mean, real_t sd);

/**
 * @brief Generate a linear ramp waveform from start to stop.
 *
 * The ramp uses midpoint sampling within each step. If @p noise is greater
 * than zero, Gaussian noise with the given RMS level is added.
 *
 * @param data  Pointer to output array for waveform samples.
 * @param size  Number of samples to generate.
 * @param start Starting value of the ramp.
 * @param stop  Ending value of the ramp.
 * @param noise RMS level of additive Gaussian noise (0 for no noise).
 */
void ramp(real_t *data, size_t size, real_t start, real_t stop, real_t noise);

/**
 * @brief Generate a sine waveform.
 *
 * Computes data[i] = ampl * sin(2*pi*freq*(i/fs + td) + phase). If @p tj
 * is greater than zero, Gaussian aperture jitter is added to the sampling
 * instants.
 *
 * @param data  Pointer to output array for waveform samples.
 * @param size  Number of samples to generate.
 * @param fs    Sample rate in Hz.
 * @param ampl  Amplitude.
 * @param freq  Signal frequency in Hz.
 * @param phase Phase offset in radians.
 * @param td    Time delay (offset) in seconds.
 * @param tj    RMS aperture jitter in seconds (0 for no jitter).
 */
void sin(real_t *data, size_t size, real_t fs, real_t ampl, real_t freq,
		real_t phase, real_t td, real_t tj);

/**
 * @brief Compute time-domain statistics of a waveform.
 *
 * Results include min, max, mid, range, average, RMS, AC RMS (DC-removed),
 * and indices of min/max values.
 *
 * @tparam T      Sample type (integer or floating-point).
 * @param wf_data Pointer to waveform data.
 * @param wf_size Number of elements in @p wf_data.
 * @return Map of metric names to values.
 */
template <typename T>
std::map<str_t, real_t> wf_analysis(const T *wf_data, size_t wf_size);

/**
 * @brief Return the ordered list of result keys for wf_analysis().
 *
 * @return Reference to a vector of key strings in canonical order.
 */
const std::vector<str_t> &wf_analysis_ordered_keys();

} // namespace genalyzer_impl

#endif // GENALYZER_IMPL_WAVEFORMS_HPP