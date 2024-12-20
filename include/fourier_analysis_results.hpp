// Copyright (C) 2024 Analog Devices, Inc.
//
// SPDX short identifier: ADIBSD OR GPL-2.0-or-later
#ifndef GENALYZER_IMPL_FOURIER_ANALYSIS_RESULTS_HPP
#define GENALYZER_IMPL_FOURIER_ANALYSIS_RESULTS_HPP

#include "enums.hpp"
#include "exceptions.hpp"
#include "utils.hpp"

#include <map>

namespace genalyzer_impl {

struct fa_tone_results {
	real_t get(FAToneResult key) const {
		if (results.find(key) == results.end()) {
			throw runtime_error(
					"fa_tone_results::get : key not found");
		}
		return results.at(key);
	}

	void set(FAToneResult key, real_t value) {
		results[key] = value;
		if (FAToneResult::I1 == key) {
			i1 = static_cast<size_t>(value);
		} else if (FAToneResult::I2 == key) {
			i2 = static_cast<size_t>(value);
		} else if (FAToneResult::NBins == key) {
			nbins = static_cast<size_t>(value);
		} else if (FAToneResult::InBand == key) {
			inband = static_cast<bool>(value);
		}
	}

	void set_mag(real_t ms_value) {
		results[FAToneResult::Mag] = std::sqrt(ms_value);
		results[FAToneResult::Mag_dBFS] = bounded_db10(ms_value);
	}

	std::map<FAToneResult, real_t> results;
	size_t i1;
	size_t i2;
	size_t nbins;
	bool inband;
};

struct fourier_analysis_results {
	real_t get(FAResult key) const {
		if (results.find(key) == results.end()) {
			throw runtime_error(
					"fourier_analysis_results::get : key not found");
		}
		return results.at(key);
	}

	void set(FAResult key, real_t value) {
		results[key] = value;
	}

	bool contains_tone(const str_t &key) const {
		return tone_results.find(key) != tone_results.end();
	}

	void add_tone(const str_t &key, fa_tone_results tr) {
		if (!contains_tone(key)) {
			tone_keys.push_back(key);
			tone_results.emplace(key, std::move(tr));
		}
	}

	const fa_tone_results &get_tone(const str_t &key) const {
		if (contains_tone(key)) {
			return tone_results.at(key);
		} else {
			throw runtime_error(
					"fourier_analysis_results::get_tone : key not found");
		}
	}

	std::map<FAResult, real_t> results;
	str_vector tone_keys;
	std::map<str_t, fa_tone_results> tone_results;
};

} // namespace genalyzer_impl

#endif // GENALYZER_IMPL_FOURIER_ANALYSIS_RESULTS_HPP