// Copyright (C) 2024-2026 Analog Devices, Inc.
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

/**
 * @brief Central class for Fourier-analysis-based RF performance metrics.
 *
 * Computes SNR, SINAD, SFDR, THD, and other spectral metrics for
 * data-converter outputs. Users configure the analysis by defining signal
 * components, setting sample/data rates, and specifying distortion orders
 * before calling analyze().
 */
class fourier_analysis final : public object {
public:
	using mask_map = std::map<int, fourier_analysis_comp_mask>;
	using min_max_def_t = std::tuple<int, int, int>;
	using var_map = expression::var_map;

public:
	/**
	 * @brief Factory method to create a new fourier_analysis object.
	 *
	 * @return Shared pointer to a new fourier_analysis instance.
	 */
	static std::shared_ptr<fourier_analysis> create() {
		return std::make_shared<fourier_analysis>();
	}

	/**
	 * @brief Load a fourier_analysis configuration from a JSON file.
	 *
	 * @param filename Path to the JSON configuration file.
	 * @return Shared pointer to the loaded fourier_analysis instance.
	 */
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
	/**
	 * @brief Run Fourier analysis on FFT magnitude-squared data.
	 *
	 * Identifies configured signal, distortion, and noise components in the
	 * spectrum and computes performance metrics (SNR, SINAD, SFDR, THD, etc.).
	 *
	 * @param in_data   Pointer to magnitude-squared FFT data.
	 * @param in_size   Number of elements in @p in_data.
	 * @param nfft      FFT size used to produce @p in_data.
	 * @param axis_type Frequency axis type of the input data.
	 * @return A fourier_analysis_results object containing all computed metrics.
	 */
	fourier_analysis_results analyze(const real_t *in_data,
			const size_t in_size,
			const size_t nfft,
			FreqAxisType axis_type) const;

public: // Component Definition
	/**
	 * @brief Add a tone component at a fixed frequency.
	 *
	 * The frequency is specified as an expression string that may reference
	 * user-defined variables.
	 *
	 * @param key  Unique key identifying this component.
	 * @param tag  Classification tag (Signal, HD, IMD, UserDist, etc.).
	 * @param freq Frequency expression string.
	 * @param ssb  Number of single-side bins (-1 to use the default).
	 */
	void add_fixed_tone(const str_t &key, FACompTag tag, const str_t &freq,
			int ssb);

	/**
	 * @brief Add a tone component located at the spectral maximum within a
	 * search range.
	 *
	 * The search range is defined by center and width expression strings.
	 *
	 * @param key    Unique key identifying this component.
	 * @param tag    Classification tag (Signal, HD, IMD, UserDist, etc.).
	 * @param center Center frequency expression for the search range.
	 * @param width  Width expression for the search range.
	 * @param ssb    Number of single-side bins (-1 to use the default).
	 */
	void add_max_tone(const str_t &key, FACompTag tag, const str_t &center,
			const str_t &width, int ssb);

	/**
	 * @brief Remove a previously added user-defined component.
	 *
	 * @param key Key of the component to remove.
	 */
	void remove_comp(const str_t &key);

public: // Configuration Getters
	/** @brief Get the analysis band center expression. */
	str_t ab_center() const {
		return m_ab_center;
	}
	/** @brief Get the analysis band width expression. */
	str_t ab_width() const {
		return m_ab_width;
	}
	/** @brief Get the set of clock sub-harmonic divisors. */
	std::set<int> clk() const {
		return m_clk;
	}
	/** @brief Get the data rate expression. */
	str_t fdata() const {
		return m_fdata;
	}
	/** @brief Get the sample rate expression. */
	str_t fsample() const {
		return m_fdata;
	}
	/** @brief Get the shift frequency expression. */
	str_t fshift() const {
		return m_fdata;
	}
	/** @brief Get the maximum harmonic distortion order. */
	int hd() const {
		return m_hd;
	}
	/** @brief Get the set of interleaving factors. */
	std::set<int> ilv() const {
		return m_ilv;
	}
	/** @brief Get the maximum intermodulation distortion order. */
	int imd() const {
		return m_imd;
	}
	/** @brief Get the number of worst-other tones. */
	int wo() const {
		return m_wo;
	}

	/**
	 * @brief Get the number of single-side bins for a component group.
	 *
	 * @param group Component group (DC, Signal, WO, or Default).
	 * @return Number of single-side bins for the specified group.
	 */
	int ssb(FASsb group) const;

public: // Configuration Setters
	/**
	 * @brief Set the analysis band center and width.
	 *
	 * Only spectral content within the analysis band is included in metric
	 * computations.
	 *
	 * @param center Center frequency expression string.
	 * @param width  Width expression string.
	 */
	void set_analysis_band(const str_t &center, const str_t &width);

	/**
	 * @brief Set clock sub-harmonic divisors.
	 *
	 * Clock spurs at fs/N are identified and classified as CLK components.
	 *
	 * @param clk Set of integer divisors.
	 */
	void set_clk(const std::set<int> &clk);

	/**
	 * @brief Set the data rate as an expression string.
	 *
	 * @param expr Data rate expression (may reference user-defined variables).
	 */
	void set_fdata(const str_t &expr);

	/**
	 * @brief Set the sample rate as an expression string.
	 *
	 * @param expr Sample rate expression (may reference user-defined variables).
	 */
	void set_fsample(const str_t &expr);

	/**
	 * @brief Set the shift frequency as an expression string.
	 *
	 * The shift frequency represents the cumulative frequency translation
	 * applied after sampling.
	 *
	 * @param expr Shift frequency expression.
	 */
	void set_fshift(const str_t &expr);

	/**
	 * @brief Set the maximum harmonic distortion order.
	 *
	 * Harmonics 2 through @p n of each signal tone are automatically generated.
	 *
	 * @param n Maximum harmonic order.
	 */
	void set_hd(int n);

	/**
	 * @brief Set interleaving factors.
	 *
	 * Interleaving spurs at fs/N offsets are identified and classified.
	 *
	 * @param ilv Set of interleaving factors.
	 */
	void set_ilv(const std::set<int> &ilv);

	/**
	 * @brief Set the maximum intermodulation distortion order.
	 *
	 * IMD products up to order @p n are automatically generated when multiple
	 * signal tones are present.
	 *
	 * @param n Maximum IMD order.
	 */
	void set_imd(int n);

	/**
	 * @brief Set the number of single-side bins for a component group.
	 *
	 * Each tone occupies 2*ssb+1 bins total (ssb bins on each side of the
	 * center bin).
	 *
	 * @param group Component group (DC, Signal, WO, or Default).
	 * @param ssb   Number of single-side bins.
	 */
	void set_ssb(FASsb group, int ssb);

	/**
	 * @brief Set the value of an expression variable.
	 *
	 * Variables can be referenced in frequency expressions (e.g., fdata,
	 * fshift, tone frequencies).
	 *
	 * @param key Variable name.
	 * @param x   Variable value.
	 */
	void set_var(const str_t &key, real_t x);

	/**
	 * @brief Set the number of worst-other tones to identify.
	 *
	 * Worst-others are the @p n largest spectral components not accounted for
	 * by other defined components.
	 *
	 * @param n Number of worst-other tones.
	 */
	void set_wo(int n);

public: // Key Queries
	/**
	 * @brief Check whether a component key is reserved (used internally).
	 *
	 * @param key Key to check.
	 * @return True if the key is reserved.
	 */
	static bool is_reserved(const str_t &key);

	/**
	 * @brief Check whether a string is a valid component key format.
	 *
	 * @param key Key to validate.
	 * @return True if the key has a valid format.
	 */
	static bool is_valid(const str_t &key);

	/**
	 * @brief Check whether a key is valid, not reserved, and not already in use.
	 *
	 * @param key Key to check.
	 * @return True if the key is available for use.
	 */
	bool is_available(const str_t &key) const {
		return !is_reserved(key) && !is_comp(key) && !is_var(key) &&
				is_valid(key);
	}

	/**
	 * @brief Check whether a key is used by a user-defined component.
	 *
	 * @param key Key to check.
	 * @return True if the key is in use by a component.
	 */
	bool is_comp(const str_t &key) const {
		return !(m_user_comps.find(key) == m_user_comps.end());
	}

	/**
	 * @brief Check whether a key is used by a user-defined variable.
	 *
	 * @param key Key to check.
	 * @return True if the key is in use by a variable.
	 */
	bool is_var(const str_t &key) const {
		return !(m_user_vars.find(key) == m_user_vars.end());
	}

public: // Other Member Functions
	static str_t flat_tone_key(const str_t &key, int result_index);

	static std::pair<str_t, str_t> split_key(const str_t &key);

	/**
	 * @brief Return a string preview of the analysis configuration.
	 *
	 * Shows all configured components and their frequencies.
	 *
	 * @param cplx If true, include complex-specific components (e.g., images).
	 * @return Configuration preview as a formatted string.
	 */
	str_t preview(bool cplx) const;

	/**
	 * @brief Reset all configuration to default values.
	 *
	 * Removes all user-defined components and variables.
	 */
	void reset();

	std::vector<size_t> result_key_lengths(size_t in_size,
			size_t nfft) const;

	/**
	 * @brief Return the number of key-value pairs in the analysis results.
	 *
	 * @param in_size Number of elements in the input data array.
	 * @param nfft    FFT size.
	 * @return Number of result key-value pairs.
	 */
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
	/** @brief If true, classify clock components as noise rather than distortion. */
	bool clk_as_noise;
	/** @brief If true, classify DC as distortion rather than noise. */
	bool dc_as_dist;
	/** @brief If true, enable the converter offset component. */
	bool en_conv_offset;
	/** @brief If true, enable fundamental image components (for complex FFT analysis). */
	bool en_fund_images;
	/** @brief If true, enable quadrature error tone components (image, gain/phase imbalance). */
	bool en_quad_errors;
	/** @brief If true, classify interleaving components as noise rather than distortion. */
	bool ilv_as_noise;

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