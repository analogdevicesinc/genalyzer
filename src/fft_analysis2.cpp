/*
File        : $HeadURL: https://swdev.cld.analog.com/svn/projects/icdev/sandbox/users/pderouni/platform-upgrades/src/analysis/fft_analysis2.cpp $
Originator  : pderouni
Revision    : $Revision: 12767 $
Last Commit : $Date: 2020-08-24 16:38:41 -0400 (Mon, 24 Aug 2020) $
Last Editor : $Author: pderouni $
*/

#include "fft_analysis2.hpp"
#include "checks.hpp"
#include "constants.hpp"
#include "dft.hpp"
#include "enum_maps.hpp"
#include "fft_component.hpp"
#include "formatted_data.hpp"
#include "io_map.hpp"
#include "io_vector.hpp"
#include "math_expression.hpp"
#include "sparse_array_mask.hpp"
#include "text_utils.hpp"
#include "var_vector.hpp"
#include <algorithm>
#include <numeric>

namespace analysis {

/*
     * Message/Notice
     * No free bins / Not enough free bins
     * Deviation
     * Collisions...
     * where does collision information go?  maybe comp_results
     * total number of components?
     * number of collisions?
     * general messages?
     *
     * todo: life might be much easier if sparse_array_mask behavior depended
     * on real vs complex.  If real, no wrap; if complex, ranges would wrap
     * automatically.  Whenever you revisit this, look into use of diff_t vs
     * size_t.  Also, look into allowing bins greater than size that would wrap
     * (probably depends on real or complex).  And one more, function that
     * takes a real number and determines in/out of band.
     *
     * In general, the amount of code in this file would be noticably reduced,
     * the code would less error prone and easier to read by better tailoring
     * sparse_array_mask to FFT data and analysis. => "fft_data_mask" ?
     */

namespace {

    using diff_p = std::pair<diff_t, diff_t>;

    struct fraction {
        int num;
        int den;
        str_t term;

        fraction(int n, int d, const str_t& var = "")
        {
            if (!(1 <= n && 1 <= d && n < d)) {
                throw base::exception("Invalid fraction");
            }
            int gcd = std::gcd(n, d);
            num = n / gcd;
            den = d / gcd;
            if (1 == num && !var.empty()) {
                term = var;
            } else {
                term = std::to_string(num) + var;
            }
            term += '/' + std::to_string(den);
        }
    };

    bool operator<(const fraction& lhs, const fraction& rhs)
    {
        // Based on value of fraction:
        //return lhs.num * rhs.den < lhs.den * rhs.num;

        // Fractions grouped by increasing denominator:
        if (lhs.den < rhs.den) {
            return true;
        } else {
            return (lhs.den == rhs.den) && (lhs.num < rhs.num);
        }
    }

    size_t validate_analyze_input(const real_t* magms_data,
        size_t magms_size,
        const real_t* phase_data,
        size_t phase_size,
        size_t fft_size)
    {
        if (nullptr == magms_data) {
            throw base::exception("Pointer to ms-magnitude array is NULL");
        }
        if (nullptr != phase_data) { // phase data is optional
            // if provided, array sizes must match
            if (magms_size != phase_size) {
                throw base::exception("Size of phase array not equal to "
                                      "size of ms-magnitude array: "
                    + std::to_string(phase_size) + " != "
                    + std::to_string(magms_size));
            }
        }
        size_t data_size = magms_size;
        if (data_size != fft_size) {
            data_size = fft_size / 2 + 1;
            if (data_size != magms_size) {
                throw base::exception("Data size, "
                    + std::to_string(data_size)
                    + ", and FFT size, "
                    + std::to_string(fft_size)
                    + ", are inconsistent");
            }
        }
        return data_size;
    }

    size_t get_fft_size(size_t data_size, size_t fft_size)
    {
        size_t even_size = 2 * (data_size - 1);
        size_t odd_size = 2 * data_size - 1;
        return (fft_size == odd_size) ? odd_size : even_size;
    }

    // mean-square from complex (rectangular) data
    real_vector get_ms_from_cplx(const cplx_vector& data)
    {
        size_t size = data.size();
        assert_gt0(size, "Data size");
        real_vector ms_data(size);
        for (size_t i = 0; i < size; ++i) {
            ms_data[i] = std::norm(data[i]);
        }
        return ms_data;
    }

    // mean-square from rms data
    real_vector get_ms_from_rms(const real_vector& data)
    {
        size_t size = data.size();
        assert_gt0(size, "Data size");
        real_vector ms_data(size);
        for (size_t i = 0; i < size; ++i) {
            ms_data[i] = data[i] * data[i];
        }
        return ms_data;
    }

    real_t get_dbfs(real_t ms)
    {
        return (min_ms_value < ms) ? 10 * std::log10(ms) : min_db_value;
    }

    real_t get_phase(const var_map& results,
        const cplx_vector& cplx_data,
        bool cplx,
        real_t fdata,
        real_t fshift)
    {
        real_t phase = 0.0;
        if (!cplx_data.empty() && 1 == results.as_int("nbins")) {
            auto i = static_cast<size_t>(results.as_real("center"));
            phase = std::arg(cplx_data[i]);
            if (!cplx) {
                real_t fshifted = results.as_real("freq") + fshift;
                real_t falias = fft_alias(fdata, fshifted, false);
                if (fdata <= 2 * falias) {
                    phase *= -1;
                }
            }
        }
        return phase;
    }

    // Returns the nearest bin and 'deviation' indicator.  Deviation is
    // true if the nearest bin differs from the exact number of cycles by
    // more than error.
    std::pair<diff_t, bool>
    nearest_bin(real_t freq, real_t fbin, real_t error = 1.0)
    {
        real_t exact_cycles = freq / fbin;
        real_t nb = std::floor(exact_cycles + 0.5);
        bool dev = (error / 2) < std::abs(exact_cycles - nb);
        return std::make_pair(static_cast<diff_t>(nb), dev);
    }

    str_t negate(const str_t& expr, bool neg)
    {
        return neg ? ("-(" + expr + ')') : expr;
    }

    bool is_tone(const fft_component& comp)
    {
        return FFTCompType::FixedTone == comp.type() || FFTCompType::MaxTone == comp.type();
    }

    const std::set<str_t> reserved_keys = {
        "abc", // var  : analysis band center
        "abw", // var  : analysis band width
        "co", // comp : converter offset
        "dc", // comp : DC
        "fbin", // var  : fbin
        "fdata", // var  : data rate (reflects rate conversion)
        "fs", // var  : sample rate of the quantizer
        "fshift", // var  : translation frequency
    };

    const std::regex wo_regex{ "^wo[1-9]?$" };

    const std::vector<std::regex> reserved_patterns = { wo_regex };

} // namespace anonymous

fft_analysis2::unique_ptr fft_analysis2::load(const str_t& filename)
{
    abstract_object::object_ptr obj = abstract_object::load(filename);
    if (ObjectType::FFTAnalysis != obj->object_type()) {
        throw base::exception("Not an FFTAnalysis Object");
    }
    if (abstract_object::is_fft_analysis_v1(filename)) {
        throw base::exception("Expected a Version 2 FFTAnalysis Object; "
                              "got Version 1");
    }
    return unique_ptr(static_cast<fft_analysis2*>(obj.release()));
}

var_map fft_analysis2::annotations(const var_map& results,
    FFTAxisFormat fmt)
{
    validate_results(results);
    const var_map& tones = results.as_map("Tones");
    const var_map& metrics = results.as_map("Metrics");
    bool cplx = metrics.as_str("signal_type") == "complex";
    bool shift = metrics.as_bool("axis_shift");
    real_t fbin = metrics.as_real("fbin");
    real_t fdata = metrics.as_real("fdata");
    real_t fshift = metrics.as_real("fshift");
    var_vector::unique_ptr labels = var_vector::create();
    var_vector::unique_ptr lines = var_vector::create();
    var_vector::unique_ptr ab_boxes = var_vector::create();
    var_vector::unique_ptr tone_boxes = var_vector::create();
    // Tone labels and boxes
    for (const str_t& k : tones) {
        const var_map& t = tones.as_map(k);
        if ("co" == k && 0.0 == fshift) {
            continue;
        }
        auto left = static_cast<real_t>(t.as_int("i1"));
        auto right = static_cast<real_t>(t.as_int("i2"));
        if (left == right) {
            continue;
        }
        left = (left - 0.5) * fbin;
        right = (right - 0.5) * fbin;
        real_t abs_left = 0.0;
        real_t abs_right = fdata / 2;
        if (cplx) {
            left = fft_alias(fdata, left, shift);
            right = fft_alias(fdata, right, shift);
            if (shift) {
                abs_left = -fdata / 2;
            } else {
                abs_right = fdata;
            }
        } else {
            left = std::fmax(abs_left, left);
            right = std::fmin(right, abs_right);
        }
        real_t x = t.as_real("ffinal");
        real_t y = t.as_real("mag");
        if (FFTAxisFormat::Bin == fmt) {
            x /= fbin;
            left /= fbin;
            right /= fbin;
            abs_left /= fbin;
            abs_right /= fbin;
        } else if (FFTAxisFormat::Norm == fmt) {
            x /= fdata;
            left /= fdata;
            right /= fdata;
            abs_left /= fdata;
            abs_right /= fdata;
        }
        var_vector::unique_ptr label = var_vector::create();
        label->push_back(x);
        label->push_back(y);
        label->push_back(k);
        labels->push_back(std::move(label));
        var_vector::unique_ptr box = var_vector::create();
        if (left < right) {
            box->push_back(left);
            box->push_back(y);
            box->push_back(right - left);
            box->push_back(y - min_db_value);
            tone_boxes->push_back(std::move(box));
        } else { // wrap -> two boxes
            box->push_back(left);
            box->push_back(y);
            box->push_back(abs_right - left);
            box->push_back(y - min_db_value);
            tone_boxes->push_back(std::move(box));
            box = var_vector::create();
            box->push_back(abs_left);
            box->push_back(y);
            box->push_back(right - abs_left);
            box->push_back(y - min_db_value);
            tone_boxes->push_back(std::move(box));
        }
    }
    // ABN line
    real_t abn = metrics.as_real("ABN");
    real_t x1 = 0.0;
    real_t x2 = fdata / 2;
    if (cplx) {
        if (shift) {
            x1 = -fdata / 2;
        } else {
            x2 = fdata;
        }
    }
    var_vector::unique_ptr line = var_vector::create();
    line->push_back(x1);
    line->push_back(abn);
    line->push_back(x2);
    line->push_back(abn);
    lines->push_back(std::move(line));
    // Analysis Band boxes
    if (metrics.as_int("ab_size") < metrics.as_int("data_size")) {
        auto left = static_cast<real_t>(metrics.as_int("ab_i1"));
        auto right = static_cast<real_t>(metrics.as_int("ab_i2"));
        real_t abs_left = 0.0;
        real_t abs_right = fdata / 2;
        if (cplx) {
            // swap L/R to get the boundaries of the masked-off span
            real_t ltmp = left;
            left = fft_alias(fdata, (right - 0.5) * fbin, shift);
            right = fft_alias(fdata, (ltmp - 0.5) * fbin, shift);
            abs_left = shift ? -fdata / 2 : 0.0;
            abs_right = shift ? fdata / 2 : fdata;
        } else {
            left = (left - 0.5) * fbin;
            right = (right - 0.5) * fbin;
        }
        if (FFTAxisFormat::Bin == fmt) {
            left /= fbin;
            right /= fbin;
            abs_left /= fbin;
            abs_right /= fbin;
        } else if (FFTAxisFormat::Norm == fmt) {
            left /= fdata;
            right /= fdata;
            abs_left /= fdata;
            abs_right /= fdata;
        }
        if (cplx) {
            var_vector::unique_ptr box = var_vector::create();
            if (left < right) {
                box->push_back(left);
                box->push_back(0.0);
                box->push_back(right - left);
                box->push_back(-min_db_value);
                ab_boxes->push_back(std::move(box));
            } else { // wrap -> two boxes
                box->push_back(left);
                box->push_back(0.0);
                box->push_back(abs_right - left);
                box->push_back(-min_db_value);
                ab_boxes->push_back(std::move(box));
                box = var_vector::create();
                box->push_back(abs_left);
                box->push_back(0.0);
                box->push_back(right - abs_left);
                box->push_back(-min_db_value);
                ab_boxes->push_back(std::move(box));
            }
        } else {
            if (abs_left < left) {
                var_vector::unique_ptr box = var_vector::create();
                box->push_back(abs_left);
                box->push_back(0.0);
                box->push_back(left);
                box->push_back(-min_db_value);
                ab_boxes->push_back(std::move(box));
            }
            if (right < abs_right) {
                var_vector::unique_ptr box = var_vector::create();
                box->push_back(right);
                box->push_back(0.0);
                box->push_back(abs_right - right);
                box->push_back(-min_db_value);
                ab_boxes->push_back(std::move(box));
            }
        }
    }
    var_map annots;
    annots.insert("labels", std::move(labels));
    annots.insert("lines", std::move(lines));
    annots.insert("ab_boxes", std::move(ab_boxes));
    annots.insert("tone_boxes", std::move(tone_boxes));
    return annots;
}

std::vector<str_vector> fft_analysis2::result_data(const var_map& results,
    const var_vector& spec)
{
    using namespace utils;
    validate_results(results);
    const var_map& tones = results.as_map("Tones");
    const var_map& metrics = results.as_map("Metrics");
    std::vector<str_vector> rdata;
    if (spec.empty()) {
        real_t fdata = metrics.as_real("fdata");
        real_t fsample = metrics.as_real("fsample");
        real_t fshift = metrics.as_real("fshift");
        add_result(rdata, metrics, "fft_size", "FFT Size");
        add_result(rdata, metrics, "fsample", "Sample Rate");
        if (fdata != fsample) {
            add_result(rdata, metrics, "fdata", "Data Rate");
        }
        if (0.0 != fshift) {
            add_result(rdata, metrics, "fshift", "Shift Freq");
        }
        add_result(rdata, metrics, "FSNR");
        add_result(rdata, metrics, "SNR");
        add_result(rdata, metrics, "SINAD");
        add_result(rdata, metrics, "NSD");
        add_result(rdata, metrics, "ABN");
        add_result(rdata, metrics, "SFDR");
        add_result(rdata, metrics, "carrier_key", "Carrier");
        add_result(rdata, metrics, "maxspur_key", "MaxSpur");
        for (const str_t& k : tones) {
            const var_map& t = tones.as_map(k);
            const str_t& tag_str = t.as_str(fft_comp_tag_map.enum_name());
            FFTCompTag tag = fft_comp_tag_map[tag_str];
            if (FFTCompTag::Signal == tag || std::regex_match(k, wo_regex)) {
                add_result(rdata, t, "ffinal", k + " Freq");
            }
            if (FFTCompTag::HD == tag || FFTCompTag::IMD == tag) {
                add_result(rdata, t, "mag_dbc", k + " Mag");
            } else {
                if ("co" == k && 0.0 == fshift) {
                    continue;
                }
                add_result(rdata, t, "mag", k + " Mag");
            }
        }
    } else {
        for (diff_t i = 0; i < spec.size(); ++i) {
            VarType vt = spec.type(i);
            if (VarType::Vector != vt) {
                throw base::exception("Expected "
                    + var_type_map.at(VarType::Vector)
                    + " for result spec; got "
                    + var_type_map.at(vt));
            }
            add_result(rdata, tones, metrics, spec.as_vector(i));
        }
    }
    return rdata;
}

bool fft_analysis2::is_reserved(const str_t& key)
{
    if (reserved_keys.find(key) != reserved_keys.end()) {
        return true;
    }
    for (auto& pat : reserved_patterns) {
        if (std::regex_match(key, pat)) {
            return true;
        }
    }
    return false;
}

} // namespace analysis

namespace analysis { // Constructors, Destructor & Assignment

fft_analysis2::fft_analysis2()
    : abstract_object{}
    ,
    // Parameters
    m_axis_shift(true)
    , m_en_co(true)
    , m_en_fi(true)
    , m_en_qe(false)
    , m_ex_clk(true)
    , m_ex_dc(true)
    , m_ex_il(true)
    , m_ab_center("0")
    , m_ab_width("fdata")
    , m_fdata("fs")
    , m_fsample("1")
    , m_fshift("0")
    , m_hd(6)
    , m_imd(3)
    , m_ssb_auto(0)
    , m_ssb_dc(-1)
    , m_ssb_wo(-1)
    , m_wo(1)
    , m_clk()
    , m_il()
    ,
    // Components
    m_user_keys()
    , m_user_comps()
    ,
    // Variables
    m_user_vars()
{
}

fft_analysis2::fft_analysis2(const fft_analysis2& obj)
    : abstract_object{}
    ,
    // Parameters
    m_axis_shift(obj.m_axis_shift)
    , m_en_co(obj.m_en_co)
    , m_en_fi(obj.m_en_fi)
    , m_en_qe(obj.m_en_qe)
    , m_ex_clk(obj.m_ex_clk)
    , m_ex_dc(obj.m_ex_dc)
    , m_ex_il(obj.m_ex_il)
    , m_ab_center(obj.m_ab_center)
    , m_ab_width(obj.m_ab_width)
    , m_fdata(obj.m_fdata)
    , m_fsample(obj.m_fsample)
    , m_fshift(obj.m_fshift)
    , m_hd(obj.m_hd)
    , m_imd(obj.m_imd)
    , m_ssb_auto(obj.m_ssb_auto)
    , m_ssb_dc(obj.m_ssb_dc)
    , m_ssb_wo(obj.m_ssb_wo)
    , m_wo(obj.m_wo)
    , m_clk(obj.m_clk)
    , m_il(obj.m_il)
    ,
    // Components
    m_user_keys(obj.m_user_keys)
    , m_user_comps()
    ,
    // Variables
    m_user_vars(obj.m_user_vars)
{
    for (const auto& k : m_user_keys) {
        m_user_comps[k] = obj.m_user_comps.at(k)->clone();
    }
}

fft_analysis2::fft_analysis2(fft_analysis2&& obj)
    : abstract_object{}
    ,
    // Parameters
    m_axis_shift(obj.m_axis_shift)
    , m_en_co(obj.m_en_co)
    , m_en_fi(obj.m_en_fi)
    , m_en_qe(obj.m_en_qe)
    , m_ex_clk(obj.m_ex_clk)
    , m_ex_dc(obj.m_ex_dc)
    , m_ex_il(obj.m_ex_il)
    , m_ab_center(std::move(obj.m_ab_center))
    , m_ab_width(std::move(obj.m_ab_width))
    , m_fdata(std::move(obj.m_fdata))
    , m_fsample(std::move(obj.m_fsample))
    , m_fshift(std::move(obj.m_fshift))
    , m_hd(obj.m_hd)
    , m_imd(obj.m_imd)
    , m_ssb_auto(obj.m_ssb_auto)
    , m_ssb_dc(obj.m_ssb_dc)
    , m_ssb_wo(obj.m_ssb_wo)
    , m_wo(obj.m_wo)
    , m_clk(std::move(obj.m_clk))
    , m_il(std::move(obj.m_il))
    ,
    // Components
    m_user_keys(std::move(obj.m_user_keys))
    , m_user_comps(std::move(obj.m_user_comps))
    ,
    // Variables
    m_user_vars(std::move(obj.m_user_vars))
{
}

fft_analysis2::~fft_analysis2() = default;

fft_analysis2& fft_analysis2::operator=(fft_analysis2&& obj)
{
    // Parameters
    std::swap(m_axis_shift, obj.m_axis_shift);
    std::swap(m_en_co, obj.m_en_co);
    std::swap(m_en_fi, obj.m_en_fi);
    std::swap(m_en_qe, obj.m_en_qe);
    std::swap(m_ex_clk, obj.m_ex_clk);
    std::swap(m_ex_dc, obj.m_ex_dc);
    std::swap(m_ex_il, obj.m_ex_il);
    std::swap(m_ab_center, obj.m_ab_center);
    std::swap(m_ab_width, obj.m_ab_width);
    std::swap(m_fdata, obj.m_fdata);
    std::swap(m_fsample, obj.m_fsample);
    std::swap(m_fshift, obj.m_fshift);
    std::swap(m_hd, obj.m_hd);
    std::swap(m_imd, obj.m_imd);
    std::swap(m_ssb_auto, obj.m_ssb_auto);
    std::swap(m_ssb_dc, obj.m_ssb_dc);
    std::swap(m_ssb_wo, obj.m_ssb_wo);
    std::swap(m_wo, obj.m_wo);
    std::swap(m_clk, obj.m_clk);
    std::swap(m_il, obj.m_il);
    // Components
    std::swap(m_user_keys, obj.m_user_keys);
    std::swap(m_user_comps, obj.m_user_comps);
    // Variables
    std::swap(m_user_vars, obj.m_user_vars);
    return *this;
}

} // namespace analysis - Constructors, Destructor & Assignment

namespace analysis { // Analysis

namespace { // Helpers for analyze

    std::map<str_t, sparse_array_mask> initialize_masks(size_t size)
    {
        auto size2 = static_cast<diff_t>(size);
        std::map<str_t, sparse_array_mask> masks{
            { "comp", sparse_array_mask(size2) },
            { "non_ab", sparse_array_mask(size2) },
            { "non_nad", sparse_array_mask(size2) },
            { "non_noise", sparse_array_mask(size2) },
            { "td", sparse_array_mask(size2) },
            { "thd", sparse_array_mask(size2) },
            { "til", sparse_array_mask(size2) }
        };
        for (FFTCompTag tag : fft_comp_tag_map) {
            if (FFTCompTag::Noise == tag) {
                continue;
            }
            masks.emplace(fft_comp_tag_map[tag], size2);
        }
        return masks;
    }

    void finalize_masks(std::map<str_t, sparse_array_mask>& masks,
        bool ex_dc,
        bool ex_il,
        bool ex_clk)
    {
        auto& signal_mask = masks.at(fft_comp_tag_map[FFTCompTag::Signal]);
        auto& dc_mask = masks.at(fft_comp_tag_map[FFTCompTag::DC]);
        auto& non_nad_mask = masks.at("non_nad");
        auto& td_mask = masks.at("td");
        // Non-NAD mask: AB, Signal, and optionally DC bins
        non_nad_mask = masks.at("non_ab");
        non_nad_mask |= signal_mask;
        if (ex_dc) {
            non_nad_mask |= dc_mask;
        }
        // Signal mask: remove out-of-band (non-AB) bins
        for (const diff_p& r : masks.at("non_ab").all_ranges()) {
            signal_mask.unset_range(r.first, r.second);
        }
        // Since the Non-NAD mask may or may not exclude DC, create another
        // mask that is sure to exclude DC
        sparse_array_mask non_nad_dc_mask = non_nad_mask | dc_mask;
        // For the following masks, remove non-NAD-DC bins
        std::vector<FFTCompTag> tags = { FFTCompTag::HD,
            FFTCompTag::IMD,
            FFTCompTag::Dist,
            FFTCompTag::ILOS,
            FFTCompTag::ILGT,
            FFTCompTag::CLK };
        for (FFTCompTag tag : tags) {
            auto& m = masks.at(fft_comp_tag_map[tag]);
            for (const diff_p& r : non_nad_dc_mask.all_ranges()) {
                m.unset_range(r.first, r.second);
            }
        }
        // Composite masks:
        masks.at("thd") = masks.at(fft_comp_tag_map[FFTCompTag::HD])
            | masks.at(fft_comp_tag_map[FFTCompTag::IMD]);
        masks.at("til") = masks.at(fft_comp_tag_map[FFTCompTag::ILOS])
            | masks.at(fft_comp_tag_map[FFTCompTag::ILGT]);
        td_mask = masks.at("thd")
            | masks.at(fft_comp_tag_map[FFTCompTag::Dist]);
        if (!ex_dc) { // "if DC is not excluded from distortion"
            td_mask |= dc_mask;
        }
        if (ex_il) { // "if IL is excluded from noise"
            td_mask |= masks.at("til");
        }
        if (ex_clk) { // "if CLK is excluded from noise"
            td_mask |= masks.at(fft_comp_tag_map[FFTCompTag::CLK]);
        }
        // Non-Noise mask:
        // note: DC will be excluded from Noise by one or the other mask
        masks.at("non_noise") = non_nad_mask | td_mask;
    }

    void update_carrier(str_t& carrier_key,
        real_t& carrier_mag,
        const str_t& key,
        real_t mag)
    {
        if (carrier_key.empty() || mag < carrier_mag) {
            carrier_key = key;
            carrier_mag = mag;
        }
    }

} // namespace anonymous - Helpers for analyze

var_map::unique_ptr fft_analysis2::analyze(const real_t* magms_data,
    size_t magms_size,
    const real_t* phase_data,
    size_t phase_size,
    size_t fft_size) const
{
    // Input Validation //
    if (nullptr == magms_data) {
        throw base::exception("Pointer to ms-magnitude array is NULL");
    }
    // If phase data is provided, then array sizes must be the same:
    if (nullptr != phase_data && magms_size != phase_size) {
        throw base::exception("Size of phase array not equal to size of "
                              "ms-magnitude array: "
            + std::to_string(phase_size) + " != "
            + std::to_string(magms_size));
    }
    size_t data_size = magms_size;
    bool cplx = data_size == fft_size;
    if (!cplx) {
        data_size = fft_size / 2 + 1;
        if (data_size != magms_size) {
            throw base::exception("");
        }
    }
    // Setup //
    me_vars_map vars = setup_vars(fft_size);
    const real_t fsample = vars.at("fs");
    const real_t fdata = vars.at("fdata");
    const real_t fbin = vars.at("fbin");
    const real_t fshift = vars.at("fshift");
    mask_map masks = initialize_masks(data_size);
    var_map::unique_ptr results_ab = var_map::create(); // Analysis Band
    var_map::unique_ptr results_b = var_map::create(); // Bands
    var_map::unique_ptr results_t = var_map::create(); // Tones
    var_map::unique_ptr results_wo = var_map::create(); // Worst Others
    var_map::unique_ptr results_m = var_map::create(); // Metrics
    // Analysis Band //
    results_ab = setup_ab(cplx, vars, masks.at("non_ab"));
    //
    static_cast<void>(fsample);
    static_cast<void>(fdata);
    static_cast<void>(fbin);
    static_cast<void>(fshift);
    // Results //
    var_map::unique_ptr results = var_map::create();
    results->insert(analysis_type_map.enum_name(),
        analysis_type_map[AnalysisType::FFT]);
    results->insert("Bands", std::move(results_b));
    results->insert("Tones", std::move(results_t));
    results->insert("Metrics", std::move(results_m));
    return results;
}

} // namespace analysis - Analysis

namespace analysis { // Analysis

var_map::unique_ptr
fft_analysis2::analyze_fft(const cplx_vector& data) const
{
    return analyze(get_ms_from_cplx(data), true, data.size(), data);
}

var_map::unique_ptr
fft_analysis2::analyze_fft(const real_vector& data) const
{
    return analyze(get_ms_from_rms(data), true, data.size());
}

var_map::unique_ptr
fft_analysis2::analyze_rfft(const cplx_vector& data, size_t fft_size) const
{
    return analyze(get_ms_from_cplx(data),
        false,
        get_fft_size(data.size(), fft_size),
        data);
}

var_map::unique_ptr
fft_analysis2::analyze_rfft(const real_vector& data, size_t fft_size) const
{
    return analyze(get_ms_from_rms(data),
        false,
        get_fft_size(data.size(), fft_size));
}

} // namespace analysis - Analysis

namespace analysis { // Component Definition

void fft_analysis2::add_band(const str_t& key,
    FFTCompTag tag,
    const str_t& center,
    const str_t& width,
    bool keep_out,
    bool omit_tones)
{
    if (FFTCompTag::Dist != tag && FFTCompTag::Noise != tag) {
        throw base::exception("Band must be tagged Dist or Noise");
    }
    if_key_not_available_throw(key);
    math_expression e1{ center, NumericFormat::Eng };
    math_expression e2{ width, NumericFormat::Eng };
    m_user_keys.push_back(key);
    m_user_comps[key] = fft_band::create(tag,
        e1.to_string(),
        e2.to_string(),
        keep_out,
        omit_tones);
}

void fft_analysis2::add_fixed_tone(const str_t& key,
    FFTCompTag tag,
    const str_t& freq,
    int ssb)
{
    if_key_not_available_throw(key);
    math_expression e1{ freq, NumericFormat::Eng };
    m_user_keys.push_back(key);
    m_user_comps[key] = fft_fixed_tone::create(tag, e1.to_string(), ssb);
}

void fft_analysis2::add_max_tone(const str_t& key,
    FFTCompTag tag,
    const str_t& center,
    const str_t& width,
    int ssb)
{
    if (FFTCompTag::Signal != tag && FFTCompTag::Dist != tag && FFTCompTag::Noise != tag) {
        throw base::exception("MaxTone must be tagged Fund, Dist, or Noise");
    }
    if_key_not_available_throw(key);
    math_expression e1{ center, NumericFormat::Eng };
    math_expression e2{ width, NumericFormat::Eng };
    m_user_keys.push_back(key);
    m_user_comps[key] = fft_max_tone::create(
        tag, e1.to_string(), e2.to_string(), ssb);
}

} // namespace analysis - Component Definition

namespace analysis { // Configuration

void fft_analysis2::set_analysis_band(const str_t& ab_center,
    const str_t& ab_width)
{
    math_expression abce{ ab_center, NumericFormat::Eng };
    math_expression abwe{ ab_width, NumericFormat::Eng };
    m_ab_center = abce.to_string();
    m_ab_width = abwe.to_string();
}

void fft_analysis2::set_clk(const std::vector<int>& clk)
{
    m_clk = clk;
    auto check = [](int x) { return x < 2 || 64 < x; };
    auto last = std::remove_if(m_clk.begin(), m_clk.end(), check);
    m_clk.erase(last, m_clk.end());
    std::sort(m_clk.begin(), m_clk.end());
    last = std::unique(m_clk.begin(), m_clk.end());
    m_clk.erase(last, m_clk.end());
}

void fft_analysis2::set_fdata(const str_t& expr)
{
    math_expression me{ expr, NumericFormat::Eng };
    for (const auto& v : me.vars()) {
        auto it = m_user_vars.find(v);
        if (m_user_vars.end() == it && "fs" != v) {
            throw base::exception("fdata can only depend on fsample (\"fs"
                                  "\") and user-defined variables");
        }
    }
    m_fdata = me.to_string();
}

void fft_analysis2::set_fsample(const str_t& expr)
{
    math_expression me{ expr, NumericFormat::Eng };
    for (const auto& v : me.vars()) {
        auto it = m_user_vars.find(v);
        if (m_user_vars.end() == it) {
            throw base::exception("fsample can only depend on user-"
                                  "defined variables");
        }
    }
    m_fsample = me.to_string();
}

void fft_analysis2::set_fshift(const str_t& expr)
{
    math_expression me{ expr, NumericFormat::Eng };
    for (const auto& v : me.vars()) {
        auto it = m_user_vars.find(v);
        if (m_user_vars.end() == it && "fs" != v && "fbin" != v) {
            throw base::exception("fshift can only depend on fsample (\"fs"
                                  "\"), fdata, fbin, and user-defined "
                                  "variables");
        }
    }
    m_fshift = me.to_string();
}

void fft_analysis2::set_hd(int hd)
{
    if (hd < 1 || 30 < hd) {
        throw base::exception("HD out of range [ 1, 30 ]");
    }
    m_hd = hd;
}

void fft_analysis2::set_il(const std::vector<int>& il)
{
    m_il = il;
    auto check = [](int x) { return x < 2 || 64 < x; };
    auto last = std::remove_if(m_il.begin(), m_il.end(), check);
    m_il.erase(last, m_il.end());
    std::sort(m_il.begin(), m_il.end());
    last = std::unique(m_il.begin(), m_il.end());
    m_il.erase(last, m_il.end());
}

void fft_analysis2::set_imd(int imd)
{
    if (imd < 1 || 9 < imd) {
        throw base::exception("IMD out of range [ 1, 9 ]");
    }
    m_imd = imd;
}

void fft_analysis2::set_ssb_auto(int ssb)
{
    m_ssb_auto = (ssb < 0) ? 0 : ssb;
}

void fft_analysis2::set_ssb_auto(WindowType win)
{
    switch (win) {
    case WindowType::BlackmanHarris:
        m_ssb_auto = 3;
        break;
    case WindowType::Rect:
        m_ssb_auto = 0;
        break;
    default:
        throw base::exception("Automatic SSB for "
            + window_type_map.qual_name(win)
            + " is not supported");
    }
}

void fft_analysis2::set_ssb_dc(int ssb)
{
    m_ssb_dc = (ssb < 0) ? -1 : ssb;
}

void fft_analysis2::set_ssb_wo(int ssb)
{
    m_ssb_wo = (ssb < 0) ? -1 : ssb;
}

void fft_analysis2::set_var(const str_t& key, real_t value)
{
    if (!is_var(key)) {
        if_key_not_available_throw(key);
    }
    m_user_vars[key] = value;
}

void fft_analysis2::set_wo(int wo)
{
    if (wo < 1 || 9 < wo) {
        throw base::exception("WO out of range [ 1, 9 ]");
    }
    m_wo = wo;
}

} // namespace analysis - Configuration

namespace analysis { // Other Functions

str_t fft_analysis2::comp_list(bool cplx) const
{
    comp_data keys_comps = generate_comps(cplx, 0);
    str_vector& keys = std::get<0>(keys_comps);
    comp_map& comps = std::get<1>(keys_comps);
    std::vector<str_vector> header{
        { "Order", "Key", "Type", "Tag", "Spec" }
    };
    std::vector<str_vector> data;
    int i = 1;
    for (const auto& k : keys) {
        data.emplace_back(comps.at(k)->table_row(i++, k));
    }
    return table(header, data, 2, true, true);
    //        // temporary, to see special keys:
    //        std::ostringstream oss;
    //        oss << table(header, data, 2, true, true);
    //        const std::set<str_t>& clk_keys = std::get<2>(keys_comps);
    //        oss << "\nCLK Keys: ";
    //        for (const str_t& k : clk_keys) {
    //            oss << k << ", ";
    //        }
    //        return oss.str();
}

} // namespace analysis - Other Functions

namespace analysis { // Protected and Private Functions

void fft_analysis2::save_state(io_map& state) const
{
    abstract_object::save_state(state);
    auto data = io_map::create();
    data->emplace("version", 2.0);
    // Parameters
    auto clk = io_vector::create();
    for (auto x : m_clk) {
        clk->emplace_back(static_cast<io_int_t>(x));
    }
    auto il = io_vector::create();
    for (auto x : m_il) {
        il->emplace_back(static_cast<io_int_t>(x));
    }
    auto params = io_map::create();
    params->emplace("AxisShift", m_axis_shift);
    params->emplace("EnableConvOffset", m_en_co);
    params->emplace("EnableFundImages", m_en_fi);
    params->emplace("EnableQuadErrors", m_en_qe);
    params->emplace("ExcludeCLK", m_ex_clk);
    params->emplace("ExcludeDC", m_ex_dc);
    params->emplace("ExcludeIL", m_ex_il);
    params->emplace("AnalysisBandCenter", m_ab_center);
    params->emplace("AnalysisBandWidth", m_ab_width);
    params->emplace("FData", m_fdata);
    params->emplace("FSample", m_fsample);
    params->emplace("FShift", m_fshift);
    params->emplace("HD", static_cast<io_int_t>(m_hd));
    params->emplace("IMD", static_cast<io_int_t>(m_imd));
    params->emplace("SSB_Auto", static_cast<io_int_t>(m_ssb_auto));
    params->emplace("SSB_DC", static_cast<io_int_t>(m_ssb_dc));
    params->emplace("SSB_WO", static_cast<io_int_t>(m_ssb_wo));
    params->emplace("WO", static_cast<io_int_t>(m_wo));
    params->insert("CLK", std::move(clk));
    params->insert("IL", std::move(il));
    data->insert(io_params_key, std::move(params));
    // Components
    auto comps = io_map::create();
    for (const auto& k : m_user_keys) {
        comps->insert(k, m_user_comps.at(k)->save());
    }
    data->insert(io_comps_key, std::move(comps));
    // Variables
    auto vars = io_map::create();
    for (const auto& kv : m_user_vars) {
        vars->emplace(kv.first, kv.second);
    }
    data->insert(io_vars_key, std::move(vars));
    state.insert(object_data_key, std::move(data));
}

const std::map<str_t, int> fft_analysis2::param_defaults = {
    { "def_hd", 6 },
    { "def_imd", 3 },
    { "def_wo", 1 }
};

const std::regex fft_analysis2::key_pattern{ "[[:alpha:]][[:alnum:]_]*" };

const str_t fft_analysis2::io_params_key = "Parameters";

const str_t fft_analysis2::io_comps_key = "Components";

const str_t fft_analysis2::io_vars_key = "Variables";

namespace {

    enum class ResultType : int {
        dB,
        dBFS_Hz,
        dBc,
        Freq,
        Magnitude,
        Phase,
        Unitless
    };

    const std::map<ResultType, std::set<str_t>> result_units_map{
        { ResultType::dB, { "dB" } },
        { ResultType::dBFS_Hz, { "dBFS/Hz" } },
        { ResultType::dBc, { "dBc" } },
        { ResultType::Freq, { "Hz", "kHz", "MHz", "GHz" } },
        { ResultType::Magnitude, { "dBFS", "rms" } },
        { ResultType::Phase, { "deg", "rad" } },
        { ResultType::Unitless, {} }
    };

    const std::map<str_t, ResultType> result_type_map{
        // Metrics:
        { "fsample", ResultType::Freq },
        { "fshift", ResultType::Freq },
        { "fdata", ResultType::Freq },
        { "fbin", ResultType::Freq },
        { "ab_cfreq", ResultType::Freq },
        { "ab_width", ResultType::Freq },
        { "ab_lfreq", ResultType::Freq },
        { "ab_rfreq", ResultType::Freq },
        { "FSNR", ResultType::dB },
        { "SNR", ResultType::dB },
        { "SINAD", ResultType::dB },
        { "NSD", ResultType::dBFS_Hz },
        { "ABN", ResultType::Magnitude },
        { "SFDR", ResultType::dB },
        { "signal", ResultType::Magnitude },
        { "nad", ResultType::Magnitude },
        { "noise", ResultType::Magnitude },
        { "thd", ResultType::Magnitude },
        { "til", ResultType::Magnitude },
        { "td", ResultType::Magnitude },
        { "sum_ab", ResultType::Magnitude },
        { "sum_hd", ResultType::Magnitude },
        { "sum_imd", ResultType::Magnitude },
        { "sum_dist", ResultType::Magnitude },
        { "sum_ilos", ResultType::Magnitude },
        { "sum_ilgt", ResultType::Magnitude },
        { "sum_clk", ResultType::Magnitude },
        // Tones:
        { "freq", ResultType::Freq },
        { "ffinal", ResultType::Freq },
        { "fwave", ResultType::Freq },
        { "fbin", ResultType::Freq },
        { "mag", ResultType::Magnitude },
        { "mag_dbc", ResultType::dBc },
        { "phase", ResultType::Phase }
    };

} // namespace anonymous

void fft_analysis2::add_result(std::vector<str_vector>& rows,
    const var_map& results,
    const str_t& key,
    str_t name,
    str_t units,
    int prec)
{
    if (!results.contains(key)) {
        return;
    }
    if (name.empty()) {
        name = key;
    }
    str_vector row{ name };
    switch (results.type(key)) {
    case VarType::Bool: // not convertible, no units
        row.push_back(results.as_bool(key) ? "True" : "False");
        row.push_back("");
        break;
    case VarType::Int: // not convertible, no units
        row.push_back(std::to_string(results.as_int(key)));
        row.push_back("");
        break;
    case VarType::Real:
        row.push_back(format_result(results, key, units, prec));
        row.push_back(units);
        break;
    case VarType::Str: // not convertible, no units
        row.push_back(results.as_str(key));
        row.push_back("");
        break;
    default:
        break;
    }
    rows.push_back(std::move(row));
}

void fft_analysis2::add_result(std::vector<str_vector>& rows,
    const var_map& comps,
    const var_map& metrics,
    const var_vector& row_spec)
{
    if (row_spec.empty()) {
        rows.push_back(str_vector());
        return;
    }
    VarType vt = row_spec.type(0);
    if (VarType::Str != vt) {
        throw base::exception("Expected key of type "
            + var_type_map.at(VarType::Str)
            + " as first element of row spec; got "
            + var_type_map.at(vt));
    }
    str_t key = row_spec.as_str(0);
    size_t dot_pos = key.find('.');
    bool is_metric = str_t::npos == dot_pos;
    str_t ckey = "";
    if (!is_metric) {
        ckey = key.substr(0, dot_pos);
        if (!comps.contains(ckey)) {
            return;
        }
        key = key.substr(dot_pos + 1);
    }
    const var_map& vmap = is_metric ? metrics : comps.as_map(ckey);
    if (!vmap.contains(key)) {
        if (is_metric) {
            throw base::exception("Metric key '" + key + "' not found");
        } else {
            throw base::exception("Component property key '" + key
                + "' not found");
        }
    }
    str_t name = is_metric ? key : ckey + ' ' + key;
    int prec = 3;
    str_t units = "";
    if (1 < row_spec.size()) {
        if (VarType::Map == row_spec.type(1)) {
            const var_map& npu = row_spec.as_map(1);
            if (npu.contains("n")) {
                vt = npu.type("n");
                if (VarType::Str != vt) {
                    throw base::exception("Expected "
                        + var_type_map.at(VarType::Str)
                        + " to be associated with key 'n'; got "
                        + var_type_map.at(vt));
                }
                const str_t& n_value = npu.as_str("n");
                if (!n_value.empty()) {
                    name = n_value;
                }
            }
            if (npu.contains("p")) {
                vt = npu.type("p");
                if (VarType::Int != vt) {
                    throw base::exception("Expected "
                        + var_type_map.at(VarType::Int)
                        + " to be assoicated with key 'p'; got "
                        + var_type_map.at(vt));
                }
                prec = static_cast<int>(npu.as_int("p"));
            }
            if (npu.contains("u")) {
                vt = npu.type("u");
                if (VarType::Str != vt) {
                    throw base::exception("Expected "
                        + var_type_map.at(VarType::Str)
                        + " to be associated with key 'u'; got "
                        + var_type_map.at(vt));
                }
                units = npu.as_str("u");
            }
        }
    }
    add_result(rows, vmap, key, name, units, prec);
}

str_t fft_analysis2::format_result(const var_map& results,
    const str_t& key,
    str_t& units,
    int prec)
{
    using namespace utils;
    ResultType rtype;
    if (result_type_map.find(key) == result_type_map.end()) {
        rtype = ResultType::Unitless;
    } else {
        rtype = result_type_map.at(key);
    }
    const std::set<str_t>& units_set = result_units_map.at(rtype);
    if (!units.empty() && units_set.find(units) == units_set.end()) {
        throw base::exception("Invalid units for result '" + key + '\'');
    }
    real_t value = results.as_real(key); // Assumes key exists!
    switch (rtype) {
    case ResultType::dB:
        if (units.empty()) {
            units = "dB";
        }
        break;
    case ResultType::dBFS_Hz:
        if (units.empty()) {
            units = "dBFS/Hz";
        }
        break;
    case ResultType::dBc:
        if (units.empty()) {
            units = "dBc";
        }
        break;
    case ResultType::Freq:
        if (units.empty()) {
            real_t abs_value = std::abs(value);
            if (abs_value < 1e3) {
                units = "Hz";
            } else if (abs_value < 1e6) {
                units = "kHz";
            } else if (abs_value < 1e9) {
                units = "MHz";
            } else {
                units = "GHz";
            }
        }
        if (units == "kHz") {
            value /= 1e3;
        } else if (units == "MHz") {
            value /= 1e6;
        } else if (units == "GHz") {
            value /= 1e9;
        }
        break;
    case ResultType::Magnitude:
        if (units.empty()) {
            units = "dBFS";
        } else if (units == "rms") {
            value = std::pow(10.0, value / 20.0);
            units.clear();
        }
        break;
    case ResultType::Phase:
        if (units.empty()) {
            units = "rad";
        } else if (units == "deg") {
            value *= 180.0 / k_pi;
        }
        break;
    default:
        break;
    }
    return real_to_fix(value, prec);
}

fft_analysis2::unique_ptr fft_analysis2::load_state(const io_map& state)
{
    unique_ptr obj = create();
    // Parameters
    auto& params = state.at(io_params_key).as_map();
    auto& io_clk = params.at("CLK").as_vector();
    std::vector<int> clk;
    for (const auto& x : io_clk) {
        clk.push_back(static_cast<int>(x->as_int()));
    }
    auto& io_il = params.at("IL").as_vector();
    std::vector<int> il;
    for (const auto& x : io_il) {
        il.push_back(static_cast<int>(x->as_int()));
    }
    obj->m_axis_shift = params.at("AxisShift").as_bool();
    obj->m_en_co = params.at("EnableConvOffset").as_bool();
    obj->m_en_fi = params.at("EnableFundImages").as_bool();
    obj->m_en_qe = params.at("EnableQuadErrors").as_bool();
    obj->m_ex_clk = params.at("ExcludeCLK").as_bool();
    obj->m_ex_dc = params.at("ExcludeDC").as_bool();
    obj->m_ex_il = params.at("ExcludeIL").as_bool();
    obj->set_analysis_band(params.at("AnalysisBandCenter").as_str(),
        params.at("AnalysisBandWidth").as_str());
    obj->set_fdata(params.at("FData").as_str());
    obj->set_fsample(params.at("FSample").as_str());
    obj->set_fshift(params.at("FShift").as_str());
    obj->set_hd(static_cast<int>(params.at("HD").as_int()));
    obj->set_imd(static_cast<int>(params.at("IMD").as_int()));
    obj->set_ssb_auto(static_cast<int>(params.at("SSB_Auto").as_int()));
    obj->set_ssb_dc(static_cast<int>(params.at("SSB_DC").as_int()));
    obj->set_ssb_wo(static_cast<int>(params.at("SSB_WO").as_int()));
    obj->set_wo(static_cast<int>(params.at("WO").as_int()));
    obj->set_clk(clk);
    obj->set_il(il);
    // Components
    auto& comps = state.at(io_comps_key).as_map();
    for (auto& key : comps) {
        obj->load_comp(key, comps.at(key).as_map());
    }
    // Variables
    auto& vars = state.at(io_vars_key).as_map();
    for (auto& name : vars) {
        obj->set_var(name, vars.at(name).as_real());
    }
    return obj;
}

void fft_analysis2::validate_results(const var_map& results)
{
    str_t an_type_str = analysis_type_map.enum_name();
    if (!results.contains(an_type_str) || !results.contains("Bands") || !results.contains("Tones") || !results.contains("Metrics")) {
        throw base::exception("Invalid FFT Analysis results");
    }
    if (VarType::Str != results.type(an_type_str) || VarType::Map != results.type("Bands") || VarType::Map != results.type("Tones") || VarType::Map != results.type("Metrics")) {
        throw base::exception("Invalid FFT Analysis results");
    }
    an_type_str = results.as_str(an_type_str);
    if (AnalysisType::FFT != analysis_type_map[an_type_str]) {
        throw base::exception("Invalid FFT Analysis results");
    }
    const var_map& bands = results.as_map("Bands");
    for (const str_t& k : bands) {
        if (VarType::Map != bands.type(k)) {
            throw base::exception("Invalid FFT Analysis results");
        }
    }
    const var_map& tones = results.as_map("Tones");
    for (const str_t& k : tones) {
        if (VarType::Map != tones.type(k)) {
            throw base::exception("Invalid FFT Analysis results");
        }
    }
}

bool fft_analysis2::add_comp(str_vector& keys,
    comp_map& comps,
    const str_t& key,
    comp_ptr&& comp) const
{
    auto ret = comps.emplace(key, std::move(comp));
    if (ret.second) {
        keys.push_back(key);
    }
    return ret.second;
}

var_map::unique_ptr
fft_analysis2::analyze(const real_vector& ms_data,
    bool cplx,
    const size_t fft_size,
    const cplx_vector& cplx_data) const
{
    /* Setup */
    me_vars_map vars = setup_vars(fft_size);
    const size_t data_size = ms_data.size();
    const real_t fsample = vars.at("fs");
    const real_t fdata = vars.at("fdata");
    const real_t fbin = vars.at("fbin");
    const real_t fshift = vars.at("fshift");
    mask_map masks = initialize_masks(data_size);
    var_map::unique_ptr results_ab = var_map::create(); // Analysis Band
    var_map::unique_ptr results_b = var_map::create(); // Bands
    var_map::unique_ptr results_t = var_map::create(); // Tones
    var_map::unique_ptr results_wo = var_map::create(); // Worst Others
    var_map::unique_ptr results_m = var_map::create(); // Metrics
    /* Analysis Band */
    results_ab = setup_ab(cplx, vars, masks.at("non_ab"));
    /* Component Generation */
    // CLK and ILOS components may overlap.  Since the ILOS tag has higher
    // priority, generate_comps() creates a special key list for CLK.
    comp_data keys_comps = generate_comps(cplx, fft_size);
    const str_vector& keys = std::get<0>(keys_comps);
    const comp_map& comps = std::get<1>(keys_comps);
    const std::set<str_t>& clk_keys = std::get<2>(keys_comps);
    /* Component Loop */
    str_t carrier_key = "";
    str_t maxspur_key = "";
    real_t carrier_mag = min_db_value;
    real_t maxspur_mag = min_db_value;
    str_vector::const_iterator key_iter = keys.begin();
    while (!std::regex_match(*key_iter, wo_regex)) {
        const str_t& key = *key_iter;
        const fft_component& comp = *comps.at(key);
        switch (comp.type()) {
        case FFTCompType::Band: {
            auto& c = static_cast<const fft_band&>(comp);
            auto results = meas_band(
                ms_data, cplx, fft_size, vars, masks, c);
            results_b->insert(key, std::move(results));
            break;
        }
        case FFTCompType::FixedTone: {
            auto& c = static_cast<const fft_fixed_tone&>(comp);
            auto results = meas_fixed_tone(
                ms_data, cplx, fft_size, vars, masks, c);
            results_t->insert(key, std::move(results));
            break;
        }
        case FFTCompType::MaxTone: {
            auto& c = static_cast<const fft_max_tone&>(comp);
            auto results = meas_max_tone(
                ms_data, cplx, fft_size, vars, masks, c);
            results_t->insert(key, std::move(results));
            break;
        }
        default:
            continue;
        }
        if (is_tone(comp)) {
            // Update Vars, CLK Mask, Carrier, MaxSpur
            const var_map& results = results_t->as_map(key);
            vars[key] = results.as_real("freq");
            if (clk_keys.find(key) != clk_keys.end()) {
                auto i1 = static_cast<diff_t>(results.as_int("i1"));
                auto i2 = static_cast<diff_t>(results.as_int("i2"));
                masks[fft_comp_tag_map[FFTCompTag::CLK]].set_range(i1, i2);
            }
            if (results.as_bool("inband")) {
                real_t mag = results.as_real("mag");
                if (FFTCompTag::Signal == comp.tag()) {
                    update_carrier(carrier_key, carrier_mag, key, mag);
                } else {
                    update_maxspur(maxspur_key, maxspur_mag, key, mag,
                        results, masks);
                }
            }
        }
        ++key_iter;
    }
    // Do Worst Others separately in order to guarantee max to min order
    std::map<real_t, str_t> wo_mags;
    std::map<str_t, var_map::unique_ptr> wo_tmp;
    while (key_iter != keys.end()) {
        const str_t& key = *key_iter;
        auto& c = static_cast<const fft_max_tone&>(*comps.at(key));
        var_map::unique_ptr results = meas_max_tone(
            ms_data, cplx, fft_size, vars, masks, c);
        real_t mag = results->as_real("mag");
        wo_mags[mag] = key;
        wo_tmp[key] = std::move(results);
        ++key_iter;
    }
    // Sort Worst Others and update MaxSpur
    int wo_num = 0;
    for (auto iter = wo_mags.crbegin(); iter != wo_mags.crend(); ++iter) {
        const str_t& key = iter->second;
        str_t new_key = "wo";
        if (1 < m_wo) {
            new_key += std::to_string(++wo_num);
        }
        if (1 == m_wo || 1 == wo_num) {
            real_t mag = wo_tmp.at(key)->as_real("mag");
            update_maxspur(maxspur_key, maxspur_mag, new_key, mag,
                *wo_tmp.at(key), masks);
        }
        results_t->insert(new_key, std::move(wo_tmp.at(key)));
    }
    // Second pass for dBc and phase
    for (const str_t& key : keys) {
        const fft_component& comp = *comps.at(key);
        if (is_tone(comp)) {
            var_data::map_ptr res = results_t->as_shared_map(key);
            real_t mag = res->as_real("mag");
            real_t phase = get_phase(*res, cplx_data, cplx, fdata, fshift);
            res->insert("mag_dbc", mag - carrier_mag);
            res->insert("phase", phase);
        }
    }
    /* Metrics */
    finalize_masks(masks, m_ex_dc, m_ex_il, m_ex_clk);
    std::map<str_t, real_t> sums_ms;
    std::map<str_t, real_t> sums_dbfs;
    size_t noise_nbins = 0;
    for (const auto& kv : masks) {
        const str_t& name = kv.first;
        const sparse_array_mask& mask = kv.second;
        real_t sum = 0.0;
        if ("non_ab" == name) {
            sum = mask.sum_open_ranges(ms_data).first;
            sums_ms["ab"] = sum;
            sums_dbfs["ab"] = get_dbfs(sum);
        } else if ("non_nad" == name) {
            sum = mask.sum_open_ranges(ms_data).first;
            sums_ms["nad"] = sum;
            sums_dbfs["nad"] = get_dbfs(sum);
        } else if ("non_noise" == name) {
            auto sum_num = mask.sum_open_ranges(ms_data);
            sums_ms["noise"] = sum_num.first;
            sums_dbfs["noise"] = get_dbfs(sum_num.first);
            noise_nbins = sum_num.second;
        } else {
            sum = mask.sum_ranges(ms_data).first;
            sums_ms[name] = sum;
            sums_dbfs[name] = get_dbfs(sum);
        }
    }
    real_t signal_dbfs = sums_dbfs.at(fft_comp_tag_map[FFTCompTag::Signal]);
    real_t nad_dbfs = sums_dbfs.at("nad");
    real_t noise_ms = sums_ms.at("noise");
    real_t noise_dbfs = sums_dbfs.at("noise");
    real_t abn_dbfs = get_dbfs(noise_ms / noise_nbins);
    real_t ab_width = results_ab->as_real("width");
    real_t nsd = get_dbfs(noise_ms / ab_width);
    // metadata
    results_m->insert("signal_type", (cplx ? "complex" : "real"));
    results_m->insert("axis_shift", cplx && m_axis_shift);
    results_m->insert("fft_size", static_cast<io_int_t>(fft_size));
    results_m->insert("data_size", static_cast<io_int_t>(data_size));
    results_m->insert("ab_size", results_ab->as_int("nbins"));
    results_m->insert("noise_bins", static_cast<io_int_t>(noise_nbins));
    // key variables
    results_m->insert("fsample", fsample);
    results_m->insert("fshift", fshift);
    results_m->insert("fdata", fdata);
    results_m->insert("fbin", fbin);
    // analysis band parameters
    results_m->insert("ab_cfreq", results_ab->as_real("cfreq"));
    results_m->insert("ab_width", results_ab->as_real("width"));
    results_m->insert("ab_lfreq", results_ab->as_real("lfreq"));
    results_m->insert("ab_rfreq", results_ab->as_real("rfreq"));
    results_m->insert("ab_i1", results_ab->as_int("lindex"));
    results_m->insert("ab_i2", results_ab->as_int("rindex"));
    // primary metrics
    results_m->insert("FSNR", -noise_dbfs);
    results_m->insert("SNR", signal_dbfs - noise_dbfs);
    results_m->insert("SINAD", signal_dbfs - nad_dbfs);
    results_m->insert("NSD", nsd);
    results_m->insert("ABN", abn_dbfs);
    results_m->insert("SFDR", carrier_mag - maxspur_mag);
    results_m->insert("carrier_key", carrier_key);
    results_m->insert("maxspur_key", maxspur_key);
    //
    results_m->insert("signal", signal_dbfs);
    results_m->insert("nad", nad_dbfs);
    results_m->insert("noise", noise_dbfs);
    results_m->insert("thd", sums_dbfs.at("thd"));
    results_m->insert("til", sums_dbfs.at("til"));
    results_m->insert("td", sums_dbfs.at("td"));
    // tag root-sum-squares
    results_m->insert("rss_ab", sums_dbfs.at("ab"));
    results_m->insert("rss_hd", sums_dbfs.at(fft_comp_tag_map[FFTCompTag::HD]));
    results_m->insert("rss_imd", sums_dbfs.at(fft_comp_tag_map[FFTCompTag::IMD]));
    results_m->insert("rss_dist", sums_dbfs.at(fft_comp_tag_map[FFTCompTag::Dist]));
    results_m->insert("rss_ilos", sums_dbfs.at(fft_comp_tag_map[FFTCompTag::ILOS]));
    results_m->insert("rss_ilgt", sums_dbfs.at(fft_comp_tag_map[FFTCompTag::ILGT]));
    results_m->insert("rss_clk", sums_dbfs.at(fft_comp_tag_map[FFTCompTag::CLK]));
    /* Results */
    var_map::unique_ptr results = var_map::create();
    results->insert(analysis_type_map.enum_name(),
        analysis_type_map[AnalysisType::FFT]);
    results->insert("Bands", std::move(results_b));
    results->insert("Tones", std::move(results_t));
    results->insert("Metrics", std::move(results_m));
    return results;
}

fft_analysis2::comp_data fft_analysis2::generate_comps(
    bool cplx, size_t fft_size) const
{
    bool find_fi = cplx && m_en_fi;
    bool find_qe = cplx && m_en_qe;
    comp_data keys_comps;
    str_vector& keys = std::get<0>(keys_comps);
    comp_map& comps = std::get<1>(keys_comps);
    std::set<str_t>& clk_keys = std::get<2>(keys_comps);
    int ssb_max = std::numeric_limits<int>::max();
    if (0 < fft_size) {
        // In execution (in contrast to preview) SSB is limited to half the
        // FFT size minus one.
        ssb_max = std::max<int>(0, static_cast<int>(fft_size) / 2 - 1);
    }
    int ssb_auto = std::min(m_ssb_auto, ssb_max);
    str_t key;
    int ssb;
    // DC
    ssb = (m_ssb_dc < 0) ? ssb_auto : m_ssb_dc;
    ssb = std::min(ssb, ssb_max);
    add_comp(keys, comps, "dc", fft_fixed_tone::create(FFTCompTag::DC, "-fshift", ssb));
    // CO (Converter Offset)
    if (m_en_co) {
        add_comp(keys, comps, "co", fft_fixed_tone::create(FFTCompTag::Dist, "0", ssb_auto));
    }
    // ILOS (Interleaving Offset)
    std::set<fraction> il_terms;
    for (int x : m_il) {
        for (int i = 1; i <= x / 2; ++i) {
            il_terms.emplace(i, x, "fs");
        }
    }
    for (const fraction& f : il_terms) {
        key = f.term;
        add_comp(keys, comps, key, fft_fixed_tone::create(FFTCompTag::ILOS, key, ssb_auto));
        if (cplx && "fs/2" != key) {
            key.insert(0, 1, '-');
            add_comp(keys, comps, key, fft_fixed_tone::create(FFTCompTag::ILOS, key, ssb_auto));
        }
    }
    // CLK
    std::set<fraction> clk_terms;
    for (int x : m_clk) {
        for (int i = 1; i <= x / 2; ++i) {
            if (1 == std::gcd(i, x)) {
                clk_terms.emplace(i, x, "fs");
            }
        }
    }
    for (const fraction& f : clk_terms) {
        key = f.term;
        add_comp(keys, comps, key, fft_fixed_tone::create(FFTCompTag::CLK, key, ssb_auto));
        clk_keys.insert(key);
        if (cplx && "fs/2" != f.term) {
            key.insert(0, 1, '-');
            add_comp(keys, comps, key, fft_fixed_tone::create(FFTCompTag::CLK, key, ssb_auto));
            clk_keys.insert(key);
        }
    }
    clk_terms.clear();
    // User and Auto Components
    str_vector fund_keys;
    for (const auto& ukey : m_user_keys) { // ukey: "User Key"
        const comp_ptr& comp = m_user_comps.at(ukey);
        if (comp->type() == FFTCompType::FixedTone) {
            const auto& c = static_cast<const fft_fixed_tone&>(*comp);
            ssb = (c.ssb() < 0) ? ssb_auto : c.ssb();
            ssb = std::min(ssb, ssb_max);
            add_comp(keys, comps, ukey, fft_fixed_tone::create(c.tag(), c.freq(), ssb));
        } else if (comp->type() == FFTCompType::MaxTone) {
            const auto& c = static_cast<const fft_max_tone&>(*comp);
            ssb = (c.ssb() < 0) ? ssb_auto : c.ssb();
            ssb = std::min(ssb, ssb_max);
            add_comp(keys, comps, ukey, fft_max_tone::create(c.tag(), c.center(), c.width(), ssb));
        } else {
            add_comp(keys, comps, ukey, comp->clone());
            continue;
        }
        if (FFTCompTag::Signal != comp->tag()) {
            continue;
        }
        fund_keys.push_back(ukey);
        // FI
        if (find_fi) {
            key = '-' + ukey;
            add_comp(keys, comps, key, fft_fixed_tone::create(FFTCompTag::HD, key, ssb_auto));
        }
        // HD
        for (int i = 2; i <= m_hd; ++i) {
            if (0 == i % 2) {
                // even order
                key = std::to_string(i) + ukey;
                add_comp(keys, comps, key, fft_fixed_tone::create(FFTCompTag::HD, key, ssb_auto));
                if (cplx) {
                    key.insert(0, 1, '-');
                    add_comp(keys, comps, key, fft_fixed_tone::create(FFTCompTag::HD, key, ssb_auto));
                }
            } else {
                // odd order
                // real data: j = +i
                // cplx data: j = -i for 3, 7, 11...
                //            j = +i for 5, 9, 13...
                int j = (cplx && (1 == (i / 2) % 2)) ? -i : i;
                key = std::to_string(j) + ukey;
                add_comp(keys, comps, key, fft_fixed_tone::create(FFTCompTag::HD, key, ssb_auto));
                if (find_qe) {
                    key = std::to_string(-j) + ukey;
                    add_comp(keys, comps, key, fft_fixed_tone::create(FFTCompTag::HD, key, ssb_auto));
                }
            }
        }
        // ILGT
        for (const fraction& f : il_terms) {
            key = ukey + '+' + f.term;
            add_comp(keys, comps, key, fft_fixed_tone::create(FFTCompTag::ILGT, key, ssb_auto));
            if ("fs/2" != f.term) {
                key = ukey + '-' + f.term;
                add_comp(keys, comps, key, fft_fixed_tone::create(FFTCompTag::ILGT, key, ssb_auto));
            }
            if (find_qe) {
                key = negate(ukey + '+' + f.term, true);
                add_comp(keys, comps, key, fft_fixed_tone::create(FFTCompTag::ILGT, key, ssb_auto));
                if ("fs/2" != f.term) {
                    key = negate(ukey + '-' + f.term, true);
                    add_comp(keys, comps, key, fft_fixed_tone::create(FFTCompTag::ILGT, key, ssb_auto));
                }
            }
        }
        // IMD
        for (const str_t& ka : fund_keys) {
            if (ka == ukey) {
                continue;
            }
            const str_t& kb = ukey;
            for (int order = 2; order <= m_imd; ++order) {
                int group = (0 == order % 2) ? 0 : 1;
                int pp, qq;
                str_t p, q;
                for (; group <= order; group += 2) {
                    if (group < order) {
                        // form: p * kx MINUS q * ky
                        pp = (order + group) / 2;
                        qq = order - pp;
                        p = (1 == pp) ? "" : std::to_string(pp);
                        q = (1 == qq) ? "" : std::to_string(qq);
                        if (0 == group) {
                            key = p + kb + '-' + q + ka;
                            add_comp(keys, comps, key, fft_fixed_tone::create(FFTCompTag::IMD, key, ssb_auto));
                            if (cplx) {
                                key = negate(key, true);
                                add_comp(keys, comps, key, fft_fixed_tone::create(FFTCompTag::IMD, key, ssb_auto));
                            }
                        } else if (0 == order % 2) {
                            // even order
                            key = p + ka + '-' + q + kb;
                            add_comp(keys, comps, key, fft_fixed_tone::create(FFTCompTag::IMD, key, ssb_auto));
                            key = p + kb + '-' + q + ka;
                            add_comp(keys, comps, key, fft_fixed_tone::create(FFTCompTag::IMD, key, ssb_auto));
                            if (cplx) {
                                key = negate(p + ka + '-' + q + kb, true);
                                add_comp(keys, comps, key, fft_fixed_tone::create(FFTCompTag::IMD, key, ssb_auto));
                                key = negate(p + kb + '-' + q + ka, true);
                                add_comp(keys, comps, key, fft_fixed_tone::create(FFTCompTag::IMD, key, ssb_auto));
                            }
                        } else {
                            // odd order
                            bool neg = cplx && (1 == (group / 2) % 2);
                            key = negate(p + ka + '-' + q + kb, neg);
                            add_comp(keys, comps, key, fft_fixed_tone::create(FFTCompTag::IMD, key, ssb_auto));
                            key = negate(p + kb + '-' + q + ka, neg);
                            add_comp(keys, comps, key, fft_fixed_tone::create(FFTCompTag::IMD, key, ssb_auto));
                            if (find_qe) {
                                key = negate(p + ka + '-' + q + kb, !neg);
                                add_comp(keys, comps, key, fft_fixed_tone::create(FFTCompTag::IMD, key, ssb_auto));
                                key = negate(p + kb + '-' + q + ka, !neg);
                                add_comp(keys, comps, key, fft_fixed_tone::create(FFTCompTag::IMD, key, ssb_auto));
                            }
                        }
                    } else { // group == order
                        // form: p * kx PLUS q * ky
                        if (0 == order % 2) {
                            // even order
                            for (qq = 1; qq < group; ++qq) {
                                pp = order - qq;
                                p = (1 == pp) ? "" : std::to_string(pp);
                                q = (1 == qq) ? "" : std::to_string(qq);
                                key = p + ka + '+' + q + kb;
                                add_comp(keys, comps, key, fft_fixed_tone::create(FFTCompTag::IMD, key, ssb_auto));
                            }
                            if (cplx) {
                                for (pp = 1; pp < group; ++pp) {
                                    qq = order - pp;
                                    p = (1 == pp) ? "" : std::to_string(pp);
                                    q = (1 == qq) ? "" : std::to_string(qq);
                                    key = negate(p + ka + '+' + q + kb, true);
                                    add_comp(keys, comps, key, fft_fixed_tone::create(FFTCompTag::IMD, key, ssb_auto));
                                }
                            }
                        } else {
                            // odd order
                            bool neg = cplx && (1 == (group / 2) % 2);
                            for (qq = 1; qq < group; ++qq) {
                                pp = order - qq;
                                p = (1 == pp) ? "" : std::to_string(pp);
                                q = (1 == qq) ? "" : std::to_string(qq);
                                key = negate(p + ka + '+' + q + kb, neg);
                                add_comp(keys, comps, key, fft_fixed_tone::create(FFTCompTag::IMD, key, ssb_auto));
                            }
                            if (find_qe) {
                                for (pp = 1; pp < group; ++pp) {
                                    qq = order - pp;
                                    p = (1 == pp) ? "" : std::to_string(pp);
                                    q = (1 == qq) ? "" : std::to_string(qq);
                                    key = negate(p + ka + '+' + q + kb, !neg);
                                    add_comp(keys, comps, key, fft_fixed_tone::create(FFTCompTag::IMD, key, ssb_auto));
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    // WO
    ssb = (m_ssb_wo < 0) ? ssb_auto : m_ssb_wo;
    ssb = std::min(ssb, ssb_max);
    if (1 == m_wo) {
        add_comp(keys, comps, "wo", fft_max_tone::create(FFTCompTag::Noise, "abc-fshift", "abw", ssb));
    } else {
        for (int i = 1; i <= m_wo; ++i) {
            key = "wo" + std::to_string(i);
            add_comp(keys, comps, key, fft_max_tone::create(FFTCompTag::Noise, "abc-fshift", "abw", ssb));
        }
    }
    return keys_comps;
}

void fft_analysis2::load_comp(const str_t& key, const io_map& state)
{
    if_key_not_available_throw(key); // paranoid; should be unnecessary
    m_user_keys.push_back(key);
    m_user_comps[key] = fft_component::load(state);
}

var_map::unique_ptr fft_analysis2::meas_band(const real_vector& ms_data,
    bool cplx,
    size_t fft_size,
    me_vars_map& vars,
    mask_map& masks,
    const fft_band& comp) const
{
    static_cast<void>(ms_data);
    static_cast<void>(cplx);
    static_cast<void>(fft_size);
    static_cast<void>(vars);
    static_cast<void>(masks);
    auto results = var_map::create();
    results->insert(fft_comp_type_map.enum_name(),
        fft_comp_type_map[comp.type()]);
    results->insert(fft_comp_tag_map.enum_name(),
        fft_comp_tag_map[comp.tag()]);
    return results;
}

var_map::unique_ptr fft_analysis2::meas_fixed_tone(
    const real_vector& ms_data,
    bool cplx,
    size_t fft_size,
    me_vars_map& vars,
    mask_map& masks,
    const fft_fixed_tone& comp) const
{
    real_t fbin = vars.at("fbin");
    real_t fdata = vars.at("fdata");
    real_t fshift = vars.at("fshift");
    real_t freq = math_expression(comp.freq()).evaluate(vars);
    bool dev = false;
    const int ssb = comp.ssb();
    if (0 == ssb) {
        // For a FixedTone with SSB=0, freq and fshift are adjusted to the
        // nearest bin, even if initially they are not on a bin.  A
        // deviation is reported if either are not within half of fbin/1000.
        auto nb_freq = nearest_bin(freq, fbin, 0.001);
        auto nb_fshift = nearest_bin(fshift, fbin, 0.001);
        dev = nb_freq.second || nb_fshift.second;
        freq = nb_freq.first * fbin;
        fshift = nb_fshift.first * fbin;
    }
    static_cast<void>(dev);
    real_t fshifted = freq + fshift;
    auto nb = nearest_bin(fshifted, fbin, 0.01);
    auto center = static_cast<real_t>(nb.first);
    bool on_half = nb.second; // on_half implies on bin boundary
    if (on_half) {
        center += (center * fbin < fshifted) ? 0.5 : -0.5;
    }
    real_t ffinal;
    if (cplx) {
        ffinal = fft_alias(fdata, fshifted, m_axis_shift);
        center = fft_alias(fdata, center * fbin, false) / fbin;
    } else {
        ffinal = rfft_alias(fdata, fshifted);
        center = rfft_alias(fdata, center * fbin) / fbin;
    }
    real_t lbin = on_half ? (center - 0.5) - ssb : center - ssb;
    real_t rbin = on_half ? (center + 0.5) + ssb : center + ssb;
    size_t i1, i2;
    if (cplx) {
        i1 = fft_index(fft_size, fdata, lbin * fbin, false);
        i2 = fft_index(fft_size, fdata, rbin * fbin, false);
    } else {
        lbin = std::fmax(0.0, lbin);
        rbin = std::fmin(rbin, static_cast<real_t>(ms_data.size() - 1));
        i1 = rfft_index(fft_size, fdata, lbin * fbin);
        i2 = rfft_index(fft_size, fdata, rbin * fbin);
    }
    size_t i2a = (i2 < i1) ? i2 + fft_size : i2;
    size_t nbins = (i2a + 1) - i1;
    real_t fwave = 0.0;
    real_t mag_ms = 0.0;
    real_t mag_dbfs = min_db_value;
    // the following is not sufficient for Real data at the boundaries
    for (size_t i = i1; i < i2a + 1; ++i) {
        real_t m = ms_data[i % fft_size];
        fwave += m * i;
        mag_ms += m;
    }
    if (0.0 == mag_ms) {
        fwave = center * fbin;
    } else {
        fwave = (fwave / mag_ms) * fbin;
        if (cplx) {
            fwave = fft_alias(fdata, fwave, m_axis_shift);
        }
        mag_dbfs = 10 * std::log10(mag_ms);
    }
    bool inband = masks.at("non_ab").in_open_range(center);
    if (FFTCompTag::Noise != comp.tag()) {
        masks[fft_comp_tag_map[comp.tag()]].set_range2(i1, i2 + 1);
    }
    masks["comp"].set_range2(i1, i2 + 1);
    auto results = var_map::create();
    results->insert(fft_comp_type_map.enum_name(),
        fft_comp_type_map[comp.type()]);
    results->insert(fft_comp_tag_map.enum_name(),
        fft_comp_tag_map[comp.tag()]);
    results->insert("freq", freq);
    results->insert("ffinal", ffinal);
    results->insert("fwave", fwave);
    //results->insert("dev"    , dev);
    results->insert("inband", inband);
    results->insert("i1", static_cast<io_int_t>(i1));
    results->insert("center", center);
    results->insert("i2", static_cast<io_int_t>(i2 + 1));
    results->insert("nbins", static_cast<io_int_t>(nbins));
    results->insert("mag", mag_dbfs);
    return results;
}

var_map::unique_ptr fft_analysis2::meas_max_tone(
    const real_vector& ms_data,
    bool cplx,
    size_t fft_size,
    me_vars_map& vars,
    mask_map& masks,
    const fft_max_tone& comp) const
{
    real_t fbin = vars.at("fbin");
    real_t fdata = vars.at("fdata");
    real_t fshift = vars.at("fshift");
    real_t sb_center = math_expression(comp.center()).evaluate(vars);
    real_t sb_width = math_expression(comp.width()).evaluate(vars);
    sb_width = std::fmax(0.0, std::fmin(sb_width, fdata));
    real_t sb_lfreq = sb_center - sb_width / 2;
    real_t sb_rfreq = sb_center + sb_width / 2;
    real_t sb_cshifted = sb_center + fshift;
    real_t sb_calias = cplx ? fft_alias(fdata, sb_cshifted, false)
                            : rfft_alias(fdata, sb_cshifted);
    real_t nz_lfreq = 0.0;
    real_t nz_rfreq = 0.0;
    sparse_array_mask sb_mask(masks["comp"]);
    if (sb_width < fdata) {
        real_t sb_lalias = sb_calias - sb_width / 2;
        real_t sb_ralias = sb_calias + sb_width / 2;
        auto sb_lbin = static_cast<diff_t>(std::ceil(sb_lalias / fbin));
        auto sb_rbin = static_cast<diff_t>(std::floor(sb_ralias / fbin));
        auto data_size = static_cast<diff_t>(ms_data.size());
        if (cplx) {
            if (sb_lbin < 0) { // wrap around DC
                sb_mask.set_range(sb_rbin + 1, sb_lbin + data_size);
            } else if (data_size <= sb_rbin) { // wrap around fdata
                sb_mask.set_range(sb_rbin - data_size + 1, sb_lbin);
            } else { // no wrap
                sb_mask.set_range(0, sb_lbin);
                if (sb_rbin < data_size - 1) {
                    sb_mask.set_range(sb_rbin + 1, data_size);
                }
            }
        } else { // Real data, no wrap
            nz_lfreq = std::floor(2 * sb_lfreq / fdata) * fdata / 2;
            nz_rfreq = std::ceil(2 * sb_rfreq / fdata) * fdata / 2;
            sb_lfreq = std::fmax(nz_lfreq, sb_lfreq);
            sb_rfreq = std::fmin(sb_rfreq, nz_rfreq);
            sb_lbin = std::max<diff_t>(0, sb_lbin);
            sb_rbin = std::min(sb_rbin, data_size - 1);
            sb_mask.set_range(0, sb_lbin);
            if (sb_rbin < data_size - 1) {
                sb_mask.set_range(sb_rbin + 1, data_size);
            }
        }
    }
    // Just to get it working, looking for max single bin.  With window in
    // play, this is not guaranteed to give correct answer (but it is
    // probably 99% good enough).  Need to figure out what is actually the
    // correct answer and how best to get it.
    real_t center = -1.0;
    real_t max = 0.0;
    for (const diff_p& range : sb_mask.all_open_ranges()) {
        auto i1 = static_cast<size_t>(range.first);
        auto i2 = static_cast<size_t>(range.second);
        for (size_t i = i1; i < i2; ++i) {
            if (max < ms_data[i]) {
                max = ms_data[i];
                center = static_cast<real_t>(i);
            }
        }
    }
    size_t i1, i2, nbins;
    real_t fwave = 0.0;
    real_t mag_ms = 0.0;
    real_t mag_dbfs = min_db_value;
    if (-1.0 == center) { // no free bins in the search band
        i1 = cplx ? fft_index(fft_size, fdata, sb_calias, false)
                  : rfft_index(fft_size, fdata, sb_calias);
        i2 = i1;
        center = static_cast<real_t>(i1);
        nbins = 0;
        fwave = sb_calias;
    } else {
        real_t lbin = center - comp.ssb();
        real_t rbin = center + comp.ssb();
        if (cplx) {
            i1 = fft_index(fft_size, fdata, lbin * fbin, false);
            i2 = fft_index(fft_size, fdata, rbin * fbin, false);
        } else {
            lbin = std::fmax(0.0, lbin);
            rbin = std::fmin(rbin, static_cast<real_t>(ms_data.size() - 1));
            i1 = rfft_index(fft_size, fdata, lbin * fbin);
            i2 = rfft_index(fft_size, fdata, rbin * fbin);
        }
        size_t i2a = (i2 < i1) ? i2 + fft_size : i2;
        nbins = (i2a + 1) - i1;
        // this is not sufficient for Real data at the boundaries
        for (size_t i = i1; i < i2a + 1; ++i) {
            real_t m = ms_data[i % fft_size];
            fwave += m * i;
            mag_ms += m;
        }
        if (0.0 == mag_ms) {
            fwave = center * fbin;
        } else {
            fwave = (fwave / mag_ms) * fbin;
            mag_dbfs = 10 * std::log10(mag_ms);
        }
        if (cplx) {
            fwave = fft_alias(fdata, fwave, m_axis_shift);
        }
    }
    real_t ffinal = fwave;
    real_t freq = ffinal - fshift;
    if (freq < sb_lfreq) {
        while (freq < sb_lfreq) {
            freq += fdata;
        }
    } else if (sb_rfreq < freq) {
        while (sb_rfreq < freq) {
            freq -= fdata;
        }
    }
    if (!cplx) {
        if (freq < sb_lfreq || sb_rfreq < freq) {
            // freq is in folded NZ
            freq = nz_rfreq - rfft_alias(fdata, freq);
        }
    }
    bool inband = masks.at("non_ab").in_open_range(center);
    if (FFTCompTag::Noise != comp.tag()) {
        masks[fft_comp_tag_map[comp.tag()]].set_range2(i1, i2 + 1);
    }
    masks["comp"].set_range2(i1, i2 + 1);
    auto results = var_map::create();
    results->insert(fft_comp_type_map.enum_name(),
        fft_comp_type_map[comp.type()]);
    results->insert(fft_comp_tag_map.enum_name(),
        fft_comp_tag_map[comp.tag()]);
    results->insert("freq", freq);
    results->insert("ffinal", ffinal);
    results->insert("fwave", fwave);
    //results->insert("dev"    , dev);
    results->insert("inband", inband);
    results->insert("i1", static_cast<io_int_t>(i1));
    results->insert("center", center);
    results->insert("i2", static_cast<io_int_t>(i2 + 1));
    results->insert("nbins", static_cast<io_int_t>(nbins));
    results->insert("mag", mag_dbfs);
    return results;
}

namespace { // Helpers for setup_ab

    var_map::unique_ptr setup_ab_cplx(real_t fbin,
        real_t fdata,
        real_t cfreq,
        real_t width,
        sparse_array_mask& mask,
        bool axis_shift)
    {
        cfreq = fft_alias(fdata, cfreq, false); // [ 0 , fdata )
        real_t lfreq = cfreq - width / 2; // [ -fdata / 2, fdata )
        real_t rfreq = cfreq + width / 2; // [ 0.0, fdata * 3/2 )
        auto lbin = static_cast<diff_t>(std::ceil(lfreq / fbin));
        auto rbin = static_cast<diff_t>(std::floor(rfreq / fbin));
        diff_t size = mask.array_size();
        auto nbins = std::max<diff_t>(0, std::min(rbin + 1 - lbin, size));
        diff_t lindex = 0;
        diff_t rindex = 0;
        if (nbins == size) {
            cfreq = axis_shift ? 0 : fdata / 2;
            lfreq = axis_shift ? -fdata / 2 : 0;
            rfreq = axis_shift ? fdata / 2 : fdata;
            rindex = size;
        } else { // nbins < size
            cfreq = fft_alias(fdata, cfreq, axis_shift);
            lfreq = fft_alias(fdata, lfreq, axis_shift);
            rfreq = fft_alias(fdata, rfreq, axis_shift);
            if (0 == nbins) {
                mask.set_all();
            } else if (lbin < 0) { // wrap around DC
                lindex = lbin + size;
                rindex = rbin + 1;
                mask.set_range(rindex, lindex);
            } else if (size <= rbin) { // wrap around FDATA
                lindex = lbin;
                rindex = rbin - size + 1;
                mask.set_range(rindex, lindex);
            } else { // no wrap
                lindex = lbin;
                rindex = rbin + 1;
                mask.set_all();
                mask.unset_range(lindex, rindex);
            }
        }
        auto results = var_map::create();
        results->insert("cfreq", cfreq);
        results->insert("width", width);
        results->insert("lfreq", lfreq);
        results->insert("rfreq", rfreq);
        results->insert("lindex", static_cast<io_int_t>(lindex));
        results->insert("rindex", static_cast<io_int_t>(rindex));
        results->insert("nbins", static_cast<io_int_t>(nbins));
        return results;
    }

    var_map::unique_ptr setup_ab_real(real_t fbin,
        real_t fdata,
        real_t cfreq,
        real_t width,
        sparse_array_mask& mask)
    {
        cfreq = rfft_alias(fdata, cfreq); // [ 0 , fdata / 2]
        real_t lfreq = cfreq - width / 2;
        real_t rfreq = cfreq + width / 2;
        // lfreq, rfreq: [ 0, fdata / 2 ]
        lfreq = std::fmax(0.0, std::fmin(lfreq, fdata / 2));
        rfreq = std::fmax(0.0, std::fmin(rfreq, fdata / 2));
        cfreq = (lfreq + rfreq) / 2;
        width = rfreq - lfreq;
        auto lbin = static_cast<diff_t>(std::ceil(lfreq / fbin));
        auto rbin = static_cast<diff_t>(std::floor(rfreq / fbin));
        diff_t size = mask.array_size();
        auto nbins = std::max<diff_t>(0, std::min(rbin + 1 - lbin, size));
        diff_t lindex = 0;
        diff_t rindex = 0;
        mask.set_all();
        if (0 < nbins) {
            lindex = lbin;
            rindex = rbin + 1;
            mask.unset_range(lindex, rindex);
        }
        auto results = var_map::create();
        results->insert("cfreq", cfreq);
        results->insert("width", width);
        results->insert("lfreq", lfreq);
        results->insert("rfreq", rfreq);
        results->insert("lindex", static_cast<io_int_t>(lindex));
        results->insert("rindex", static_cast<io_int_t>(rindex));
        results->insert("nbins", static_cast<io_int_t>(nbins));
        return results;
    }

} // namespace anonymous - Helpers for setup_ab

/*
     * This function is a little confusing.  The results it returns relate to
     * the AnalysisBand, however, the way the mask is set indicates the inverse
     * of the AnalysisBand; the ranges in the mask represent the non-AB bins.
     *
     * Another little thing that will probably go unnoticed involves the result
     * 'width'.  Width reflects the user's input, subject to limits:
     *     Cplx: fdata
     *     Real: fdata / 2
     * This width is used in the analysis to calculate NSD.  Technically, width
     * should reflect the number of bins in the AB: nbins * fbin.  However, if
     * User specifies 500e6, User probably expects to see 500e6; this allows
     * allows User to verify NSD is his or her own.
     */
var_map::unique_ptr fft_analysis2::setup_ab(
    bool cplx, me_vars_map& vars, sparse_array_mask& mask) const
{
    real_t fbin = vars.at("fbin");
    real_t fdata = vars.at("fdata");
    real_t cfreq = math_expression(m_ab_center).evaluate(vars);
    real_t width = math_expression(m_ab_width).evaluate(vars);
    width = std::fmax(0.0, std::fmin(width, fdata)); // [ 0 , fdata ]
    var_map::unique_ptr ab;
    if (cplx) {
        ab = setup_ab_cplx(fbin, fdata, cfreq, width, mask, m_axis_shift);
    } else {
        ab = setup_ab_real(fbin, fdata, cfreq, width, mask);
    }
    vars["abc"] = ab->as_real("cfreq");
    vars["abw"] = ab->as_real("width");
    return ab;
}

fft_analysis2::me_vars_map fft_analysis2::setup_vars(size_t fft_size) const
{
    me_vars_map vars(m_user_vars.begin(), m_user_vars.end());
    real_t fsample = math_expression(m_fsample).evaluate(vars);
    check_fs(fsample);
    vars["fs"] = fsample;
    real_t fdata = math_expression(m_fdata).evaluate(vars);
    check_fs(fdata);
    real_t fbin = fdata / fft_size;
    vars["fdata"] = fdata;
    vars["fbin"] = fbin;
    const real_t fshift = math_expression(m_fshift).evaluate(vars);
    vars["fshift"] = fshift;
    return vars;
}

void fft_analysis2::update_maxspur(str_t& maxspur_key,
    real_t& maxspur_mag,
    const str_t& key,
    real_t mag,
    const var_map& results,
    mask_map& masks) const
{
    // key and mag represent an in-band non-Signal tone.
    // The tone is eligible for maxspur if two conditions are met:
    // 1. If analysis "excludes DC" (m_ex_dc),
    //    Then tone is not centered at bin zero
    // 2. tone is nonoverlapping with all Signal tones
    if (m_ex_dc && 0.0 == results.as_real("center")) {
        return;
    }
    const sparse_array_mask& sig_mask
        = masks[fft_comp_tag_map[FFTCompTag::Signal]];
    auto i1 = static_cast<size_t>(results.as_int("i1"));
    auto i2 = static_cast<size_t>(results.as_int("i2"));
    if (sig_mask.is_nonoverlapping(i1, i2)) {
        if (maxspur_key.empty() || maxspur_mag < mag) {
            maxspur_key = key;
            maxspur_mag = mag;
        }
    }
}

} // namespace analysis - Protected and Private Functions
