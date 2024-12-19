// Copyright (C) 2024 Analog Devices, Inc.
//
// SPDX short identifier: ADIBSD OR GPL-2.0-or-later
#ifndef GENALYZER_IMPL_WAVEFORMS_HPP
#define GENALYZER_IMPL_WAVEFORMS_HPP

#include "type_aliases.hpp"

#include <map>

namespace genalyzer_impl {

void cos(real_t *data, size_t size, real_t fs, real_t ampl, real_t freq,
		real_t phase, real_t td, real_t tj);

void gaussian(real_t *data, size_t size, real_t mean, real_t sd);

void ramp(real_t *data, size_t size, real_t start, real_t stop, real_t noise);

void sin(real_t *data, size_t size, real_t fs, real_t ampl, real_t freq,
		real_t phase, real_t td, real_t tj);

template <typename T>
std::map<str_t, real_t> wf_analysis(const T *wf_data, size_t wf_size);

const std::vector<str_t> &wf_analysis_ordered_keys();

} // namespace genalyzer_impl

#endif // GENALYZER_IMPL_WAVEFORMS_HPP