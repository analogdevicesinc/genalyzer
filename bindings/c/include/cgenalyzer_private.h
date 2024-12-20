// Copyright (C) 2024 Analog Devices, Inc.
//
// SPDX short identifier: ADIBSD OR GPL-2.0-or-later
#ifndef CGENALYZER_PRIVATE_H
#define CGENALYZER_PRIVATE_H
#include "cgenalyzer_simplified_beta.h"

#include <array_ops.hpp>
#include <code_density.hpp>
#include <constants.hpp>
#include <enum_map.hpp>
#include <enum_maps.hpp>
#include <enums.hpp>
#include <exceptions.hpp>
#include <expression.hpp>
#include <formatted_data.hpp>
#include <fourier_analysis.hpp>
#include <fourier_analysis_comp_mask.hpp>
#include <fourier_analysis_component.hpp>
#include <fourier_analysis_results.hpp>
#include <fourier_transforms.hpp>
#include <fourier_utilities.hpp>
#include <json.hpp>
#include <manager.hpp>
#include <object.hpp>
#include <processes.hpp>
#include <reductions.hpp>
#include <type_aliases.hpp>
#include <utils.hpp>
#include <version.hpp>
#include <waveforms.hpp>

#include <cstring>
#include <regex>
#include <stdexcept>
#include <string>
#include <string_view>

constexpr int gn_success = 0;
constexpr int gn_failure = 1;

namespace gn = ::genalyzer_impl;

namespace util {

class log {
public:
	log() :
			m_log{}, m_flag{ false } {
	}

public:
	bool check() {
		return m_flag;
	}

	void clear() {
		m_log.clear();
		m_flag = false;
	}

	std::string_view get() const {
		return m_log;
	}

	void set(const char *msg) {
		m_log = msg;
		m_flag = true;
	}

	size_t size() const {
		return m_log.size();
	}

public:
	void append() {
	}

	template <typename... Types>
	void append(const char *s, Types... the_rest) {
		m_log.append(s);
		append(the_rest...);
	}

	void prepend() {
	}

	template <typename... Types>
	void prepend(const char *s, Types... the_rest) {
		prepend(the_rest...);
		m_log.insert(0, s);
	}

private:
	std::string m_log;
	bool m_flag;

}; // class log

static log gn_error_log;

template <typename... Types>
int return_on_exception(const char *s, Types... the_rest) {
	gn_error_log.set(s);
	gn_error_log.append(the_rest...);
	return gn_failure;
}

template <typename T>
int check_pointer(const T *p) {
	if (nullptr == p) {
		throw std::runtime_error("check_pointer : pointer is NULL");
	}
	return gn_success;
}

size_t terminated_size(size_t string_size);
void fill_string_buffer(
		const char *src, // Pointer to source
		size_t src_size, // Size of source; should not count null-terminator, if it
		// exists
		char *dst, // Pointer to destination
		size_t dst_size // Size of destination
);
std::string get_object_key_from_filename(const std::string &filename);
} // namespace util

#ifdef __cplusplus
extern "C" {
#endif

using namespace genalyzer_impl;

struct gn_config_private {
	bool _gn_config_calloced = false;

	// waveform and FFT settings
	tone_type ttype;
	gn::size_t npts;
	gn::real_t sample_rate;
	gn::real_t *tone_freq;
	gn::real_t *tone_ampl;
	gn::real_t *tone_phase;
	gn::size_t num_tones;
	gn::real_t fsr;
	int qres;
	gn::real_t noise_rms;
	GnCodeFormat code_format;
	gn::size_t nfft;
	gn::size_t fft_navg;
	GnFreqAxisType axis_type;
	gn::real_t data_rate;
	gn::real_t shift_freq;
	GnWindow win;
	gn::real_t ramp_start;
	gn::real_t ramp_stop;

	// analysis settings
	char *obj_key;
	char *comp_key;
	int ssb_fund;
	int ssb_rest;
	int max_harm_order;
	GnDnlSignal dnla_signal_type;
	GnInlLineFit inla_fit;
	gn::size_t _code_density_size;

	// keys, values and sizes for Fourier analysis results
	char **_fa_result_keys;
	gn::real_t *_fa_result_values;
	gn::size_t *_fa_result_key_sizes;
	gn::size_t _fa_results_size;

	// keys, values and sizes for waveform analysis results
	char **_wfa_result_keys;
	gn::real_t *_wfa_result_values;
	gn::size_t *_wfa_result_key_sizes;
	gn::size_t _wfa_results_size;

	// keys, values and sizes for histogram results
	char **_hist_result_keys;
	gn::real_t *_hist_result_values;
	gn::size_t *_hist_result_key_sizes;
	gn::size_t _hist_results_size;

	// keys, values and sizes for DNL results
	char **_dnl_result_keys;
	gn::real_t *_dnl_result_values;
	gn::size_t *_dnl_result_key_sizes;
	gn::size_t _dnl_results_size;

	// keys, values and sizes for INL results
	char **_inl_result_keys;
	gn::real_t *_inl_result_values;
	gn::size_t *_inl_result_key_sizes;
	gn::size_t _inl_results_size;
};

#ifdef __cplusplus
}
#endif

#endif
