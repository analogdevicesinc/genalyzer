// Copyright (C) 2024 Analog Devices, Inc.
//
// SPDX short identifier: ADIBSD OR GPL-2.0-or-later
#include "fourier_utilities.hpp"

#include "constants.hpp"
#include "exceptions.hpp"
#include "utils.hpp"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <vector>

namespace genalyzer_impl {

real_t alias(real_t fs, real_t freq, FreqAxisType axis_type) {
	assert_gt0("", "fs", fs);
	freq -= std::floor(freq / fs) * fs; // freq in [0, fs)
	if (FreqAxisType::DcCenter == axis_type) {
		return (fs <= 2 * freq) ? (freq - fs) : freq;
	} else if (FreqAxisType::Real == axis_type) {
		return (fs < 2 * freq) ? (fs - freq) : freq;
	} else {
		return freq;
	}
}

real_t coherent(size_t nfft, real_t fs, real_t freq) {
	assert_gt0("", "nfft", nfft);
	assert_gt0("", "fs", fs);
	if (1 == nfft) {
		return freq;
	}
	real_t fbin = fs / static_cast<real_t>(nfft);
	assert_gt0("", "fbin", fbin);
	real_t cycles = std::fabs(freq) / fbin;
	if (is_pow2(nfft)) {
		cycles = std::floor(cycles);
		if (0.0 == std::fmod(cycles, 2.0)) {
			cycles += 1.0;
		}
		cycles = std::copysign(cycles, freq);
	} else {
		cycles = std::copysign(std::round(cycles), freq);
	}
	return cycles * fbin;
}

void fftshift(const real_t *in_data, size_t in_size, real_t *out_data,
		size_t out_size) {
	check_array_pair("", "input array", in_data, in_size, "output_array",
			out_data, out_size);
	size_t offset = in_size / 2;
	if (is_odd(in_size)) {
		offset += 1;
	}
	if (in_data == out_data) {
		std::rotate(out_data, out_data + offset, out_data + out_size);
	} else {
		std::rotate_copy(in_data, in_data + offset, in_data + in_size,
				out_data);
	}
}

void freq_axis(real_t *data, size_t size, size_t nfft, FreqAxisType axis_type,
		real_t fs, FreqAxisFormat axis_format) {
	check_array("", "output array", data, size);
	assert_eq("", "array size", size, "expected",
			freq_axis_size(nfft, axis_type));
	real_t start = 0.0;
	if (FreqAxisType::DcCenter == axis_type) {
		start = -static_cast<real_t>(nfft / 2);
	}
	std::iota(data, data + size, start);
	real_t unit = 1.0;
	if (FreqAxisFormat::Freq == axis_format) {
		unit = fs / static_cast<real_t>(nfft);
	} else if (FreqAxisFormat::Norm == axis_format) {
		unit /= static_cast<real_t>(nfft);
	}
	if (1.0 != unit) {
		for (size_t i = 0; i < size; ++i) {
			data[i] *= unit;
		}
	}
}

void ifftshift(const real_t *in_data, size_t in_size, real_t *out_data,
		size_t out_size) {
	check_array_pair("", "input array", in_data, in_size, "output_array",
			out_data, out_size);
	size_t offset = in_size / 2;
	if (in_data == out_data) {
		std::rotate(out_data, out_data + offset, out_data + out_size);
	} else {
		std::rotate_copy(in_data, in_data + offset, in_data + in_size,
				out_data);
	}
}

size_t freq_axis_size(size_t nfft, FreqAxisType axis_type) {
	assert_gt0("", "nfft", nfft);
	size_t size = (FreqAxisType::Real == axis_type) ? nfft / 2 + 1 : nfft;
	return size;
}

} // namespace genalyzer_impl