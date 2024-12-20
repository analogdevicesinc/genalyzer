// Copyright (C) 2024 Analog Devices, Inc.
//
// SPDX short identifier: ADIBSD OR GPL-2.0-or-later
#include "cgenalyzer.h"
#include "cgenalyzer_private.h"

using namespace util;

namespace util {

static bool gn_null_terminate = true;

size_t terminated_size(size_t string_size) {
	return string_size + (util::gn_null_terminate ? 1 : 0);
}

void fill_string_buffer(
		const char *src, // Pointer to source
		size_t src_size, // Size of source; should not count null-terminator, if it
		// exists
		char *dst, // Pointer to destination
		size_t dst_size // Size of destination
) {
	if (nullptr == src) {
		throw std::runtime_error("fill_string_buffer : source is NULL");
	}
	if (nullptr == dst) {
		throw std::runtime_error(
				"fill_string_buffer : destination is NULL");
	}
	if (dst_size < terminated_size(src_size)) {
		throw std::runtime_error(
				"fill_string_buffer : destination too small");
	}
	for (size_t i = 0; i < src_size; ++i) {
		dst[i] = src[i];
	}
	if (gn_null_terminate) {
		dst[src_size] = '\0';
	}
}

std::string get_object_key_from_filename(const std::string &filename) {
	static const std::regex re{
		"(" + gn::manager::key_pattern + ")[.]json$", std::regex::icase
	};
	std::smatch matches;
	if (std::regex_search(filename, matches, re)) {
		if (1 == matches.size()) {
			throw std::runtime_error(
					"unable to derive object key from filename '" +
					filename + "'");
		}
		return matches[1].str();
	} else {
		throw std::runtime_error("invalid filename '" + filename + "'");
	}
}

} // namespace util

/**************************************************************************/
/* API Utilities                                                          */
/**************************************************************************/

int gn_analysis_results_key_sizes(size_t *key_sizes, size_t key_sizes_size,
		GnAnalysisType type) {
	try {
		util::check_pointer(key_sizes);
		std::vector<std::string> keys;
		switch (gn::get_enum<gn::AnalysisType>(type)) {
			case gn::AnalysisType::DNL:
				keys = gn::dnl_analysis_ordered_keys();
				break;
			case gn::AnalysisType::Histogram:
				keys = gn::hist_analysis_ordered_keys();
				break;
			case gn::AnalysisType::INL:
				keys = gn::inl_analysis_ordered_keys();
				break;
			case gn::AnalysisType::Waveform:
				keys = gn::wf_analysis_ordered_keys();
				break;
			default:
				throw std::runtime_error("Invalid analysis type");
		}
		if (keys.size() != key_sizes_size) {
			throw std::runtime_error(
					"Number of keys does not match output array size");
		}
		for (size_t i = 0; i < key_sizes_size; ++i) {
			key_sizes[i] = util::terminated_size(keys[i].size());
		}
		return gn_success;
	} catch (const std::exception &e) {
		std::fill(key_sizes, key_sizes + key_sizes_size, 0);
		return util::return_on_exception(
				"gn_analysis_results_key_sizes : ", e.what());
	}
}

int gn_analysis_results_size(size_t *size, GnAnalysisType type) {
	try {
		util::check_pointer(size);
		switch (gn::get_enum<gn::AnalysisType>(type)) {
			case gn::AnalysisType::DNL:
				*size = gn::dnl_analysis_ordered_keys().size();
				break;
			case gn::AnalysisType::Histogram:
				*size = gn::hist_analysis_ordered_keys().size();
				break;
			case gn::AnalysisType::INL:
				*size = gn::inl_analysis_ordered_keys().size();
				break;
			case gn::AnalysisType::Waveform:
				*size = gn::wf_analysis_ordered_keys().size();
				break;
			default:
				throw std::runtime_error("Invalid analysis type");
		}
		return gn_success;
	} catch (const std::exception &e) {
		*size = 0;
		return util::return_on_exception("gn_analysis_results_size : ",
				e.what());
	}
}

int gn_enum_value(int *value, const char *enumeration, const char *enumerator) {
	try {
		util::check_pointer(value);
		*value = gn::enum_value(enumeration, enumerator);
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_enum_value : ", e.what());
	}
}

int gn_error_check(bool *error) {
	try {
		util::check_pointer(error);
		*error = util::gn_error_log.check();
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_error_check : ", e.what());
	}
}

int gn_error_clear() {
	util::gn_error_log.clear();
	return gn_success;
}

int gn_error_string(char *buf, size_t size) {
	try {
		std::string_view s = util::gn_error_log.get();
		util::fill_string_buffer(s.data(), s.size(), buf, size);
	} catch (const std::exception &) {
		return gn_failure;
	}
	return gn_success;
}

int gn_set_string_termination(bool null_terminated) {
	util::gn_null_terminate = null_terminated;
	return gn_success;
}

int gn_version_string(char *buf, size_t size) {
	try {
		std::string_view s = gn::version_string();
		util::fill_string_buffer(s.data(), s.size(), buf, size);
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_version_string : ",
				e.what());
	}
	return gn_success;
}

/**************************************************************************/
/* API Utility Helpers                                                    */
/**************************************************************************/

int gn_error_string_size(size_t *size) {
	try {
		util::check_pointer(size);
		*size = util::terminated_size(util::gn_error_log.size());
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_error_string_size : ",
				e.what());
	}
}

int gn_version_string_size(size_t *size) {
	try {
		util::check_pointer(size);
		*size = util::terminated_size(gn::version_string().size());
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_version_string_size : ",
				e.what());
	}
}

/**************************************************************************/
/* Array Operations                                                       */
/**************************************************************************/

int gn_abs(double *out, size_t out_size, const double *in, size_t in_size) {
	try {
		gn::abs(in, in_size, out, out_size);
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_abs : ", e.what());
	}
}

int gn_angle(double *out, size_t out_size, const double *in, size_t in_size) {
	try {
		gn::angle(in, in_size, out, out_size);
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_angle : ", e.what());
	}
}

int gn_db(double *out, size_t out_size, const double *in, size_t in_size) {
	try {
		gn::db(in, in_size, out, out_size);
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_db : ", e.what());
	}
}

int gn_db10(double *out, size_t out_size, const double *in, size_t in_size) {
	try {
		gn::db10(in, in_size, out, out_size);
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_db10 : ", e.what());
	}
}

int gn_db20(double *out, size_t out_size, const double *in, size_t in_size) {
	try {
		gn::db20(in, in_size, out, out_size);
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_db20 : ", e.what());
	}
}

int gn_norm(double *out, size_t out_size, const double *in, size_t in_size) {
	try {
		gn::norm(in, in_size, out, out_size);
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_norm : ", e.what());
	}
}

/**************************************************************************/
/* Code Density                                                           */
/**************************************************************************/

namespace {

template <typename T>
int gn_hist(const char *suffix, uint64_t *hist, size_t hist_size, const T *in,
		size_t in_size, int n, GnCodeFormat format, bool preserve) {
	try {
		gn::CodeFormat f = gn::get_enum<gn::CodeFormat>(format);
		gn::hist(hist, hist_size, in, in_size, n, f, preserve);
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_hist", suffix, " : ",
				e.what());
	}
}

template <typename T>
int gn_histx(const char *suffix, uint64_t *hist, size_t hist_size, const T *in,
		size_t in_size, int64_t min, int64_t max, bool preserve) {
	try {
		gn::histx(hist, hist_size, in, in_size, min, max, preserve);
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_histx", suffix, " : ",
				e.what());
	}
}

} // namespace

int gn_code_axis(double *out, size_t size, int n, GnCodeFormat format) {
	try {
		gn::CodeFormat f = gn::get_enum<gn::CodeFormat>(format);
		gn::code_axis(out, size, n, f);
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_code_axis : ", e.what());
	}
}

int gn_code_axisx(double *out, size_t size, int64_t min, int64_t max) {
	try {
		gn::code_axisx(out, size, min, max);
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_code_axisx : ", e.what());
	}
}

int gn_dnl(double *dnl, size_t dnl_size, const uint64_t *hist, size_t hist_size,
		GnDnlSignal type) {
	try {
		gn::DnlSignal t = gn::get_enum<gn::DnlSignal>(type);
		gn::dnl(dnl, dnl_size, hist, hist_size, t);
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_dnl : ", e.what());
	}
}

int gn_dnl_analysis(char **rkeys, size_t rkeys_size, double *rvalues,
		size_t rvalues_size, const double *dnl, size_t dnl_size) {
	try {
		util::check_pointer(rkeys);
		util::check_pointer(rvalues);
		const std::vector<std::string> &keys =
				gn::dnl_analysis_ordered_keys();
		if (keys.size() != rkeys_size) {
			throw std::runtime_error(
					"Size of result key array is wrong");
		}
		if (rvalues_size != rkeys_size) {
			throw std::runtime_error(
					"Size of result keys does not match size of result values");
		}
		std::map<std::string, double> results =
				gn::dnl_analysis(dnl, dnl_size);
		for (size_t i = 0; i < keys.size(); ++i) {
			const std::string &src = keys[i];
			char *dst = rkeys[i];
			size_t dst_size = util::terminated_size(src.size());
			util::fill_string_buffer(src.data(), src.size(), dst,
					dst_size);
			rvalues[i] = results.at(src);
		}
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_dnl_analysis : ",
				e.what());
	}
}

int gn_hist16(uint64_t *hist, size_t hist_size, const int16_t *in,
		size_t in_size, int n, GnCodeFormat format, bool preserve) {
	return gn_hist("16", hist, hist_size, in, in_size, n, format, preserve);
}

int gn_hist32(uint64_t *hist, size_t hist_size, const int32_t *in,
		size_t in_size, int n, GnCodeFormat format, bool preserve) {
	return gn_hist("32", hist, hist_size, in, in_size, n, format, preserve);
}

int gn_hist64(uint64_t *hist, size_t hist_size, const int64_t *in,
		size_t in_size, int n, GnCodeFormat format, bool preserve) {
	return gn_hist("64", hist, hist_size, in, in_size, n, format, preserve);
}

int gn_histx16(uint64_t *hist, size_t hist_size, const int16_t *in,
		size_t in_size, int64_t min, int64_t max, bool preserve) {
	return gn_histx("16", hist, hist_size, in, in_size, min, max, preserve);
}

int gn_histx32(uint64_t *hist, size_t hist_size, const int32_t *in,
		size_t in_size, int64_t min, int64_t max, bool preserve) {
	return gn_histx("32", hist, hist_size, in, in_size, min, max, preserve);
}

int gn_histx64(uint64_t *hist, size_t hist_size, const int64_t *in,
		size_t in_size, int64_t min, int64_t max, bool preserve) {
	return gn_histx("64", hist, hist_size, in, in_size, min, max, preserve);
}

int gn_hist_analysis(char **rkeys, size_t rkeys_size, double *rvalues,
		size_t rvalues_size, const uint64_t *hist,
		size_t hist_size) {
	try {
		util::check_pointer(rkeys);
		util::check_pointer(rvalues);
		const std::vector<std::string> &keys =
				gn::hist_analysis_ordered_keys();
		if (keys.size() != rkeys_size) {
			throw std::runtime_error(
					"Size of result key array is wrong");
		}
		if (rvalues_size != rkeys_size) {
			throw std::runtime_error(
					"Size of result keys does not match size of result values");
		}
		std::map<std::string, double> results =
				gn::hist_analysis(hist, hist_size);
		for (size_t i = 0; i < keys.size(); ++i) {
			const std::string &src = keys[i];
			char *dst = rkeys[i];
			size_t dst_size = util::terminated_size(src.size());
			util::fill_string_buffer(src.data(), src.size(), dst,
					dst_size);
			rvalues[i] = results.at(src);
		}
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_hist_analysis : ",
				e.what());
	}
}

int gn_inl(double *inl, size_t inl_size, const double *dnl, size_t dnl_size,
		GnInlLineFit fit) {
	try {
		gn::InlLineFit f = gn::get_enum<gn::InlLineFit>(fit);
		gn::inl(inl, inl_size, dnl, dnl_size, f);
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_inl : ", e.what());
	}
}

int gn_inl_analysis(char **rkeys, size_t rkeys_size, double *rvalues,
		size_t rvalues_size, const double *inl, size_t inl_size) {
	try {
		util::check_pointer(rkeys);
		util::check_pointer(rvalues);
		const std::vector<std::string> &keys =
				gn::inl_analysis_ordered_keys();
		if (keys.size() != rkeys_size) {
			throw std::runtime_error(
					"Size of result key array is wrong");
		}
		if (rvalues_size != rkeys_size) {
			throw std::runtime_error(
					"Size of result keys does not match size of result values");
		}
		std::map<std::string, double> results =
				gn::inl_analysis(inl, inl_size);
		for (size_t i = 0; i < keys.size(); ++i) {
			const std::string &src = keys[i];
			char *dst = rkeys[i];
			size_t dst_size = util::terminated_size(src.size());
			util::fill_string_buffer(src.data(), src.size(), dst,
					dst_size);
			rvalues[i] = results.at(src);
		}
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_inl_analysis : ",
				e.what());
	}
}

/**************************************************************************/
/* Code Density Helpers                                                   */
/**************************************************************************/

int gn_code_density_size(size_t *size, int n, GnCodeFormat format) {
	try {
		util::check_pointer(size);
		gn::CodeFormat f = gn::get_enum<gn::CodeFormat>(format);
		*size = gn::code_density_size(n, f);
		return gn_success;
	} catch (const std::exception &e) {
		*size = 0;
		return util::return_on_exception("gn_code_density_size : ",
				e.what());
	}
}

int gn_code_densityx_size(size_t *size, int64_t min, int64_t max) {
	try {
		util::check_pointer(size);
		*size = gn::code_densityx_size(min, max);
		return gn_success;
	} catch (const std::exception &e) {
		*size = 0;
		return util::return_on_exception("gn_code_densityx_size : ",
				e.what());
	}
}

/**************************************************************************/
/* Fourier Analysis                                                       */
/**************************************************************************/

namespace {

using fa_ptr = std::shared_ptr<gn::fourier_analysis>;

fa_ptr get_fa_object(const std::string &obj_key) {
	gn::object::pointer pobj = gn::manager::get_object(obj_key);
	const gn::ObjectType obj_type = gn::ObjectType::FourierAnalysis;
	if (obj_type != pobj->object_type()) {
		throw std::runtime_error(
				"object '" + obj_key + "' is not of type " +
				gn::object_type_map.at(static_cast<int>(obj_type)));
	}
	return std::static_pointer_cast<gn::fourier_analysis>(pobj);
}

fa_ptr get_fa_object_or_load_from_file(std::string cfg_id) {
	if (gn::manager::contains(cfg_id)) {
		return get_fa_object(cfg_id);
	} else {
		return gn::fourier_analysis::load(cfg_id);
	}
}

size_t get_fa_result_key_index(const char **rkeys, size_t rkeys_size,
		const char *rkey) {
	size_t i = 0;
	for (; i < rkeys_size; ++i) {
		if (0 == strcmp(rkeys[i], rkey)) {
			break;
		}
	}
	if (rkeys_size == i) {
		throw std::runtime_error("Result key '" + std::string(rkey) +
				"' not found");
	}
	return i;
}

std::string get_fa_result_string(const char **rkeys, size_t rkeys_size,
		const double *rvalues, size_t rvalues_size,
		const char *rkey) {
	if (rkeys_size != rvalues_size) {
		throw std::runtime_error(
				"Size of result keys does not match size of result values");
	}
	if (gn::fa_result_map.contains(rkey, true)) {
		gn::FAResult renum =
				static_cast<gn::FAResult>(gn::fa_result_map.at(rkey));
		if (gn::FAResult::CarrierIndex == renum ||
				gn::FAResult::MaxSpurIndex == renum) {
			// Caller requests the Carrier or MaxSpur tone key.
			size_t key_index = get_fa_result_key_index(
					rkeys, rkeys_size, rkey);
			size_t order_index =
					static_cast<size_t>(rvalues[key_index]);
			int order_index_int =
					static_cast<int>(gn::FAToneResult::OrderIndex);
			std::string search_str =
					":" +
					gn::fa_tone_result_map.at(order_index_int);
			const char *search_cstr = search_str.c_str();
			size_t i = 0;
			for (; i < rvalues_size; ++i) {
				if (strstr(rkeys[i], search_cstr)) {
					if (rvalues[i] == order_index) {
						break;
					}
				}
			}
			if (rvalues_size == i) {
				return "Not Found";
			} else {
				return gn::fourier_analysis::split_key(rkeys[i])
						.first;
			}
		}
	} else {
		// In the future, there could be a string associated with a tone result.
	}
	throw std::runtime_error("no string associated with result key '" +
			std::string(rkey) + "'");
}

int get_fa_single_result(const gn::fourier_analysis_results &results,
		const char *rkey, double *rvalue) {
	*rvalue = 0.0;
	std::pair<std::string, std::string> keys =
			gn::fourier_analysis::split_key(rkey);
	if (!keys.first.empty()) {
		if (keys.second.empty()) {
			if (gn::fa_result_map.contains(keys.first)) {
				int i = gn::fa_result_map.at(keys.first);
				*rvalue = results.get(
						static_cast<gn::FAResult>(i));
				return gn_success;
			}
		} else {
			if (results.contains_tone(keys.first)) {
				const gn::fa_tone_results &tres =
						results.get_tone(keys.first);
				if (gn::fa_tone_result_map.at(keys.second)) {
					int i = gn::fa_tone_result_map.at(
							keys.second);
					*rvalue = tres.get(
							static_cast<gn::FAToneResult>(
									i));
					return gn_success;
				}
			}
		}
	}
	return gn_failure;
}

} // namespace

int gn_fft_analysis(char **rkeys, size_t rkeys_size, double *rvalues,
		size_t rvalues_size, const char *cfg_id, const double *in,
		size_t in_size, size_t nfft, GnFreqAxisType axis_type) {
	try {
		if (rkeys_size != rvalues_size) {
			throw std::runtime_error(
					"Size of result keys does not match size of result values");
		}
		fa_ptr obj = get_fa_object_or_load_from_file(cfg_id);
		gn::FreqAxisType at = gn::get_enum<gn::FreqAxisType>(axis_type);
		gn::fourier_analysis_results results =
				obj->analyze(in, in_size, nfft, at);
		// The rest of this function flattens results into a key-array and
		// value-array pair
		size_t i = 0; // index for rkeys, rvalues
		const std::map<gn::FAResult, double> &rmap = results.results;
		for (int j = 0; j < static_cast<int>(gn::FAResult::__SIZE__);
				++j) {
			const std::string &src = gn::fa_result_map.at(j);
			char *dst = rkeys[i];
			size_t dst_size = util::terminated_size(src.size());
			util::fill_string_buffer(src.data(), src.size(), dst,
					dst_size);
			rvalues[i] = rmap.at(static_cast<gn::FAResult>(j));
			i += 1;
		}
		for (const std::string &tkey : results.tone_keys) {
			const gn::fa_tone_results &tone_results =
					results.get_tone(tkey);
			const std::map<gn::FAToneResult, double> &trmap =
					tone_results.results;
			for (int j = 0;
					j < static_cast<int>(gn::FAToneResult::__SIZE__);
					++j) {
				std::string src =
						gn::fourier_analysis::flat_tone_key(
								tkey, j);
				char *dst = rkeys[i];
				size_t dst_size =
						util::terminated_size(src.size());
				util::fill_string_buffer(src.data(), src.size(),
						dst, dst_size);
				rvalues[i] = trmap.at(
						static_cast<gn::FAToneResult>(j));
				i += 1;
			}
		}
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_fa_execute : ", e.what());
	}
}

int gn_fft_analysis_select(double *rvalues, size_t rvalues_size,
		const char *cfg_id, const char **rkeys,
		size_t rkeys_size, const double *in, size_t in_size,
		size_t nfft, GnFreqAxisType axis_type) {
	try {
		if (rkeys_size != rvalues_size) {
			throw std::runtime_error(
					"Size of result keys does not match size of result values");
		}
		fa_ptr obj = get_fa_object_or_load_from_file(cfg_id);
		gn::FreqAxisType at = gn::get_enum<gn::FreqAxisType>(axis_type);
		gn::fourier_analysis_results results =
				obj->analyze(in, in_size, nfft, at);
		std::string missing_keys{};
		for (size_t i = 0; i < rkeys_size; ++i) {
			int error = get_fa_single_result(results, rkeys[i],
					&rvalues[i]);
			if (error) {
				if (!missing_keys.empty()) {
					missing_keys += ", ";
				}
				missing_keys.append(
						"'" + std::string(rkeys[i]) + "'");
			}
		}
		if (!missing_keys.empty()) {
			throw std::runtime_error("Keys not found: " +
					missing_keys);
		}
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_fa_execute2 : ", e.what());
	}
}

int gn_fft_analysis_single(double *rvalue, const char *cfg_id, const char *rkey,
		const double *in, size_t in_size, size_t nfft,
		GnFreqAxisType axis_type) {
	try {
		fa_ptr obj = get_fa_object_or_load_from_file(cfg_id);
		gn::FreqAxisType at = gn::get_enum<gn::FreqAxisType>(axis_type);
		gn::fourier_analysis_results results =
				obj->analyze(in, in_size, nfft, at);
		int error = get_fa_single_result(results, rkey, rvalue);
		if (error) {
			throw std::runtime_error("Key '" + std::string(rkey) +
					"' not found");
		}
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_fa_execute1 : ", e.what());
	}
}

/**************************************************************************/
/* Fourier Analysis Configuration                                         */
/**************************************************************************/

int gn_fa_analysis_band(const char *obj_key, double center, double width) {
	try {
		fa_ptr obj = get_fa_object(obj_key);
		std::string center_s = gn::to_string(center, gn::FPFormat::Eng);
		std::string width_s = gn::to_string(width, gn::FPFormat::Eng);
		obj->set_analysis_band(center_s, width_s);
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_fa_analysis_band : ",
				e.what());
	}
}

int gn_fa_analysis_band_e(const char *obj_key, const char *center,
		const char *width) {
	try {
		fa_ptr obj = get_fa_object(obj_key);
		obj->set_analysis_band(center, width);
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_fa_analysis_band_e : ",
				e.what());
	}
}

int gn_fa_clk(const char *obj_key, const int *clk, size_t clk_size,
		bool as_noise) {
	try {
		fa_ptr obj = get_fa_object(obj_key);
		std::set<int> clk2(clk, clk + clk_size);
		obj->set_clk(clk2);
		obj->clk_as_noise = as_noise;
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_fa_clk : ", e.what());
	}
}

int gn_fa_conv_offset(const char *obj_key, bool enable) {
	try {
		fa_ptr obj = get_fa_object(obj_key);
		obj->en_conv_offset = enable;
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_fa_conv_offset : ",
				e.what());
	}
}

int gn_fa_create(const char *obj_key) {
	try {
		gn::manager::add_object(obj_key, gn::fourier_analysis::create(),
				false);
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_fa_create : ", e.what());
	}
}

int gn_fa_dc(const char *obj_key, bool as_dist) {
	try {
		fa_ptr obj = get_fa_object(obj_key);
		obj->dc_as_dist = as_dist;
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_fa_dc : ", e.what());
	}
}

int gn_fa_fdata(const char *obj_key, double f) {
	try {
		fa_ptr obj = get_fa_object(obj_key);
		std::string f_s = gn::to_string(f, gn::FPFormat::Eng);
		obj->set_fdata(f_s);
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_fa_fdata : ", e.what());
	}
}

int gn_fa_fdata_e(const char *obj_key, const char *f) {
	try {
		fa_ptr obj = get_fa_object(obj_key);
		obj->set_fdata(f);
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_fa_fdata_e : ", e.what());
	}
}

int gn_fa_fixed_tone(const char *obj_key, const char *comp_key, GnFACompTag tag,
		double freq, int ssb) {
	try {
		fa_ptr obj = get_fa_object(obj_key);
		gn::FACompTag t = gn::get_enum<gn::FACompTag>(tag);
		std::string freq_s = gn::to_string(freq, gn::FPFormat::Eng);
		obj->add_fixed_tone(comp_key, t, freq_s, ssb);
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_fa_fixed_tone : ",
				e.what());
	}
}

int gn_fa_fixed_tone_e(const char *obj_key, const char *comp_key,
		GnFACompTag tag, const char *freq, int ssb) {
	try {
		fa_ptr obj = get_fa_object(obj_key);
		gn::FACompTag t = gn::get_enum<gn::FACompTag>(tag);
		obj->add_fixed_tone(comp_key, t, freq, ssb);
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_fa_fixed_tone_e : ",
				e.what());
	}
}

int gn_fa_fsample(const char *obj_key, double f) {
	try {
		fa_ptr obj = get_fa_object(obj_key);
		std::string f_s = gn::to_string(f, gn::FPFormat::Eng);
		obj->set_fsample(f_s);
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_fa_fsample : ", e.what());
	}
}

int gn_fa_fsample_e(const char *obj_key, const char *f) {
	try {
		fa_ptr obj = get_fa_object(obj_key);
		obj->set_fsample(f);
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_fa_fsample_e : ",
				e.what());
	}
}

int gn_fa_fshift(const char *obj_key, double f) {
	try {
		fa_ptr obj = get_fa_object(obj_key);
		std::string f_s = gn::to_string(f, gn::FPFormat::Eng);
		obj->set_fshift(f_s);
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_fa_fshift : ", e.what());
	}
}

int gn_fa_fshift_e(const char *obj_key, const char *f) {
	try {
		fa_ptr obj = get_fa_object(obj_key);
		obj->set_fshift(f);
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_fa_fshift_e : ", e.what());
	}
}

int gn_fa_fund_images(const char *obj_key, bool enable) {
	try {
		fa_ptr obj = get_fa_object(obj_key);
		obj->en_fund_images = enable;
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_fa_fund_images : ",
				e.what());
	}
}

int gn_fa_hd(const char *obj_key, int n) {
	try {
		fa_ptr obj = get_fa_object(obj_key);
		obj->set_hd(n);
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_fa_hd : ", e.what());
	}
}

int gn_fa_ilv(const char *obj_key, const int *ilv, size_t ilv_size,
		bool as_noise) {
	try {
		fa_ptr obj = get_fa_object(obj_key);
		std::set<int> ilv2(ilv, ilv + ilv_size);
		obj->set_ilv(ilv2);
		obj->ilv_as_noise = as_noise;
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_fa_ilv : ", e.what());
	}
}

int gn_fa_imd(const char *obj_key, int n) {
	try {
		fa_ptr obj = get_fa_object(obj_key);
		obj->set_imd(n);
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_fa_imd : ", e.what());
	}
}

int gn_fa_load(char *buf, size_t size, const char *filename,
		const char *obj_key) {
	try {
		std::string key(obj_key);
		if (key.empty()) {
			key = util::get_object_key_from_filename(filename);
		}
		gn::manager::add_object(
				key, gn::fourier_analysis::load(filename), true);
		util::fill_string_buffer(key.data(), key.size(), buf, size);
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_fa_load : ", e.what());
	}
}

int gn_fa_max_tone(const char *obj_key, const char *comp_key, GnFACompTag tag,
		int ssb) {
	try {
		fa_ptr obj = get_fa_object(obj_key);
		gn::FACompTag t = gn::get_enum<gn::FACompTag>(tag);
		obj->add_max_tone(comp_key, t, "0.0", "fdata", ssb);
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_fa_max_tone : ", e.what());
	}
}

int gn_fa_preview(char *buf, size_t size, const char *cfg_id, bool cplx) {
	try {
		fa_ptr obj = get_fa_object_or_load_from_file(cfg_id);
		std::string s = obj->preview(cplx);
		util::fill_string_buffer(s.data(), s.size(), buf, size);
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_fa_preview : ", e.what());
	}
}

int gn_fa_quad_errors(const char *obj_key, bool enable) {
	try {
		fa_ptr obj = get_fa_object(obj_key);
		obj->en_quad_errors = enable;
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_fa_quad_errors : ",
				e.what());
	}
}

int gn_fa_remove_comp(const char *obj_key, const char *comp_key) {
	try {
		fa_ptr obj = get_fa_object(obj_key);
		obj->remove_comp(comp_key);
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_fa_remove_comp : ",
				e.what());
	}
}

int gn_fa_reset(const char *obj_key) {
	try {
		fa_ptr obj = get_fa_object(obj_key);
		obj->reset();
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_fa_reset : ", e.what());
	}
}

int gn_fa_ssb(const char *obj_key, GnFASsb group, int ssb) {
	try {
		gn::FASsb g = gn::get_enum<gn::FASsb>(group);
		fa_ptr obj = get_fa_object(obj_key);
		obj->set_ssb(g, ssb);
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_fa_ssb_dc : ", e.what());
	}
}

int gn_fa_var(const char *obj_key, const char *name, double value) {
	try {
		fa_ptr obj = get_fa_object(obj_key);
		obj->set_var(name, value);
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_fa_var : ", e.what());
	}
}

int gn_fa_wo(const char *obj_key, int n) {
	try {
		fa_ptr obj = get_fa_object(obj_key);
		obj->set_wo(n);
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_fa_wo : ", e.what());
	}
}

/**************************************************************************/
/* Fourier Analysis Results                                               */
/**************************************************************************/

int gn_fa_result(double *result, const char **rkeys, size_t rkeys_size,
		const double *rvalues, size_t rvalues_size, const char *rkey) {
	try {
		util::check_pointer(result);
		if (rkeys_size != rvalues_size) {
			throw std::runtime_error(
					"Size of result keys does not match size of result values");
		}
		size_t key_index =
				get_fa_result_key_index(rkeys, rkeys_size, rkey);
		*result = rvalues[key_index];
		return gn_success;
	} catch (const std::exception &e) {
		*result = 0.0;
		return util::return_on_exception("gn_fa_result : ", e.what());
	}
}

int gn_fa_result_string(char *result, size_t result_size, const char **rkeys,
		size_t rkeys_size, const double *rvalues,
		size_t rvalues_size, const char *rkey) {
	try {
		util::check_pointer(result);
		std::string rstr = get_fa_result_string(
				rkeys, rkeys_size, rvalues, rvalues_size, rkey);
		util::fill_string_buffer(rstr.data(), rstr.size(), result,
				result_size);
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_fa_result_string : ",
				e.what());
	}
}

/**************************************************************************/
/* Fourier Analysis Helpers                                               */
/**************************************************************************/

int gn_fa_load_key_size(size_t *size, const char *filename, const char *obj_key) {
	try {
		util::check_pointer(size);
		std::string key(obj_key);
		if (key.empty()) {
			key = util::get_object_key_from_filename(filename);
		}
		*size = util::terminated_size(key.size());
		return gn_success;
	} catch (const std::exception &e) {
		*size = 0;
		return util::return_on_exception("gn_fa_load_key_size : ",
				e.what());
	}
}

int gn_fa_preview_size(size_t *size, const char *cfg_id, bool cplx) {
	try {
		util::check_pointer(size);
		fa_ptr obj = get_fa_object_or_load_from_file(cfg_id);
		std::string s = obj->preview(cplx);
		*size = util::terminated_size(s.size());
		return gn_success;
	} catch (const std::exception &e) {
		*size = 0;
		return util::return_on_exception("gn_fa_preview_size : ",
				e.what());
	}
}

int gn_fa_result_string_size(size_t *size, const char **rkeys,
		size_t rkeys_size, const double *rvalues,
		size_t rvalues_size, const char *rkey) {
	try {
		util::check_pointer(size);
		std::string rstr = get_fa_result_string(
				rkeys, rkeys_size, rvalues, rvalues_size, rkey);
		*size = util::terminated_size(rstr.size());
		return gn_success;
	} catch (const std::exception &e) {
		*size = 0;
		return util::return_on_exception("gn_fa_result_string_size : ",
				e.what());
	}
}

int gn_fft_analysis_results_key_sizes(size_t *key_sizes, size_t key_sizes_size,
		const char *cfg_id, size_t in_size,
		size_t nfft) {
	try {
		fa_ptr obj = get_fa_object_or_load_from_file(cfg_id);
		std::vector<size_t> key_sizes_src =
				obj->result_key_lengths(in_size, nfft);
		if (key_sizes_src.size() != key_sizes_size) {
			throw std::runtime_error(
					"Number of keys does not match output array size");
		}
		for (size_t i = 0; i < key_sizes_size; ++i) {
			key_sizes[i] = util::terminated_size(key_sizes_src[i]);
		}
		return gn_success;
	} catch (const std::exception &e) {
		std::fill(key_sizes, key_sizes + key_sizes_size, 0);
		return util::return_on_exception("gn_fa_results_key_sizes : ",
				e.what());
	}
}

int gn_fft_analysis_results_size(size_t *size, const char *cfg_id,
		size_t in_size, size_t nfft) {
	try {
		util::check_pointer(size);
		fa_ptr obj = get_fa_object_or_load_from_file(cfg_id);
		*size = obj->results_size(in_size, nfft);
		return gn_success;
	} catch (const std::exception &e) {
		*size = 0;
		return util::return_on_exception("gn_fa_results_size : ",
				e.what());
	}
}

/**************************************************************************/
/* Fourier Transforms                                                     */
/**************************************************************************/

namespace {

template <typename T>
int gn_fftxx(const char *suffix, double *out, size_t out_size, const T *i,
		size_t i_size, const T *q, size_t q_size, int n, size_t navg,
		size_t nfft, GnWindow window, GnCodeFormat format) {
	try {
		gn::Window w = gn::get_enum<gn::Window>(window);
		gn::CodeFormat f = gn::get_enum<gn::CodeFormat>(format);
		gn::fft(i, i_size, q, q_size, out, out_size, n, navg, nfft, w,
				f);
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_fft", suffix, " : ",
				e.what());
	}
}

template <typename T>
int gn_rfftxx(const char *suffix, double *out, size_t out_size, const T *in,
		size_t in_size, int n, size_t navg, size_t nfft, GnWindow window,
		GnCodeFormat format, GnRfftScale scale) {
	try {
		gn::Window w = gn::get_enum<gn::Window>(window);
		gn::CodeFormat f = gn::get_enum<gn::CodeFormat>(format);
		gn::RfftScale s = gn::get_enum<gn::RfftScale>(scale);
		gn::rfft(in, in_size, out, out_size, n, navg, nfft, w, f, s);
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_rfft", suffix, " : ",
				e.what());
	}
}

} // namespace

int gn_fft(double *out, size_t out_size, const double *i, size_t i_size,
		const double *q, size_t q_size, size_t navg, size_t nfft,
		GnWindow window) {
	try {
		gn::Window w = gn::get_enum<gn::Window>(window);
		gn::fft(i, i_size, q, q_size, out, out_size, navg, nfft, w);
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_fft : ", e.what());
	}
}

int gn_fft16(double *out, size_t out_size, const int16_t *i, size_t i_size,
		const int16_t *q, size_t q_size, int n, size_t navg, size_t nfft,
		GnWindow window, GnCodeFormat format) {
	return gn_fftxx("16", out, out_size, i, i_size, q, q_size, n, navg,
			nfft, window, format);
}

int gn_fft32(double *out, size_t out_size, const int32_t *i, size_t i_size,
		const int32_t *q, size_t q_size, int n, size_t navg, size_t nfft,
		GnWindow window, GnCodeFormat format) {
	return gn_fftxx("32", out, out_size, i, i_size, q, q_size, n, navg,
			nfft, window, format);
}

int gn_fft64(double *out, size_t out_size, const int64_t *i, size_t i_size,
		const int64_t *q, size_t q_size, int n, size_t navg, size_t nfft,
		GnWindow window, GnCodeFormat format) {
	return gn_fftxx("64", out, out_size, i, i_size, q, q_size, n, navg,
			nfft, window, format);
}

int gn_rfft(double *out, size_t out_size, const double *in, size_t in_size,
		size_t navg, size_t nfft, GnWindow window, GnRfftScale scale) {
	try {
		gn::Window w = gn::get_enum<gn::Window>(window);
		gn::RfftScale s = gn::get_enum<gn::RfftScale>(scale);
		gn::rfft(in, in_size, out, out_size, navg, nfft, w, s);
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_rfft : ", e.what());
	}
}

int gn_rfft16(double *out, size_t out_size, const int16_t *in, size_t in_size,
		int n, size_t navg, size_t nfft, GnWindow window,
		GnCodeFormat format, GnRfftScale scale) {
	return gn_rfftxx("16", out, out_size, in, in_size, n, navg, nfft,
			window, format, scale);
}

int gn_rfft32(double *out, size_t out_size, const int32_t *in, size_t in_size,
		int n, size_t navg, size_t nfft, GnWindow window,
		GnCodeFormat format, GnRfftScale scale) {
	return gn_rfftxx("32", out, out_size, in, in_size, n, navg, nfft,
			window, format, scale);
}

int gn_rfft64(double *out, size_t out_size, const int64_t *in, size_t in_size,
		int n, size_t navg, size_t nfft, GnWindow window,
		GnCodeFormat format, GnRfftScale scale) {
	return gn_rfftxx("64", out, out_size, in, in_size, n, navg, nfft,
			window, format, scale);
}

/**************************************************************************/
/* Fourier Transform Helpers                                              */
/**************************************************************************/

int gn_fft_size(size_t *out_size, size_t i_size, size_t q_size, size_t navg,
		size_t nfft) {
	try {
		util::check_pointer(out_size);
		*out_size = gn::fft_size(i_size, q_size, navg, nfft);
		return gn_success;
	} catch (const std::exception &e) {
		*out_size = 0;
		return util::return_on_exception("gn_fft_size : ", e.what());
	}
}

int gn_rfft_size(size_t *out_size, size_t in_size, size_t navg, size_t nfft) {
	try {
		util::check_pointer(out_size);
		*out_size = gn::rfft_size(in_size, navg, nfft);
		return gn_success;
	} catch (const std::exception &e) {
		*out_size = 0;
		return util::return_on_exception("gn_rfft_size : ", e.what());
	}
}

/**************************************************************************/
/* Fourier Utilities                                                      */
/**************************************************************************/

int gn_alias(double *out, double fs, double freq, GnFreqAxisType axis_type) {
	try {
		util::check_pointer(out);
		gn::FreqAxisType at = gn::get_enum<gn::FreqAxisType>(axis_type);
		*out = gn::alias(fs, freq, at);
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_alias : ", e.what());
	}
}

int gn_coherent(double *out, size_t nfft, double fs, double freq) {
	try {
		util::check_pointer(out);
		*out = gn::coherent(nfft, fs, freq);
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_coherent : ", e.what());
	}
}

int gn_fftshift(double *out, size_t out_size, const double *in, size_t in_size) {
	try {
		gn::fftshift(in, in_size, out, out_size);
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_fftshift : ", e.what());
	}
}

int gn_freq_axis(double *out, size_t size, size_t nfft,
		GnFreqAxisType axis_type, double fs,
		GnFreqAxisFormat axis_format) {
	try {
		gn::FreqAxisType at = gn::get_enum<gn::FreqAxisType>(axis_type);
		gn::FreqAxisFormat af =
				gn::get_enum<gn::FreqAxisFormat>(axis_format);
		gn::freq_axis(out, size, nfft, at, fs, af);
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_fftshift : ", e.what());
	}
}

int gn_ifftshift(double *out, size_t out_size, const double *in, size_t in_size) {
	try {
		gn::ifftshift(in, in_size, out, out_size);
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_ifftshift : ", e.what());
	}
}

/**************************************************************************/
/* Fourier Utility Helpers                                                */
/**************************************************************************/

int gn_freq_axis_size(size_t *size, size_t nfft, GnFreqAxisType axis_type) {
	try {
		util::check_pointer(size);
		gn::FreqAxisType at = gn::get_enum<gn::FreqAxisType>(axis_type);
		*size = gn::freq_axis_size(nfft, at);
		return gn_success;
	} catch (const std::exception &e) {
		*size = 0;
		return util::return_on_exception("gn_freq_axis_size : ",
				e.what());
	}
}

/**************************************************************************/
/* Manager                                                                */
/**************************************************************************/

int gn_mgr_clear() {
	gn::manager::clear();
	return gn_success;
}

int gn_mgr_compare(bool *result, const char *obj_key1, const char *obj_key2) {
	try {
		*result = gn::manager::equal(obj_key1, obj_key2);
		return gn_success;
	} catch (const std::exception &e) {
		*result = false;
		return util::return_on_exception("gn_mgr_equal : ", e.what());
	}
}

int gn_mgr_contains(bool *result, const char *obj_key) {
	*result = gn::manager::contains(obj_key);
	return gn_success;
}

int gn_mgr_remove(const char *obj_key) {
	gn::manager::remove(obj_key);
	return gn_success;
}

int gn_mgr_save(char *buf, size_t size, const char *obj_key,
		const char *filename) {
	try {
		std::string fn = gn::manager::save(obj_key, filename);
		util::fill_string_buffer(fn.data(), fn.size(), buf, size);
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_mgr_save : ", e.what());
	}
}

int gn_mgr_size(size_t *size) {
	*size = gn::manager::size();
	return gn_success;
}

int gn_mgr_to_string(char *buf, size_t size, const char *obj_key) {
	try {
		std::string s = gn::manager::to_string(obj_key);
		util::fill_string_buffer(s.data(), s.size(), buf, size);
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_mgr_to_string : ",
				e.what());
	}
}

int gn_mgr_type(char *buf, size_t size, const char *obj_key) {
	try {
		std::string s = gn::manager::type_str(obj_key);
		util::fill_string_buffer(s.data(), s.size(), buf, size);
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_mgr_type : ", e.what());
	}
}

/**************************************************************************/
/* Manager Helpers                                                        */
/**************************************************************************/

int gn_mgr_save_filename_size(size_t *size, const char *obj_key,
		const char *filename) {
	try {
		util::check_pointer(size);
		size_t fn_size = gn::manager::get_filename_from_object_key(
				obj_key, filename)
								 .size();
		*size = util::terminated_size(fn_size);
		return gn_success;
	} catch (const std::exception &e) {
		*size = 0;
		return util::return_on_exception("gn_mgr_save_filename_size : ",
				e.what());
	}
}

int gn_mgr_to_string_size(size_t *size, const char *obj_key) {
	try {
		util::check_pointer(size);
		*size = util::terminated_size(
				gn::manager::to_string(obj_key).size());
		return gn_success;
	} catch (const std::exception &e) {
		*size = 0;
		return util::return_on_exception("gn_mgr_to_string_size : ",
				e.what());
	}
}

int gn_mgr_type_size(size_t *size, const char *obj_key) {
	try {
		util::check_pointer(size);
		*size = util::terminated_size(
				gn::manager::type_str(obj_key).size());
		return gn_success;
	} catch (const std::exception &e) {
		*size = 0;
		return util::return_on_exception("gn_mgr_type_size : ",
				e.what());
	}
}

/**************************************************************************/
/* Signal Processing                                                      */
/**************************************************************************/

namespace {

template <typename T>
int gn_downsamplex(const char *suffix, T *out, size_t out_size, const T *in,
		size_t in_size, int ratio, bool interleaved) {
	try {
		gn::downsample(in, in_size, out, out_size, ratio, interleaved);
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_downsample", suffix, " : ",
				e.what());
	}
}

template <typename T>
int gn_fshiftx(const char *suffix, T *out, size_t out_size, const T *i,
		size_t i_size, const T *q, size_t q_size, int n, double fs,
		double fshift, GnCodeFormat format) {
	try {
		gn::CodeFormat f = gn::get_enum<gn::CodeFormat>(format);
		gn::fshift(i, i_size, q, q_size, out, out_size, n, fs, fshift,
				f);
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_fshift", suffix, " : ",
				e.what());
	}
}

template <typename T>
int gn_normalize(const char *suffix, double *out, size_t out_size, const T *in,
		size_t in_size, int n, GnCodeFormat format) {
	try {
		gn::CodeFormat f = gn::get_enum<gn::CodeFormat>(format);
		gn::normalize(in, in_size, out, out_size, n, f);
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_normalize", suffix, " : ",
				e.what());
	}
}

template <typename T>
int gn_quantize(const char *suffix, T *out, size_t out_size, const double *in,
		size_t in_size, double fsr, int n, double noise,
		GnCodeFormat format) {
	try {
		gn::CodeFormat f = gn::get_enum<gn::CodeFormat>(format);
		gn::quantize(in, in_size, out, out_size, fsr, n, noise, f);
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_quantize", suffix, " : ",
				e.what());
	}
}

} // namespace

int gn_downsample(double *out, size_t out_size, const double *in,
		size_t in_size, int ratio, bool interleaved) {
	return gn_downsamplex("", out, out_size, in, in_size, ratio,
			interleaved);
}

int gn_downsample16(int16_t *out, size_t out_size, const int16_t *in,
		size_t in_size, int ratio, bool interleaved) {
	return gn_downsamplex("16", out, out_size, in, in_size, ratio,
			interleaved);
}

int gn_downsample32(int32_t *out, size_t out_size, const int32_t *in,
		size_t in_size, int ratio, bool interleaved) {
	return gn_downsamplex("32", out, out_size, in, in_size, ratio,
			interleaved);
}

int gn_downsample64(int64_t *out, size_t out_size, const int64_t *in,
		size_t in_size, int ratio, bool interleaved) {
	return gn_downsamplex("64", out, out_size, in, in_size, ratio,
			interleaved);
}

int gn_fshift(double *out, size_t out_size, const double *i, size_t i_size,
		const double *q, size_t q_size, double fs, double fshift) {
	try {
		gn::fshift(i, i_size, q, q_size, out, out_size, fs, fshift);
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_fshift : ", e.what());
	}
}

int gn_fshift16(int16_t *out, size_t out_size, const int16_t *i, size_t i_size,
		const int16_t *q, size_t q_size, int n, double fs,
		double fshift, GnCodeFormat format) {
	return gn_fshiftx("16", out, out_size, i, i_size, q, q_size, n, fs,
			fshift, format);
}

int gn_fshift32(int32_t *out, size_t out_size, const int32_t *i, size_t i_size,
		const int32_t *q, size_t q_size, int n, double fs,
		double fshift, GnCodeFormat format) {
	return gn_fshiftx("32", out, out_size, i, i_size, q, q_size, n, fs,
			fshift, format);
}

int gn_fshift64(int64_t *out, size_t out_size, const int64_t *i, size_t i_size,
		const int64_t *q, size_t q_size, int n, double fs,
		double fshift, GnCodeFormat format) {
	return gn_fshiftx("64", out, out_size, i, i_size, q, q_size, n, fs,
			fshift, format);
}

int gn_normalize16(double *out, size_t out_size, const int16_t *in,
		size_t in_size, int n, GnCodeFormat format) {
	return gn_normalize("16", out, out_size, in, in_size, n, format);
}

int gn_normalize32(double *out, size_t out_size, const int32_t *in,
		size_t in_size, int n, GnCodeFormat format) {
	return gn_normalize("32", out, out_size, in, in_size, n, format);
}

int gn_normalize64(double *out, size_t out_size, const int64_t *in,
		size_t in_size, int n, GnCodeFormat format) {
	return gn_normalize("64", out, out_size, in, in_size, n, format);
}

int gn_polyval(double *out, size_t out_size, const double *in, size_t in_size,
		const double *c, size_t c_size) {
	try {
		gn::polyval(in, in_size, out, out_size, c, c_size);
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_polyval : ", e.what());
	}
}

int gn_quantize16(int16_t *out, size_t out_size, const double *in,
		size_t in_size, double fsr, int n, double noise,
		GnCodeFormat format) {
	return gn_quantize("16", out, out_size, in, in_size, fsr, n, noise,
			format);
}

int gn_quantize32(int32_t *out, size_t out_size, const double *in,
		size_t in_size, double fsr, int n, double noise,
		GnCodeFormat format) {
	return gn_quantize("32", out, out_size, in, in_size, fsr, n, noise,
			format);
}

int gn_quantize64(int64_t *out, size_t out_size, const double *in,
		size_t in_size, double fsr, int n, double noise,
		GnCodeFormat format) {
	return gn_quantize("64", out, out_size, in, in_size, fsr, n, noise,
			format);
}

/**************************************************************************/
/* Signal Processing Helpers                                              */
/**************************************************************************/

int gn_downsample_size(size_t *out_size, size_t in_size, int ratio,
		bool interleaved) {
	try {
		util::check_pointer(out_size);
		*out_size = gn::downsample_size(in_size, ratio, interleaved);
		return gn_success;
	} catch (const std::exception &e) {
		*out_size = 0;
		return util::return_on_exception("gn_downsample_size : ",
				e.what());
	}
}

int gn_fshift_size(size_t *out_size, size_t i_size, size_t q_size) {
	try {
		util::check_pointer(out_size);
		*out_size = gn::fshift_size(i_size, q_size);
		return gn_success;
	} catch (const std::exception &e) {
		*out_size = 0;
		return util::return_on_exception("gn_fshift_size : ", e.what());
	}
}

/**************************************************************************/
/* Waveforms                                                              */
/**************************************************************************/

namespace {

template <typename T>
int gn_wf_analysisx(const char *suffix, char **rkeys, size_t rkeys_size,
		double *rvalues, size_t rvalues_size, const T *in,
		size_t in_size) {
	try {
		util::check_pointer(rkeys);
		util::check_pointer(rvalues);
		const std::vector<std::string> &keys =
				gn::wf_analysis_ordered_keys();
		if (keys.size() != rkeys_size) {
			throw std::runtime_error(
					"Size of result key array is wrong");
		}
		if (rvalues_size != rkeys_size) {
			throw std::runtime_error(
					"Size of result keys does not match size of result values");
		}
		std::map<std::string, double> results =
				gn::wf_analysis(in, in_size);
		for (size_t i = 0; i < keys.size(); ++i) {
			const std::string &src = keys[i];
			char *dst = rkeys[i];
			size_t dst_size = util::terminated_size(src.size());
			util::fill_string_buffer(src.data(), src.size(), dst,
					dst_size);
			rvalues[i] = results.at(src);
		}
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_wf_analysis", suffix,
				" : ", e.what());
	}
}

} // namespace

int gn_cos(double *out, size_t size, double fs, double ampl, double freq,
		double phase, double td, double tj) {
	try {
		gn::cos(out, size, fs, ampl, freq, phase, td, tj);
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_cos : ", e.what());
	}
}

int gn_gaussian(double *out, size_t size, double mean, double sd) {
	try {
		gn::gaussian(out, size, mean, sd);
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_gaussian : ", e.what());
	}
}

int gn_ramp(double *out, size_t size, double start, double stop, double noise) {
	try {
		gn::ramp(out, size, start, stop, noise);
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_ramp : ", e.what());
	}
}

int gn_sin(double *out, size_t size, double fs, double ampl, double freq,
		double phase, double td, double tj) {
	try {
		gn::sin(out, size, fs, ampl, freq, phase, td, tj);
		return gn_success;
	} catch (const std::exception &e) {
		return util::return_on_exception("gn_sin : ", e.what());
	}
}

int gn_wf_analysis(char **rkeys, size_t rkeys_size, double *rvalues,
		size_t rvalues_size, const double *in, size_t in_size) {
	return gn_wf_analysisx("", rkeys, rkeys_size, rvalues, rvalues_size, in,
			in_size);
}

int gn_wf_analysis16(char **rkeys, size_t rkeys_size, double *rvalues,
		size_t rvalues_size, const int16_t *in, size_t in_size) {
	return gn_wf_analysisx("16", rkeys, rkeys_size, rvalues, rvalues_size,
			in, in_size);
}

int gn_wf_analysis32(char **rkeys, size_t rkeys_size, double *rvalues,
		size_t rvalues_size, const int32_t *in, size_t in_size) {
	return gn_wf_analysisx("32", rkeys, rkeys_size, rvalues, rvalues_size,
			in, in_size);
}

int gn_wf_analysis64(char **rkeys, size_t rkeys_size, double *rvalues,
		size_t rvalues_size, const int64_t *in, size_t in_size) {
	return gn_wf_analysisx("64", rkeys, rkeys_size, rvalues, rvalues_size,
			in, in_size);
}