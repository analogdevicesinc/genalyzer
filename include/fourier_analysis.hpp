// Copyright (C) 2024-2025 Analog Devices, Inc.
//
// SPDX short identifier: ADIBSD OR GPL-2.0-or-later
#ifndef GENALYZER_IMPL_FOURIER_ANALYSIS_HPP
#define GENALYZER_IMPL_FOURIER_ANALYSIS_HPP

#include "enums.hpp"
#include "expression.hpp"
#include "fourier_analysis_comp_mask.hpp"
#include "fourier_analysis_component.hpp"
#include "fourier_analysis_results.hpp"
#include "object.hpp"
#include "type_aliases.hpp"

#include <map>
#include <memory>
#include <set>
#include <tuple>

namespace genalyzer_impl {

class fourier_analysis final : public object {
public:
	using mask_map = std::map<int, fourier_analysis_comp_mask>;
	using min_max_def_t = std::tuple<int, int, int>;
	using var_map = expression::var_map;

public:
	static std::shared_ptr<fourier_analysis> create() {
		return std::make_shared<fourier_analysis>();
	}

	static std::shared_ptr<fourier_analysis> load(const str_t &filename);

	static const min_max_def_t mmd_hd;
	static const min_max_def_t mmd_imd;
	static const min_max_def_t mmd_wo;
	static const min_max_def_t mmd_ssb;

public: // Constructors, Destructor, and Assignment
	fourier_analysis();

	fourier_analysis(const fourier_analysis &);

	fourier_analysis(fourier_analysis &&);

	~fourier_analysis() = default;

	fourier_analysis &operator=(const fourier_analysis &);

	fourier_analysis &operator=(fourier_analysis &&);

public: // Analysis
	fourier_analysis_results analyze(const real_t *in_data,
			const size_t in_size,
			const size_t nfft,
			FreqAxisType axis_type) const;

public: // Component Definition
	void add_fixed_tone(const str_t &key, FACompTag tag, const str_t &freq,
			int ssb);

	void add_max_tone(const str_t &key, FACompTag tag, const str_t &center,
			const str_t &width, int ssb);

	void remove_comp(const str_t &key);

public: // Configuration Getters
	str_t ab_center() const {
		return m_ab_center;
	}
	str_t ab_width() const {
		return m_ab_width;
	}
	std::set<int> clk() const {
		return m_clk;
	}
	str_t fdata() const {
		return m_fdata;
	}
	str_t fsample() const {
		return m_fdata;
	}
	str_t fshift() const {
		return m_fdata;
	}
	int hd() const {
		return m_hd;
	}
	std::set<int> ilv() const {
		return m_ilv;
	}
	int imd() const {
		return m_imd;
	}
	int wo() const {
		return m_wo;
	}

	int ssb(FASsb group) const;

public: // Configuration Setters
	void set_analysis_band(const str_t &center, const str_t &width);

	void set_clk(const std::set<int> &clk);

	void set_fdata(const str_t &expr);

	void set_fsample(const str_t &expr);

	void set_fshift(const str_t &expr);

	void set_hd(int n);

	void set_ilv(const std::set<int> &ilv);

	void set_imd(int n);

	void set_ssb(FASsb group, int ssb);

	void set_var(const str_t &key, real_t x);

	void set_wo(int n);

public: // Key Queries
	static bool is_reserved(const str_t &key);

	static bool is_valid(const str_t &key);

	bool is_available(const str_t &key) const {
		return !is_reserved(key) && !is_comp(key) && !is_var(key) &&
				is_valid(key);
	}

	bool is_comp(const str_t &key) const {
		return !(m_user_comps.find(key) == m_user_comps.end());
	}

	bool is_var(const str_t &key) const {
		return !(m_user_vars.find(key) == m_user_vars.end());
	}

public: // Other Member Functions
	static str_t flat_tone_key(const str_t &key, int result_index);

	static std::pair<str_t, str_t> split_key(const str_t &key);

	str_t preview(bool cplx) const;

	void reset();

	std::vector<size_t> result_key_lengths(size_t in_size,
			size_t nfft) const;

	size_t results_size(size_t in_size, size_t nfft) const;

private:
	static const str_t key_pattern;
	static const str_t wo_pattern;
	static const std::set<str_t> reserved_keys;
	static const str_vector reserved_patterns;
	static const str_t flat_key_coupler;

private: // Virtual Function Overrides
	bool equals_impl(const object &that) const override;

	ObjectType object_type_impl() const override {
		return ObjectType::FourierAnalysis;
	}

	void save_impl(const str_t &filename) const override;

	str_t to_string_impl() const override;

private:
	using comp_ptr = std::unique_ptr<fourier_analysis_component>;
	using comp_map = std::map<str_t, comp_ptr>;
	using comp_data_t = std::tuple<str_vector, comp_map, std::set<str_t>>;

	static void add_comp(str_vector &keys, comp_map &comps, const str_t &k,
			comp_ptr c);

	static int limit_ssb(int ssb, int lower_limit);

	comp_data_t generate_comps(bool cplx) const;

	void if_key_not_available_throw(const str_t &key) const;

private: // Analysis and related subroutines
	static mask_map initialize_masks(bool cplx, size_t size);

	fourier_analysis_results analyze_impl(
			const real_t *msq_data, // mean-square FFT magnitude data
			const size_t msq_size, // size of ms_data
			const size_t nfft, // FFT size
			FreqAxisType axis_type, //
			const cplx_t *fft_data =
					nullptr, // complex FFT data; if provided, results include phase
			const size_t fft_size =
					0 // size of fft_data; if fft_data is not Null,
	) const; // fft_size should equal msq_size

	void finalize_masks(mask_map &masks) const;

	var_map initialize_vars(size_t nfft) const;

	void setup_analysis_band(bool cplx, fourier_analysis_comp_mask &mask,
			var_map &vars) const;

public: // Public configuration parameters
	bool clk_as_noise; // Treat CLK components as noise
	bool dc_as_dist; // Treat DC component as distortion
	bool en_conv_offset; // Enable converter offset component
	bool en_fund_images; // Enable fundamental image component(s)
	bool en_quad_errors; // Enable quadrature error tone components
	bool ilv_as_noise; // Treat ILV components as noise

private: // Private configuration parameters
	int m_hd; // Order of harmonic distortion
	int m_imd; // Order of intermodulation distortion
	int m_wo; // Number of worst others
	int m_ssb_def; // Default SSB (for auto-generated tone components)
	int m_ssb_dc; // SSB for DC
	int m_ssb_sig; // SSB for Signals
	int m_ssb_wo; // SSB for WO
	str_t m_ab_center; // Analysis band center
	str_t m_ab_width; // Analysis band width
	str_t m_fdata; // Data rate
	str_t m_fsample; // Sample rate
	str_t m_fshift; // Shift frequency (sum of frequency translations after
					// sampling)
	std::set<int> m_clk; // Clock sub-harmonic divisors
	std::set<int> m_ilv; // Interleaving factors

	// User-defined components and variables
	str_vector m_user_keys;
	comp_map m_user_comps;
	var_map m_user_vars;

}; // class fourier_analysis

} // namespace genalyzer_impl

#endif // GENALYZER_IMPL_FOURIER_ANALYSIS_HPP