// Copyright (C) 2024 Analog Devices, Inc.
//
// SPDX short identifier: ADIBSD OR GPL-2.0-or-later
#include "waveforms.hpp"

#include "constants.hpp"
#include "reductions.hpp"
#include "utils.hpp"

#include <cmath>
#include <functional>
#include <random>

namespace genalyzer_impl {

namespace {

void sinusoid(double (*func)(double), real_t *data, size_t size, real_t fs,
		real_t ampl, real_t freq, real_t phase, real_t td, real_t tj) {
	check_array("", "output array", data, size);
	assert_gt0("", "fs", fs);
	const real_t twopif = k_2pi * freq;
	const real_t twopifts = twopif / fs;
	const real_t twopiftd_plus_phase = std::fma(twopif, td, phase);
	real_t theta = twopiftd_plus_phase;
	if (0.0 == tj) {
		for (size_t i = 0; i < size; ++i) {
			data[i] = ampl * func(theta);
			theta += twopifts;
		}
	} else {
		std::random_device rdev;
		std::mt19937 rgen(rdev());
		auto ngen = std::bind(
				std::normal_distribution<real_t>(0.0, twopif * tj),
				rgen);
		for (size_t i = 0; i < size; ++i) {
			data[i] = theta + ngen();
			theta += twopifts;
		} // breaking into two loops gives modest speed improvement
		for (size_t i = 0; i < size; ++i) {
			data[i] = ampl * func(data[i]);
		}
	}
}

} // namespace

void cos(real_t *data, size_t size, real_t fs, real_t ampl, real_t freq,
		real_t phase, real_t td, real_t tj) {
	sinusoid(std::cos, data, size, fs, ampl, freq, phase, td, tj);
}

void gaussian(real_t *data, size_t size, real_t mean, real_t sd) {
	check_array("", "output array", data, size);
	if (0.0 == sd) {
		for (size_t i = 0; i < size; ++i) {
			data[i] = mean;
		}
	} else {
		std::random_device rdev;
		std::mt19937 rgen(rdev());
		auto ngen = std::bind(
				std::normal_distribution<real_t>(mean, std::fabs(sd)),
				rgen);
		for (size_t i = 0; i < size; ++i) {
			data[i] = ngen();
		}
	}
}

void ramp(real_t *data, size_t size, real_t start, real_t stop, real_t noise) {
	check_array("", "output array", data, size);
	const real_t step = (stop - start) / static_cast<real_t>(size);
	real_t x = start + step / 2;
	if (0.0 == noise) {
		for (size_t i = 0; i < size; ++i) {
			data[i] = x;
			x += step;
		}
	} else {
		std::random_device rdev;
		std::mt19937 rgen(rdev());
		auto ngen = std::bind(
				std::normal_distribution<real_t>(0.0, std::fabs(noise)),
				rgen);
		for (size_t i = 0; i < size; ++i) {
			data[i] = x + ngen();
			x += step;
		}
	}
}

void sin(real_t *data, size_t size, real_t fs, real_t ampl, real_t freq,
		real_t phase, real_t td, real_t tj) {
	sinusoid(std::sin, data, size, fs, ampl, freq, phase, td, tj);
}

template <typename T>
std::map<str_t, real_t> wf_analysis(const T *wf_data, size_t wf_size) {
	check_array("", "waveform array", wf_data, wf_size);
	std_reduce_t r = std_reduce(wf_data, wf_size, 0, wf_size);
	real_t n = static_cast<real_t>(wf_size);
	real_t avg = r.sum / n;
	real_t rms = std::sqrt(r.sumsq / n);
	real_t rmsac = std::sqrt(rms * rms - avg * avg);
	std::vector<str_t> keys = wf_analysis_ordered_keys();
	return std::map<str_t, real_t>{
		{ keys[0], r.min },
		{ keys[1], r.max },
		{ keys[2], (r.max + r.min) / 2 },
		{ keys[3], r.max - r.min },
		{ keys[4], avg },
		{ keys[5], rms },
		{ keys[6], rmsac },
		{ keys[7], static_cast<real_t>(r.min_index) },
		{ keys[8], static_cast<real_t>(r.max_index) }
	};
}

template std::map<str_t, real_t> wf_analysis(const int16_t *, size_t);
template std::map<str_t, real_t> wf_analysis(const int32_t *, size_t);
template std::map<str_t, real_t> wf_analysis(const int64_t *, size_t);
template std::map<str_t, real_t> wf_analysis(const real_t *, size_t);

const std::vector<str_t> &wf_analysis_ordered_keys() {
	static const std::vector<str_t> keys{
		"min", "max", "mid", "range", "avg",
		"rms", "rmsac", "min_index", "max_index"
	};
	return keys;
}

} // namespace genalyzer_impl
