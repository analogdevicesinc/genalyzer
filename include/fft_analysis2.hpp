/*
File        : $HeadURL: https://swdev.cld.analog.com/svn/projects/icdev/sandbox/users/pderouni/platform-upgrades/src/analysis/fft_analysis2.hpp $
Originator  : pderouni
Revision    : $Revision: 12767 $
Last Commit : $Date: 2020-08-24 16:38:41 -0400 (Mon, 24 Aug 2020) $
Last Editor : $Author: pderouni $
*/

#ifndef ICD_ANALYSIS_FFT_ANALYSIS2_HPP
#define ICD_ANALYSIS_FFT_ANALYSIS2_HPP

#include "abstract_object.hpp"
#include "type_aliases.hpp"
#include "types.hpp"
#include "var_map.hpp"
#include <regex>
#include <set>
#include <unordered_map>

namespace analysis {

class fft_component;
class fft_band;
class fft_fixed_tone;
class fft_max_tone;
class sparse_array_mask;
class var_vector;

/**
     * @brief The fft_analysis2 class
     */
class ICD_ANALYSIS_DECL fft_analysis2 final : public abstract_object {
    friend class abstract_object; // needs access to load_state

public:
    using shared_ptr = std::shared_ptr<fft_analysis2>;
    using unique_ptr = std::unique_ptr<fft_analysis2>;

public: /// @{ @name Factories
    /**
         * @brief create
         * @return
         */
    static unique_ptr create()
    {
        return std::make_unique<fft_analysis2>();
    }

    /// @}

public: /// @{ @name Deserialization
    /**
         * @brief load
         * @param filename
         * @return
         */
    static unique_ptr load(const str_t& filename);

    /// @}

public: /// @{ @name Result Formatting
    /**
         * @brief annotations
         * @param results
         * @param fmt
         * @return
         */
    static var_map annotations(const var_map& results,
        FFTAxisFormat fmt = FFTAxisFormat::Freq);

    /**
         * @brief result_data
         * @param results
         * @param spec
         * @return
         */
    static std::vector<str_vector> result_data(const var_map& results,
        const var_vector& spec);

    /// @}

public: /// @{ @name Key Queries
    /**
         * @brief is_reserved
         * @param key
         * @return
         */
    static bool is_reserved(const str_t& key);

    /**
         * @brief is_valid
         * @param key
         * @return
         */
    static bool is_valid(const str_t& key)
    {
        return std::regex_match(key, key_pattern);
    }

    /// @}

public: /// @{ @name Constructors
    /**
         * @brief Default constructor
         */
    fft_analysis2();

    /**
         * @brief Copy constructor
         * @param obj
         */
    fft_analysis2(const fft_analysis2& obj);

    /**
         * @brief Move constructor
         * @param obj
         */
    fft_analysis2(fft_analysis2&& obj);

    /// @}

public: /// @{ @name Destructor
    ~fft_analysis2();

    /// @}

public: /// @{ @name Assignment
    /**
         * @brief Copy assignment
         * @param obj
         * @return
         */
    fft_analysis2& operator=(const fft_analysis2& obj)
    {
        if (&obj != this) {
            fft_analysis2 the_copy(obj);
            std::swap(the_copy, *this);
        }
        return *this;
    }

    /**
         * @brief Move assignment
         * @param obj
         * @return
         */
    fft_analysis2& operator=(fft_analysis2&& obj);

    /// @}

public: /// @{ @name Analysis
    /**
         * @brief analyze
         * @return
         */
    var_map::unique_ptr analyze(
        const real_t* magms_data, ///< [in] pointer to magnitude array
        size_t magms_size, ///< [in] size of array magms_data
        const real_t* phase_data, ///< [in] pointer to phase array
        size_t phase_size, ///< [in] size of array phase_data
        size_t fft_size ///< [in] FFT size
        ) const;

    /// @}

public: /// @{ @name Analysis
    /**
         * @brief analyze_fft
         * @param data
         * @return
         */
    var_map::unique_ptr analyze_fft(const cplx_vector& data) const;

    /**
         * @brief analyze_fft
         * @param data
         * @return
         */
    var_map::unique_ptr analyze_fft(const real_vector& data) const;

    /**
         * @brief analyze_rfft
         * @param data
         * @param fft_size
         * @return
         */
    var_map::unique_ptr analyze_rfft(const cplx_vector& data,
        size_t fft_size = 0) const;

    /**
         * @brief analyze_rfft
         * @param data
         * @param fft_size
         * @return
         */
    var_map::unique_ptr analyze_rfft(const real_vector& data,
        size_t fft_size = 0) const;

    /// @}

public: /// @{ @name Component Definition
    /**
         * @brief add_band
         * @param key
         * @param tag
         * @param center
         * @param width
         * @param keep_out
         * @param omit_tones
         * @details A band is defined by its center and width, both of which
         * are given as expressions to be evaluated during the execution of the
         * analysis.  The expressions may reference other FFT components and
         * variables.
         */
    void add_band(const str_t& key,
        FFTCompTag tag,
        const str_t& center,
        const str_t& width,
        bool keep_out,
        bool omit_tones);

    /**
         * @brief add_fixed_tone
         * @param key
         * @param tag
         * @param freq
         * @param ssb
         */
    void add_fixed_tone(const str_t& key,
        FFTCompTag tag,
        const str_t& freq,
        int ssb);

    /**
         * @brief add_max_tone
         * @param key
         * @param tag
         * @param center
         * @param width
         * @param ssb
         */
    void add_max_tone(const str_t& key,
        FFTCompTag tag,
        const str_t& center,
        const str_t& width,
        int ssb);

    /// @}

public: /// @{ @name Configuration
    /**
         * @brief set_analysis_band
         * @param center Analysis band center
         * @param width Analysis band width
         */
    void set_analysis_band(const str_t& center, const str_t& width);

    /**
         * @brief set_clk
         * @param clk Clock sub-harmonic divisors
         */
    void set_clk(const std::vector<int>& clk);

    /**
         * @brief set_fdata
         * @param expr
         */
    void set_fdata(const str_t& expr);

    /**
         * @brief set_fsample
         * @param expr
         */
    void set_fsample(const str_t& expr);

    /**
         * @brief set_fshift
         * @param expr
         */
    void set_fshift(const str_t& expr);

    /**
         * @brief set_hd
         * @param hd Order of harmonic distortion
         */
    void set_hd(int hd);

    /**
         * @brief set_il
         * @param il Interleaving factors
         */
    void set_il(const std::vector<int>& il);

    /**
         * @brief set_imd
         * @param imd Order of intermodulation distortion
         */
    void set_imd(int imd);

    /**
         * @brief set_ssb_auto
         * @param ssb Number of single-side bins for auto-generated tones
         */
    void set_ssb_auto(int ssb);

    /**
         * @brief set_ssb_auto
         * @param win Window type
         */
    void set_ssb_auto(WindowType win);

    /**
         * @brief set_ssb_dc
         * @param ssb Number of single-side bins for DC
         */
    void set_ssb_dc(int ssb);

    /**
         * @brief set_ssb_wo
         * @param ssb Number of single-side bins for auto-generated Worst
         *        Other(s)
         */
    void set_ssb_wo(int ssb);

    /**
         * @brief set_var
         * @param name
         * @param value
         */
    void set_var(const str_t& name, real_t value);

    /**
         * @brief set_wo
         * @param wo Number of worst others
         */
    void set_wo(int wo);

public: /// @{ @name Other Member Functions
    /**
         * @brief clear
         */
    void clear()
    {
        fft_analysis2 new_obj{};
        std::swap(new_obj, *this);
    }

    /**
         * @brief comp_list
         * @param cplx If True, the list will contain tones due to quadrature
         *        error, according to other configuration options
         * @return
         */
    str_t comp_list(bool cplx) const;

    /// @}

public: /// @{ @name Key Queries
    /**
         * @brief is_available
         * @param key
         * @return
         */
    bool is_available(const str_t& key) const
    {
        return !is_key(key) && is_valid(key) && !is_reserved(key);
    }

    /**
         * @brief is_comp
         * @param key
         * @return
         */
    bool is_comp(const str_t& key) const
    {
        return m_user_comps.find(key) != m_user_comps.end();
    }

    /**
         * @brief is_key
         * @param key
         * @return
         */
    bool is_key(const str_t& key) const
    {
        return is_comp(key) || is_var(key);
    }

    /**
         * @brief is_var
         * @param key
         * @return
         */
    bool is_var(const str_t& key) const
    {
        return m_user_vars.find(key) != m_user_vars.end();
    }

    /// @}

protected: // abstract_object overrides
    void save_state(io_map& state) const override;

private: // abstract_object overrides
    object_ptr clone_impl() const override
    {
        return std::make_unique<fft_analysis2>(*this);
    }

    ObjectType object_type_impl() const override
    {
        return ObjectType::FFTAnalysis;
    }

private:
    static const std::map<str_t, int> param_defaults;

    static const std::regex key_pattern;

    static const str_t io_params_key;

    static const str_t io_comps_key;

    static const str_t io_vars_key;

private:
    static void add_result(std::vector<str_vector>& rows,
        const var_map& results,
        const str_t& key,
        str_t name = "",
        str_t units = "",
        int prec = 3);

    static void add_result(std::vector<str_vector>& rows,
        const var_map& comps,
        const var_map& metrics,
        const var_vector& row_spec);

    static str_t format_result(const var_map& results,
        const str_t& key,
        str_t& units,
        int prec);

    static unique_ptr load_state(const io_map& state);

    static void validate_results(const var_map& results);

private:
    void if_key_not_available_throw(const str_t& key)
    {
        if (is_key(key)) {
            throw base::exception("'" + key + "' already exists");
        } else if (!is_valid(key)) {
            throw base::exception("'" + key + "' is an invalid key");
        } else if (is_reserved(key)) {
            throw base::exception("'" + key + "' is a reserved key");
        }
    }

private:
    using comp_ptr = std::unique_ptr<fft_component>;
    using comp_map = std::unordered_map<str_t, comp_ptr>;
    using comp_data = std::tuple<str_vector, comp_map, std::set<str_t>>;

    bool add_comp(str_vector& keys,
        comp_map& comps,
        const str_t& key,
        comp_ptr&& comp) const;

    void load_comp(const str_t& key, const io_map& state);

private:
    var_map::unique_ptr analyze(const real_vector& ms_data,
        bool cplx,
        const size_t fft_size,
        const cplx_vector& cplx_data = {}) const;

private: // analyze sub-routines
    using mask_map = std::map<str_t, sparse_array_mask>;
    using me_vars_map = std::unordered_map<str_t, real_t>;

    comp_data generate_comps(bool cplx, size_t fft_size) const;

    var_map::unique_ptr meas_band(const real_vector& ms_data,
        bool cplx,
        size_t fft_size,
        me_vars_map& vars,
        mask_map& masks,
        const fft_band& comp) const;

    var_map::unique_ptr meas_fixed_tone(const real_vector& ms_data,
        bool cplx,
        size_t fft_size,
        me_vars_map& vars,
        mask_map& masks,
        const fft_fixed_tone& comp) const;

    var_map::unique_ptr meas_max_tone(const real_vector& ms_data,
        bool cplx,
        size_t fft_size,
        me_vars_map& vars,
        mask_map& masks,
        const fft_max_tone& comp) const;

    var_map::unique_ptr setup_ab(bool cplx,
        me_vars_map& vars,
        sparse_array_mask& mask) const;

    me_vars_map setup_vars(size_t fft_size) const;

    void update_maxspur(str_t& maxspur_key,
        real_t& maxspur_mag,
        const str_t& key,
        real_t mag,
        const var_map& results,
        mask_map& masks) const;

public: /// @{ @name Public Parameters
    bool m_axis_shift; //  1 T:[-0.5,0.5); F:[0,1) (IQ only)
    bool m_en_co; //  2 Enable Converter Offset
    bool m_en_fi; //  3 Enable Fundamental Images
    bool m_en_qe; //  4 Enable Quadrature Error tones
    bool m_ex_clk; //  5 Exclude CLK from Noise
    bool m_ex_dc; //  6 Exclude DC from distortion and LS
    bool m_ex_il; //  7 Exclude IL from Noise

    /// @}

private:
    // Parameters
    str_t m_ab_center; //  8 Analysis Band center
    str_t m_ab_width; //  9 Analysis Band width
    str_t m_fdata; // 10 Data rate
    str_t m_fsample; // 11 Sample rate
    str_t m_fshift; // 12 Shift (translation) frequency
    int m_hd; // 13 Order of harmonic distortion
    int m_imd; // 14 Order of intermodulation distortion
    int m_ssb_auto; // 15 SSB for auto-generated tones
    int m_ssb_dc; // 16 SSB for DC
    int m_ssb_wo; // 17 SSB for WO
    int m_wo; // 18 Number of worst others
    std::vector<int> m_clk; // 19 Clock sub-harmonic divisors
    std::vector<int> m_il; // 20 Interleaving factors

    // User-defined Components and Variables
    str_vector m_user_keys;
    comp_map m_user_comps;
    std::map<str_t, real_t> m_user_vars;

}; // class fft_analysis2

} // namespace analysis

#endif // ICD_ANALYSIS_FFT_ANALYSIS2_HPP
