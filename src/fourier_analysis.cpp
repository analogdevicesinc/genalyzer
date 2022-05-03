#include "fourier_analysis.hpp"

#include "array_ops.hpp"
#include "enum_maps.hpp"
#include "exceptions.hpp"
#include "expression.hpp"
#include "formatted_data.hpp"
#include "utils.hpp"

#include <algorithm>
#include <numeric>
#include <regex>
#include <sstream>

namespace dcanalysis_impl {

    namespace {

        struct fraction
        {
            int num;
            int den;
            str_t term;

            fraction(int n, int d, const str_t& var = "")
            {
                if (!(1 <= n && 1 <= d && n < d)) {
                    throw runtime_error("Invalid fraction");
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

        }; // struct fraction

        bool operator<(const fraction& lhs, const fraction& rhs)
        {
            // Based on numeric value, this operator would:
            // return lhs.num * rhs.den < lhs.den * rhs.num;
            // But in this implementation of Fourier analysis, fractions are grouped by
            // denominator, in increasing order:
            if (lhs.den < rhs.den) {
                return true;
            } else {
                return (lhs.den == rhs.den) && (lhs.num < rhs.num);
            }
        }

        str_t negate(const str_t& expr, bool neg)
        {
            return neg ? ("-(" + expr + ')') : expr;
        }

    } // namespace anonymous

    const fourier_analysis::min_max_def_t fourier_analysis::mmd_hd  = { 1, 99, 6 };
    const fourier_analysis::min_max_def_t fourier_analysis::mmd_imd = { 1,  9, 3 };
    const fourier_analysis::min_max_def_t fourier_analysis::mmd_wo  = { 1,  9, 1 };
    const fourier_analysis::min_max_def_t fourier_analysis::mmd_ssb = { 0,  1 << 29, 0 };

} // namespace dcanalysis_impl

namespace dcanalysis_impl { // Constructors, Destructor, and Assignment

    fourier_analysis::fourier_analysis()
        : object {},
          // Parameters
          clk_as_noise   (false),
          dc_as_dist     (false),
          en_conv_offset (false),
          en_fund_images (true),
          en_quad_errors (false),
          ilv_as_noise   (false),
          m_hd           (std::get<2>(mmd_hd)),
          m_imd          (std::get<2>(mmd_imd)),
          m_wo           (std::get<2>(mmd_wo)),
          m_ssb_def      (std::get<2>(mmd_ssb)),
          m_ssb_dc       (-1),
          m_ssb_sig      (-1),
          m_ssb_wo       (-1),
          m_ab_center    ("0"),
          m_ab_width     ("fdata"),
          m_fdata        ("fs"),
          m_fsample      ("1"),
          m_fshift       ("0"),
          m_clk          (),
          m_ilv          (),
          // Components  
          m_user_keys    (),
          m_user_comps   (),
          // Variables  
          m_user_vars    ()
    {}

    fourier_analysis::fourier_analysis(const fourier_analysis& obj)
        : object {},
          // Parameters
          clk_as_noise   (obj.clk_as_noise),
          dc_as_dist     (obj.dc_as_dist),
          en_conv_offset (obj.en_conv_offset),
          en_fund_images (obj.en_fund_images),
          en_quad_errors (obj.en_quad_errors),
          ilv_as_noise   (obj.ilv_as_noise),
          m_hd           (obj.m_hd),
          m_imd          (obj.m_imd),
          m_wo           (obj.m_wo),
          m_ssb_def      (obj.m_ssb_def),
          m_ssb_dc       (obj.m_ssb_dc),
          m_ssb_sig      (obj.m_ssb_sig),
          m_ssb_wo       (obj.m_ssb_wo),
          m_ab_center    (obj.m_ab_center),
          m_ab_width     (obj.m_ab_width),
          m_fdata        (obj.m_fdata),
          m_fsample      (obj.m_fsample),
          m_fshift       (obj.m_fshift),
          m_clk          (obj.m_clk),
          m_ilv          (obj.m_ilv),
          // Components  
          m_user_keys    (obj.m_user_keys),
          m_user_comps   (),
          // Variables  
          m_user_vars    (obj.m_user_vars)
    {
        for (const str_t& key : m_user_keys) {
            m_user_comps[key] = obj.m_user_comps.at(key)->clone();
        }
    }

    fourier_analysis::fourier_analysis(fourier_analysis&& obj)
        : object {},
          // Parameters
          clk_as_noise   (obj.clk_as_noise),
          dc_as_dist     (obj.dc_as_dist),
          en_conv_offset (obj.en_conv_offset),
          en_fund_images (obj.en_fund_images),
          en_quad_errors (obj.en_quad_errors),
          ilv_as_noise   (obj.ilv_as_noise),
          m_hd           (obj.m_hd),
          m_imd          (obj.m_imd),
          m_wo           (obj.m_wo),
          m_ssb_def      (obj.m_ssb_def),
          m_ssb_dc       (obj.m_ssb_dc),
          m_ssb_sig      (obj.m_ssb_sig),
          m_ssb_wo       (obj.m_ssb_wo),
          m_ab_center    (obj.m_ab_center),
          m_ab_width     (obj.m_ab_width),
          m_fdata        (obj.m_fdata),
          m_fsample      (obj.m_fsample),
          m_fshift       (obj.m_fshift),
          m_clk          (std::move(obj.m_clk)),
          m_ilv          (std::move(obj.m_ilv)),
          // Components  
          m_user_keys    (std::move(obj.m_user_keys)),
          m_user_comps   (std::move(obj.m_user_comps)),
          // Variables  
          m_user_vars    (std::move(obj.m_user_vars))
    {}

    fourier_analysis& fourier_analysis::operator=(const fourier_analysis& obj)
    {
        if (&obj != this) {
            fourier_analysis the_copy (obj);
            std::swap(the_copy, *this);
        }
        return *this;
    }

    fourier_analysis& fourier_analysis::operator=(fourier_analysis&& obj)
    {
        // Parameters
        std::swap(clk_as_noise   , obj.clk_as_noise);
        std::swap(dc_as_dist     , obj.dc_as_dist);
        std::swap(en_conv_offset , obj.en_conv_offset);
        std::swap(en_fund_images , obj.en_fund_images);
        std::swap(en_quad_errors , obj.en_quad_errors);
        std::swap(ilv_as_noise   , obj.ilv_as_noise);
        std::swap(m_hd           , obj.m_hd);
        std::swap(m_imd          , obj.m_imd);
        std::swap(m_wo           , obj.m_wo);
        std::swap(m_ssb_def      , obj.m_ssb_def);
        std::swap(m_ssb_dc       , obj.m_ssb_dc);
        std::swap(m_ssb_sig      , obj.m_ssb_sig);
        std::swap(m_ssb_wo       , obj.m_ssb_wo);
        std::swap(m_ab_center    , obj.m_ab_center);
        std::swap(m_ab_width     , obj.m_ab_width);
        std::swap(m_fdata        , obj.m_fdata);
        std::swap(m_fsample      , obj.m_fsample);
        std::swap(m_fshift       , obj.m_fshift);
        std::swap(m_clk          , obj.m_clk);
        std::swap(m_ilv          , obj.m_ilv);
        // Components    
        std::swap(m_user_keys    , obj.m_user_keys);
        std::swap(m_user_comps   , obj.m_user_comps);
        // Variables     
        std::swap(m_user_vars    , obj.m_user_vars);
        return *this;
    }

} // namespace dcanalysis_impl - Constructors, Destructor, and Assignment

namespace dcanalysis_impl { // Analysis

    fourier_analysis_results fourier_analysis::analyze(
        const real_t* in_data,
        const size_t in_size,
        const size_t nfft,
        FreqAxisType axis_type
        ) const
    {
        check_array("", "input array", in_data, in_size);
        std::vector<real_t> msq; // used only if in_data is complex
        const real_t* msq_data = nullptr;
        size_t msq_size = 0;
        const cplx_t* fft_data = nullptr;
        size_t fft_size = 0;
        if (in_size == nfft ||                          // Real data, complex analysis
            in_size == nfft / 2 + 1) {                  // Real data, real analysis
            // input array is mean-square FFT data (phase not available)
            msq_data = in_data;
            msq_size = in_size;
        } else if (in_size == nfft * 2 ||               // Complex data, complex analysis
                   in_size == (nfft / 2 + 1) * 2) {     // Complex data, real analysis
            // input array is interleaved Re/Im FFT data
            msq_size = in_size / 2;
            msq = std::vector<real_t>(msq_size);
            norm(in_data, in_size, msq.data(), msq.size());
            msq_data = msq.data();
            fft_data = reinterpret_cast<const cplx_t*>(in_data);
            fft_size = in_size / 2;
        } else {
            throw runtime_error("Mismatch between data size and NFFT");
        }
        fourier_analysis_results results = analyze_impl(msq_data, msq_size, nfft, axis_type, fft_data, fft_size);
        return results;
    }

} // namespace dcanalysis_impl - Analysis

namespace dcanalysis_impl { // Component Definition

    void fourier_analysis::add_fixed_tone(
        const str_t& key, FACompTag tag, const str_t& freq, int ssb)
    {
        if_key_not_available_throw(key);
        expression fe (freq);
        ssb = limit_ssb(ssb, -1);
        m_user_keys.push_back(key);
        m_user_comps.insert({key, std::make_unique<fa_fixed_tone>(
            tag, fe.to_string(FPFormat::Eng), ssb)});
    }

    void fourier_analysis::add_max_tone(
        const str_t& key, FACompTag tag, const str_t& center, const str_t& width, int ssb)
    {
        if (FACompTag::Signal == tag || FACompTag::UserDist == tag || FACompTag::Noise == tag) {
            if_key_not_available_throw(key);
            expression ce (center);
            expression we (width);
            ssb = limit_ssb(ssb, -1);
            m_user_keys.push_back(key);
            m_user_comps.insert({key, std::make_unique<fa_max_tone>(
                tag, ce.to_string(FPFormat::Eng), we.to_string(FPFormat::Eng), ssb)});
        } else {
            throw runtime_error("tag must be one of {"
                + fa_comp_tag_map.at(to_int(FACompTag::Signal)) + ", "
                + fa_comp_tag_map.at(to_int(FACompTag::UserDist)) + ", "
                + fa_comp_tag_map.at(to_int(FACompTag::Noise)) + '}');
        }
    }

    void fourier_analysis::remove_comp(const str_t& key)
    {
        if (is_comp(key)) {
            m_user_comps.erase(key);
            m_user_keys.erase(std::remove(m_user_keys.begin(), m_user_keys.end(), key), m_user_keys.end());
        }
    }

} // namespace dcanalysis_impl - Component Definition

namespace dcanalysis_impl { // Configuration

    namespace {

        void fa_set_expr(const str_t& name, str_t& m_expr, const str_t& expr, const expression::var_set& disallowed)
        {
            expression e (expr);
            if (e.depends_on(disallowed)) {
                std::ostringstream ss (name);
                ss << " may not depend on";
                for (const str_t& s : disallowed) {
                    ss << " '" << s << "',";
                }
                ss.seekp(-1, std::ios_base::end);
                throw runtime_error(ss.str());
            } else {
                m_expr = e.to_string(FPFormat::Eng);
            }
        }

    } // namespace anonymous

    int fourier_analysis::ssb(FASsb group) const
    {
        switch (group)
        {
        case FASsb::DC :
            return m_ssb_dc;
        case FASsb::Signal :
            return m_ssb_sig;
        case FASsb::WO :
            return m_ssb_wo;
        default :
            return m_ssb_def;
        }
    }

    void fourier_analysis::set_analysis_band(const str_t& center, const str_t& width)
    {
        fa_set_expr("ab_center", m_ab_center, center, {});
        fa_set_expr("ab_width", m_ab_width, width, {});
    }

    void fourier_analysis::set_clk(const std::set<int>& clk)
    {
        m_clk.clear();
        for (int n : clk) {
            if (2 <= n && n <= 256) {
                m_clk.insert(n);
            }
        }
    }

    void fourier_analysis::set_fdata(const str_t& expr)
    {
        expression::var_set disallowed {"fbin", "fdata", "fshift"};
        fa_set_expr("fdata", m_fdata, expr, disallowed);
    }

    void fourier_analysis::set_fsample(const str_t& expr)
    {
        expression::var_set disallowed {"fbin", "fdata", "fs", "fshift"};
        fa_set_expr("fsample", m_fsample, expr, disallowed);
    }

    void fourier_analysis::set_fshift(const str_t& expr)
    {
        expression::var_set disallowed {"fshift"};
        fa_set_expr("fshift", m_fshift, expr, disallowed);
    }

    void fourier_analysis::set_hd(int n)
    {
        m_hd = std::clamp(n, std::get<0>(mmd_hd), std::get<1>(mmd_hd));
    }

    void fourier_analysis::set_ilv(const std::set<int>& ilv)
    {
        m_ilv.clear();
        for (int n : ilv) {
            if (2 <= n && n <= 64) {
                m_ilv.insert(n);
            }
        }
    }

    void fourier_analysis::set_imd(int n)
    {
        m_imd = std::clamp(n, std::get<0>(mmd_imd), std::get<1>(mmd_imd));
    }

    void fourier_analysis::set_ssb(FASsb group, int ssb)
    {
        switch (group)
        {
        case FASsb::DC :
            m_ssb_dc = limit_ssb(ssb, -1);
            break;
        case FASsb::Signal :
            m_ssb_sig = limit_ssb(ssb, -1);
            break;
        case FASsb::WO :
            m_ssb_wo = limit_ssb(ssb, -1);
            break;
        default :
            m_ssb_def = limit_ssb(ssb, std::get<0>(mmd_ssb));
            break;
        }
    }

    void fourier_analysis::set_var(const str_t& key, real_t x)
    {
        if (!is_var(key)) {
            if_key_not_available_throw(key);
        }
        if (!std::isfinite(x)) {
            throw runtime_error("got non-finite value");
        }
        m_user_vars[key] = x;
    }

    void fourier_analysis::set_wo(int n)
    {
        m_wo = std::clamp(n, std::get<0>(mmd_wo), std::get<1>(mmd_wo));
    }

} // namespace dcanalysis_impl - Configuration

namespace dcanalysis_impl { // Key Queries

    bool fourier_analysis::is_reserved(const str_t& key)
    {
        if (reserved_keys.end() != reserved_keys.find(key)) {
            return true;
        }
        for (const str_t& pat : reserved_patterns) {
            std::regex re (pat);
            if (std::regex_match(key, re)) {
                return true;
            }
        }
        return false;
    }

    bool fourier_analysis::is_valid(const str_t& key)
    {
        std::regex re (key_pattern);
        return std::regex_match(key, re);
    }

} // namespace dcanalysis_impl - Key Queries

namespace dcanalysis_impl { // Other Member Functions
        
    str_t fourier_analysis::flat_tone_key(const str_t& key, int result_index)
    {
        fa_tone_result_map.contains(result_index, true);
        return key + flat_key_coupler + fa_tone_result_map.at(result_index);
    }

    std::pair<str_t, str_t> fourier_analysis::split_key(const str_t& key)
    {
        std::pair<str_t, str_t> keys {"", ""};
        size_t pos = key.find(flat_key_coupler);
        if (std::string::npos == pos) {
            keys.first = key;
        } else {
            keys.first = key.substr(0, pos);
            keys.second = key.substr(pos + flat_key_coupler.length());
        }
        return keys;
    }

    str_t fourier_analysis::preview(bool cplx) const
    {
        const comp_data_t comp_data = generate_comps(cplx);
        const str_vector& keys = std::get<0>(comp_data);
        const comp_map& comps  = std::get<1>(comp_data);
        std::vector<str_vector> header {{"Index", "Key", "Type", "Tag", "Spec"}};
        std::vector<str_vector> data;
        int i = 0;
        for (const str_t& key : keys) {
            const comp_ptr& comp = comps.at(key);
            str_vector row {std::to_string(i), key};
            row.push_back(fa_comp_type_map.at(to_int(comp->type)));
            row.push_back(fa_comp_tag_map.at(to_int(comp->tag)));
            row.push_back(comp->spec());
            data.push_back(row);
            ++i;
        }
        return table(header, data, 2, true, true);
    }

    void fourier_analysis::reset()
    {
        fourier_analysis new_obj {};
        std::swap(new_obj, *this);
    }

    namespace {

        bool is_cplx_analysis(size_t in_size, size_t nfft)
        {
            bool cplx = false;
            if (in_size == nfft || in_size == nfft * 2) {
                cplx = true;
            } else if (!(in_size == nfft / 2 + 1 || in_size == (nfft / 2 + 1) * 2)) {
                throw runtime_error("Invalid combination of data size and NFFT");
            }
            return cplx;
        }

    } // namespace anonymous

    std::vector<size_t> fourier_analysis::result_key_lengths(size_t in_size, size_t nfft) const
    {
        bool cplx = is_cplx_analysis(in_size, nfft);
        const comp_data_t comp_data = generate_comps(cplx);
        const str_vector& comp_keys = std::get<0>(comp_data);
        const size_t num_result_keys = static_cast<size_t>(FAResult::__SIZE__);
        const size_t num_tone_result_keys = static_cast<size_t>(FAToneResult::__SIZE__);
        size_t total_keys = num_result_keys + num_tone_result_keys * comp_keys.size();
        std::vector<size_t> key_lengths (total_keys);
        size_t i = 0;
        for (int j = 0; j < static_cast<int>(num_result_keys); ++j) {
            key_lengths[i] = fa_result_map.at(j).length();
            i += 1;
        }
        size_t key_coupler_len = flat_key_coupler.length();
        std::vector<size_t> tone_result_key_lengths (num_tone_result_keys, key_coupler_len);
        for (int j = 0; j < static_cast<int>(num_tone_result_keys); ++j) {
            tone_result_key_lengths[j] += fa_tone_result_map.at(j).length();
        }
        for (const str_t& ckey : comp_keys) {
            const size_t ckey_len = ckey.length();
            for (size_t rkey_len : tone_result_key_lengths) {
                key_lengths[i] = ckey_len + rkey_len;
                i += 1;
            }
        }
        return key_lengths;
    }

    size_t fourier_analysis::results_size(size_t in_size, size_t nfft) const
    {
        bool cplx = is_cplx_analysis(in_size, nfft);
        const comp_data_t comp_data = generate_comps(cplx);
        const str_vector& keys = std::get<0>(comp_data);
        size_t size = fa_result_map.size();
        size += fa_tone_result_map.size() * keys.size();
        return size;
    }

} // namespace dcanalysis_impl - Other Member Functions

namespace dcanalysis_impl { // Virtual Function Overrides

    bool fourier_analysis::equals_impl(const object& that_obj) const
    {
        if (ObjectType::FourierAnalysis != that_obj.object_type()) {
            return false;
        }
        auto& that = static_cast<const fourier_analysis&>(that_obj);
        if ( (this->clk_as_noise   != that.clk_as_noise  ) ||
             (this->dc_as_dist     != that.dc_as_dist    ) ||
             (this->en_conv_offset != that.en_conv_offset) ||
             (this->en_fund_images != that.en_fund_images) ||
             (this->en_quad_errors != that.en_quad_errors) ||
             (this->ilv_as_noise   != that.ilv_as_noise  ) ) {
            return false;
        }
        if ( (this->m_hd           != that.m_hd          ) ||
             (this->m_imd          != that.m_imd         ) ||
             (this->m_wo           != that.m_wo          ) ||
             (this->m_ssb_def      != that.m_ssb_def     ) ||
             (this->m_ssb_dc       != that.m_ssb_dc      ) ||
             (this->m_ssb_sig      != that.m_ssb_sig     ) ||
             (this->m_ssb_wo       != that.m_ssb_wo      ) ) {
            return false;
        }
        if ( (this->m_ab_center    != that.m_ab_center   ) ||
             (this->m_ab_width     != that.m_ab_width    ) ||
             (this->m_fdata        != that.m_fdata       ) ||
             (this->m_fsample      != that.m_fsample     ) ||
             (this->m_fshift       != that.m_fshift      ) ) {
            return false;
        }
        if ( (this->m_clk          != that.m_clk         ) ||
             (this->m_ilv          != that.m_ilv         ) ||
             (this->m_user_keys    != that.m_user_keys   ) ||
             (this->m_user_vars    != that.m_user_vars   ) ) {
            return false;
        }
        for (const str_t& key : m_user_keys) {
            const fourier_analysis_component& this_comp = *this->m_user_comps.at(key);
            const fourier_analysis_component& that_comp = *that.m_user_comps.at(key);
            if (!this_comp.equals(that_comp)) {
                return false;
            }
        }
        return true;
    }

    str_t fourier_analysis::to_string_impl() const
    {
        return "=====\nFIXME\n=====\n";
    }
    
} // namespace dcanalysis_impl

namespace dcanalysis_impl { // Non-Public

    const str_t fourier_analysis::key_pattern = "[[:alpha:]][[:alnum:]_]*";

    const str_t fourier_analysis::wo_pattern = "^wo[1-9]?$";

    const std::set<str_t> fourier_analysis::reserved_keys = {
        "co",       // comp/var : converter offset
        "dc",       // comp     : DC
        "fbin",     // var      : frequency bin size
        "fdata",    // var      : data rate
        "fs",       // var      : sample rate
        "fshift"    // var      : shift frequency
        };

    const str_vector fourier_analysis::reserved_patterns = {
        fourier_analysis::wo_pattern
        };

    const str_t fourier_analysis::flat_key_coupler = ":";

    void fourier_analysis::add_comp(str_vector& keys, comp_map& comps, const str_t& k, comp_ptr c)
    {
        if (comps.find(k) != comps.end()) {
            throw runtime_error("fourier_analysis::add_comp : key '" + k + "' already exists");
        }
        keys.push_back(k);
        comps.insert({k, std::move(c)});
    }

    int fourier_analysis::limit_ssb(int ssb, int lower_limit) {
        lower_limit = (lower_limit < 0) ? -1 : std::get<0>(mmd_ssb);
        return std::clamp(ssb, lower_limit, std::get<1>(mmd_ssb));
    }

    fourier_analysis::comp_data_t fourier_analysis::generate_comps(bool cplx) const
    {
        bool find_fi = en_fund_images && cplx;
        bool find_qe = en_quad_errors && cplx;
        comp_data_t comp_data;
        str_vector& keys               = std::get<0>(comp_data);
        comp_map& comps                = std::get<1>(comp_data);
        std::set<str_t>& ilos_clk_keys = std::get<2>(comp_data);
        str_t key = "";
        int ssb = 0;
        // DC and Converter Offset Components
        ssb = (m_ssb_dc < 0) ? m_ssb_def : m_ssb_dc;
        add_comp(keys, comps, "dc", fa_dc::create(ssb));
        if (en_conv_offset) {
            add_comp(keys, comps, "co", fa_fixed_tone::create(FACompTag::UserDist, "0", ssb));
        }
        // Interleaving Offset and Clock Components
        std::set<fraction> ilos_clk_terms;
        std::set<fraction> ilv_terms;
        for (int x : m_ilv) {
            for (int i = 1; i <= x / 2; ++i) {
                ilv_terms.emplace(i, x, "fs");
                ilos_clk_terms.emplace(i, x, "fs");
            }
        }
        std::set<fraction> clk_terms;
        for (int x : m_clk)
        {
            for (int i = 1; i <= x / 2; ++i) {
                if (1 == std::gcd(i, x)) {
                    clk_terms.emplace(i, x, "fs");
                    ilos_clk_terms.emplace(i, x, "fs");
                }
            }
        }
        for (const fraction& f : ilos_clk_terms) {
            key = f.term;
            if (ilv_terms.end() != ilv_terms.find(f)) {
                bool is_also_clk = clk_terms.end() != clk_terms.find(f);
                add_comp(keys, comps, key, fa_fixed_tone::create(FACompTag::ILOS, key, m_ssb_def));
                if (is_also_clk) {
                    ilos_clk_keys.insert(key);
                }
                if (cplx && "fs/2" != key) {
                    key.insert(0, 1, '-');
                    add_comp(keys, comps, key, fa_fixed_tone::create(FACompTag::ILOS, key, m_ssb_def));
                    if (is_also_clk) {
                        ilos_clk_keys.insert(key);
                    }
                }
            } else {
                add_comp(keys, comps, key, fa_fixed_tone::create(FACompTag::CLK, key, m_ssb_def));
                if (cplx && "fs/2" != key) {
                    key.insert(0, 1, '-');
                    add_comp(keys, comps, key, fa_fixed_tone::create(FACompTag::CLK, key, m_ssb_def));
                }
            }
        }
        // User and Auto Components
        str_vector fund_keys;
        int def_sig_ssb = (m_ssb_sig < 0) ? m_ssb_def : m_ssb_sig;
        for (const str_t& ukey : m_user_keys) {
            const comp_ptr& comp = m_user_comps.at(ukey);
            int def_user_ssb = (FACompTag::Signal == comp->tag) ? def_sig_ssb : m_ssb_def;
            if (FACompType::FixedTone == comp->type) {
                const auto& c = static_cast<const fa_fixed_tone&>(*comp);
                ssb = (c.ssb < 0) ? def_user_ssb : c.ssb;
                add_comp(keys, comps, ukey, fa_fixed_tone::create(c.tag, c.freq, ssb));
            } else if (FACompType::MaxTone == comp->type) {
                const auto& c = static_cast<const fa_max_tone&>(*comp);
                ssb = (c.ssb < 0) ? def_user_ssb : c.ssb;
                add_comp(keys, comps, ukey, fa_max_tone::create(c.tag, c.center, c.width, ssb));
            } else {
                throw runtime_error("fourier_analysis::generate_comps : unsupported component type");
            }
            if (FACompTag::Signal != comp->tag) {
                continue;
            }
            fund_keys.push_back(ukey);
            ssb = m_ssb_def;
            // Fundamental Image Component
            if (find_fi) {
                key = '-' + ukey;
                add_comp(keys, comps, key, fa_fixed_tone::create(FACompTag::HD, key, ssb));
            }
            // Harmonic Distortion Components
            for (int i = 2; i <= m_hd; ++i) {
                if (is_even(i)) {
                    // even order
                    key = std::to_string(i) + ukey;
                    add_comp(keys, comps, key, fa_fixed_tone::create(FACompTag::HD, key, ssb));
                    if (cplx) {
                        key.insert(0, 1, '-');
                        add_comp(keys, comps, key, fa_fixed_tone::create(FACompTag::HD, key, ssb));
                    }
                } else {
                    // odd order
                    // real data: j = +i
                    // cplx data: j = -i for 3, 7, 11...
                    //            j = +i for 5, 9, 13...
                    int j = (cplx && is_odd(i / 2)) ? -i : i;
                    key = std::to_string(j) + ukey;
                    add_comp(keys, comps, key, fa_fixed_tone::create(FACompTag::HD, key, ssb));
                    if (find_qe) {
                        key = std::to_string(-j) + ukey;
                        add_comp(keys, comps, key, fa_fixed_tone::create(FACompTag::HD, key, ssb));
                    }
                }
            }
            // Interleaving Gain and Timing Components
            for (const fraction& f : ilv_terms) {
                key = ukey + '+' + f.term;
                add_comp(keys, comps, key, fa_fixed_tone::create(FACompTag::ILGT, key, ssb));
                if ("fs/2" != f.term) {
                    key = ukey + '-' + f.term;
                    add_comp(keys, comps, key, fa_fixed_tone::create(FACompTag::ILGT, key, ssb));
                }
                if (find_qe) {
                    key = negate(ukey + '+' + f.term, true);
                    add_comp(keys, comps, key, fa_fixed_tone::create(FACompTag::ILGT, key, ssb));
                    if ("fs/2" != f.term) {
                        key = negate(ukey + '-' + f.term, true);
                        add_comp(keys, comps, key, fa_fixed_tone::create(FACompTag::ILGT, key, ssb));
                    }
                }
            }
            // Intermodulation Distorion Components
            for (const str_t& ka : fund_keys) {
                if (ka == ukey) {
                    continue;
                }
                const str_t& kb = ukey;
                for (int order = 2; order <= m_imd; ++order) {
                    int group = is_even(order) ? 0 : 1;
                    int pp = 0;
                    int qq = 0;
                    str_t p = "";
                    str_t q = "";
                    for (; group <= order; group += 2) {
                        if (group < order) {
                            // form: p * kx MINUS q * ky
                            pp = (order + group) / 2;
                            qq = order - pp;
                            p = (1 == pp) ? "" : std::to_string(pp);
                            q = (1 == qq) ? "" : std::to_string(qq);
                            if (0 == group) {
                                key = p + kb + '-' + q + ka;
                                add_comp(keys, comps, key, fa_fixed_tone::create(FACompTag::IMD, key, ssb));
                                if (cplx) {
                                    key = negate(key, true);
                                    add_comp(keys, comps, key, fa_fixed_tone::create(FACompTag::IMD, key, ssb));
                                }
                            } else if (is_even(order)) {
                                // even order
                                key = p + ka + '-' + q + kb;
                                add_comp(keys, comps, key, fa_fixed_tone::create(FACompTag::IMD, key, ssb));
                                key = p + kb + '-' + q + ka;
                                add_comp(keys, comps, key, fa_fixed_tone::create(FACompTag::IMD, key, ssb));
                                if (cplx) {
                                    key = negate(p + ka + '-' + q + kb, true);
                                    add_comp(keys, comps, key, fa_fixed_tone::create(FACompTag::IMD, key, ssb));
                                    key = negate(p + kb + '-' + q + ka, true);
                                    add_comp(keys, comps, key, fa_fixed_tone::create(FACompTag::IMD, key, ssb));
                                }
                            } else {
                                // odd order
                                bool neg = cplx && is_odd(group / 2);
                                key = negate(p + ka + '-' + q + kb, neg);
                                add_comp(keys, comps, key, fa_fixed_tone::create(FACompTag::IMD, key, ssb));
                                key = negate(p + kb + '-' + q + ka, neg);
                                add_comp(keys, comps, key, fa_fixed_tone::create(FACompTag::IMD, key, ssb));
                                if (cplx) {
                                    key = negate(p + ka + '-' + q + kb, !neg);
                                    add_comp(keys, comps, key, fa_fixed_tone::create(FACompTag::IMD, key, ssb));
                                    key = negate(p + kb + '-' + q + ka, !neg);
                                    add_comp(keys, comps, key, fa_fixed_tone::create(FACompTag::IMD, key, ssb));
                                }
                            }
                        } else { // group == order
                            // form: p * kx PLUS q * ky
                            if (is_even(order)) {
                                // even order
                                for (qq = 1; qq < group; ++qq) {
                                    pp = order - qq;
                                    p = (1 == pp) ? "" : std::to_string(pp);
                                    q = (1 == qq) ? "" : std::to_string(qq);
                                    key = p + ka + '+' + q + kb;
                                    add_comp(keys, comps, key, fa_fixed_tone::create(FACompTag::IMD, key, ssb));
                                }
                                if (cplx) {
                                    for (pp = 1; pp < group; ++pp) {
                                        qq = order - pp;
                                        p = (1 == pp) ? "" : std::to_string(pp);
                                        q = (1 == qq) ? "" : std::to_string(qq);
                                        key = negate(p + ka + '+' + q + kb, true);
                                        add_comp(keys, comps, key, fa_fixed_tone::create(FACompTag::IMD, key, ssb));
                                    }
                                }
                            } else {
                                // odd order
                                bool neg = cplx && is_odd(group / 2);
                                for (qq = 1; qq < group; ++qq) {
                                    pp = order - qq;
                                    p = (1 == pp) ? "" : std::to_string(pp);
                                    q = (1 == qq) ? "" : std::to_string(qq);
                                    key = negate(p + ka + '+' + q + kb, neg);
                                    add_comp(keys, comps, key, fa_fixed_tone::create(FACompTag::IMD, key, ssb));
                                }
                                if (find_qe) {
                                    for (pp = 1; pp < group; ++pp) {
                                        qq = order - pp;
                                        p = (1 == pp) ? "" : std::to_string(pp);
                                        q = (1 == qq) ? "" : std::to_string(qq);
                                        key = negate(p + ka + '+' + q + kb, !neg);
                                        add_comp(keys, comps, key, fa_fixed_tone::create(FACompTag::IMD, key, ssb));
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        // WO Components
        ssb = (m_ssb_wo < 0) ? m_ssb_def : m_ssb_wo;
        if (1 == m_wo) {
            add_comp(keys, comps, "wo", fa_wo_tone::create(ssb));
        } else {
            for (int i = 1; i <= m_wo; ++i) {
                key = "wo" + std::to_string(i);
                add_comp(keys, comps, key, fa_wo_tone::create(ssb));
            }
        }
        // Done!
        return comp_data;
    }

    void fourier_analysis::if_key_not_available_throw(const str_t& key) const
    {
        if (is_reserved(key)) {
            throw runtime_error("key '" + key + "' is reserved");
        } else if (is_comp(key) || is_var(key)) {
            throw runtime_error("key '" + key + "' already exists");
        } else if (!is_valid(key)) {
            throw runtime_error("key '" + key + "' is invalid");
        }
    }

} // namespace dcanalysis_impl - Non-Public

namespace dcanalysis_impl { // Analysis and related subroutines

    namespace {

        using comp_index_mag_t = std::pair<diff_t, real_t>;

        enum class FAMask : int {
            AB = 0x1000,    // Analysis Band
            Comp,           // All components
            WO,             // Used to search for worst others
            NAD,            // Noise and distortion
            THD,            // Total harmonic distortion: HD + IMD
            ILV,            // Interleaving: ILOS + ILGT
            Dist,           // Total distortion: THD + UserDist [+ CLK] [+ ILV]
            Noise           // Total noise: NAD - Dist
        };

        real_t alias(real_t freq, real_t fs, bool fold)
        {
            freq -= std::floor(freq / fs) * fs; // freq in [0, fs)
            return (fold && fs < 2 * freq) ? (fs - freq) : freq;
        }

        bool check_args(
            const real_t* msq_data,
            const size_t msq_size,
            const size_t nfft,
            const cplx_t* fft_data,
            const size_t fft_size
            )
        {
            check_array("", "mean-square magnitude array", msq_data, msq_size);
            bool cplx = msq_size == nfft;
            if (!cplx) {
                if (msq_size != nfft / 2 + 1) {
                    throw runtime_error("check_args : data size error");
                }
            }
            if (fft_data) {
                if (msq_size != fft_size) {
                    throw runtime_error("check_args : data size error");
                }
            }
            return cplx;
        }

        fa_tone_results null_tone_results(FACompTag tag)
        {
            fa_tone_results results;
            results.set(FAToneResult::Tag    ,  static_cast<real_t>(tag));
            results.set(FAToneResult::Freq   ,  0.0);
            results.set(FAToneResult::FFinal ,  0.0);
            results.set(FAToneResult::FWAvg  ,  0.0);
            results.set(FAToneResult::InBand ,  0.0);
            results.set(FAToneResult::I1     , -1.0);
            results.set(FAToneResult::I2     , -1.0);
            results.set(FAToneResult::NBins  ,  0.0);
            results.set_mag(0.0);
            return results;
        }

        real_t fa_db10(real_t msq_num, real_t msq_den = 1.0)
        {
            if (msq_num <= 0.0 && msq_den <= 0.0) {
                return 0.0;
            } else if (msq_num <= 0.0) {
                return k_abs_min_db;
            } else if (msq_den <= 0.0) {
                return k_abs_max_db;
            } else {
                return bounded_db10(msq_num / msq_den);
            }
        }

        real_t fa_phase(
            const fa_tone_results& r,
            const cplx_t* fft_data,
            bool cplx,
            real_t fdata,
            real_t fshift
            )
        {
            real_t phase = 0.0;
            if (fft_data && 1 == r.get(FAToneResult::NBins)) {
                diff_t index = static_cast<diff_t>(r.get(FAToneResult::I1));
                phase = std::arg(fft_data[index]);
                if (!cplx) {
                    real_t f = r.get(FAToneResult::Freq) + fshift;
                    f -= std::floor(f / fdata) * fdata; // f in [0, fdata)
                    if (fdata <= 2 * f) {
                        phase *= -1.0; // invert phase if falias is in 2nd NZ
                    }
                }
            }
            return phase;
        }

        diff_p get_lrbins(size_t nfft, bool cplx, real_t cycles, int ssb)
        {
            // Find nearest half cycle:
            real_t nearest_half_cycle = cycles;                         // initialize to exact cycles
            bool on_half_cycle = false;                                 // false: x.0, true: x.5
            if (0 < ssb) {                                              // generate_comps() guarantees 0 <= ssb
                nearest_half_cycle = std::round(nearest_half_cycle * 2.0);
                on_half_cycle = is_odd(static_cast<size_t>(nearest_half_cycle));
                nearest_half_cycle *= 0.5;                              // nearest half cycle (x.0 or x.5)
            } else {
                nearest_half_cycle = std::round(nearest_half_cycle);    // nearest whole cycle (x.0)
            }
            // Re-alias for special cases
            nearest_half_cycle = alias(nearest_half_cycle, static_cast<real_t>(nfft), !cplx);
            // Resolve SSB:
            int min_ssb = std::get<0>(fourier_analysis::mmd_ssb);
            int max_ssb = std::get<1>(fourier_analysis::mmd_ssb);
            max_ssb = static_cast<int>(std::min<size_t>(nfft / 2, max_ssb));
            if (is_even(nfft) && !on_half_cycle) { // Example( NFFT=32, !on_half_cycle ):
                max_ssb -= 1;                      //             ssb=16 -> 1+2*16=33 bins - too many!
            }                                      // subtract 1: ssb=15 -> 1+2*15=31 bins - OK
            ssb = std::clamp(ssb, min_ssb, max_ssb);
            real_t half_width = static_cast<real_t>(ssb);
            if (on_half_cycle) {
                half_width -= 0.5;
            }
            // Left and right bins:
            diff_t lbin = static_cast<diff_t>(nearest_half_cycle - half_width);
            diff_t rbin = static_cast<diff_t>(nearest_half_cycle + half_width);
            return diff_p(lbin, rbin);
        }

        bool has_tone_results(const fourier_analysis_component& c)
        {
            return (FACompType::DC        == c.type)
                || (FACompType::FixedTone == c.type)
                || (FACompType::MaxTone   == c.type)
                || (FACompType::WOTone    == c.type);
        }

        fa_tone_results meas_dc(
            const fa_dc& comp,
            const real_t* msq_data,
            const size_t msq_size,
            const size_t nfft,
            fourier_analysis::mask_map& masks
            )
        {
            const bool cplx = msq_size == nfft;
            const diff_p lrbins = get_lrbins(nfft, cplx, 0.0, comp.ssb);
            fourier_analysis_comp_mask m (cplx, msq_size);
            m.set_range(lrbins.first, lrbins.second);
            masks.at(to_int(FACompTag::DC)) = m;
            masks.at(to_int(FAMask::Comp)) |= m;
            size_t i1, i2, nbins;
            std::tie(i1, i2, nbins) = m.get_indexes();
            real_t fwavg = 0.0; // FIXME: Real: 0.0 by definition; Cplx: calc
            bool inband = masks.at(to_int(FAMask::AB)).overlaps(i1, i2);
            real_t mag2 = m.sum(msq_data, msq_size);
            fa_tone_results results;
            results.set(FAToneResult::Tag    , static_cast<real_t>(FACompTag::DC));
            results.set(FAToneResult::Freq   , 0.0);
            results.set(FAToneResult::FFinal , 0.0);
            results.set(FAToneResult::FWAvg  , fwavg);
            results.set(FAToneResult::I1     , static_cast<real_t>(i1));
            results.set(FAToneResult::I2     , static_cast<real_t>(i2));
            results.set(FAToneResult::NBins  , static_cast<real_t>(nbins));
            results.set(FAToneResult::InBand , inband ? 1.0 : 0.0);
            results.set_mag(mag2);
            return results;
        }

        fa_tone_results meas_fixed_tone(
            const str_t& key,
            const fa_fixed_tone& comp,
            const real_t* msq_data,
            const size_t msq_size,
            const size_t nfft,
            fourier_analysis::mask_map& masks,
            fourier_analysis::var_map& vars
            )
        {
            const bool cplx     = msq_size == nfft;
            const real_t fbin   = vars.at("fbin");
            const real_t fdata  = vars.at("fdata");
            const real_t fshift = vars.at("fshift");
            const real_t freq   = expression(comp.freq).evaluate(vars);     // "actual" frequency
            const real_t ffinal = alias(freq + fshift, fdata, !cplx);       // freq after translation and aliasing
            const diff_p lrbins = get_lrbins(nfft, cplx, (ffinal / fbin), comp.ssb);
            vars[key] = freq;
            fourier_analysis_comp_mask m (cplx, msq_size);
            m.set_range(lrbins.first, lrbins.second);
            masks.at(to_int(comp.tag)) |= m;
            masks.at(to_int(FAMask::Comp)) |= m;
            size_t i1, i2, nbins;
            std::tie(i1, i2, nbins) = m.get_indexes();
            real_t fwavg = 0.0; // FIXME
            bool inband = masks.at(to_int(FAMask::AB)).overlaps(i1, i2);
            real_t mag2 = m.sum(msq_data, msq_size);
            fa_tone_results results;
            results.set(FAToneResult::Tag    , static_cast<real_t>(comp.tag));
            results.set(FAToneResult::Freq   , freq);
            results.set(FAToneResult::FFinal , ffinal);
            results.set(FAToneResult::FWAvg  , fwavg);
            results.set(FAToneResult::InBand , inband ? 1.0 : 0.0);
            results.set(FAToneResult::I1     , static_cast<real_t>(i1));
            results.set(FAToneResult::I2     , static_cast<real_t>(i2));
            results.set(FAToneResult::NBins  , static_cast<real_t>(nbins));
            results.set_mag(mag2);
            return results;
        }

        // May implement search band in future.  Note: search band != analysis band.
        fa_tone_results meas_max_tone(
            const str_t& key,
            const fa_max_tone& comp,
            const real_t* msq_data,
            const size_t msq_size,
            const size_t nfft,
            fourier_analysis::mask_map& masks,
            fourier_analysis::var_map& vars
            )
        {
            fourier_analysis_comp_mask search_mask = masks.at(to_int(FAMask::AB));
            fourier_analysis_comp_mask& comp_mask = masks.at(to_int(FAMask::Comp));
            search_mask.unset_ranges(comp_mask);
            diff_t max_index, lower, upper;
            std::tie(max_index, lower, upper) = search_mask.find_max_index(msq_data, msq_size);
            if (max_index < 0) {
                return null_tone_results(comp.tag);
            }
            const bool cplx = msq_size == nfft;
            const real_t fbin   = vars.at("fbin");
            const real_t fshift = vars.at("fshift");
            const real_t ffinal = max_index * fbin;
            const real_t freq = ffinal - fshift; // this is a best guess
            diff_p lrbins = get_lrbins(nfft, cplx, static_cast<real_t>(max_index), comp.ssb);
            // Unlike FixedTone, MaxTone stops when it runs into another tone.  Since DC is always
            // found and is always at bin 0, we never have to worry about MaxTone wrapping.
            lrbins.first = std::max<diff_t>(lower, lrbins.first);
            lrbins.second = std::min<diff_t>(lrbins.second, upper);
            fourier_analysis_comp_mask m (cplx, msq_size);
            m.set_range(lrbins.first, lrbins.second);
            comp_mask |= m;                     // add this range to component mask
            masks.at(to_int(comp.tag)) |= m;    // add this range to tag mask
            size_t i1, i2, nbins;
            std::tie(i1, i2, nbins) = m.get_indexes();
            real_t fwavg = 0.0; // FIXME
            bool inband = masks.at(to_int(FAMask::AB)).overlaps(i1, i2);
            real_t mag2 = m.sum(msq_data, msq_size);
            vars[key] = freq; // should probably use fwavg (once implemented)
            fa_tone_results results;
            results.set(FAToneResult::Tag    , static_cast<real_t>(comp.tag));
            results.set(FAToneResult::Freq   , freq);
            results.set(FAToneResult::FFinal , ffinal);
            results.set(FAToneResult::FWAvg  , fwavg);
            results.set(FAToneResult::InBand , inband ? 1.0 : 0.0);
            results.set(FAToneResult::I1     , static_cast<real_t>(i1));
            results.set(FAToneResult::I2     , static_cast<real_t>(i2));
            results.set(FAToneResult::NBins  , static_cast<real_t>(nbins));
            results.set_mag(mag2);
            return results;
        }

        fa_tone_results meas_wo_tone(
            const fa_wo_tone& comp,
            const real_t* msq_data,
            const size_t msq_size,
            const size_t nfft,
            const real_t fbin,
            const real_t fshift,
            fourier_analysis::mask_map& masks
            )
        {
            fourier_analysis_comp_mask& wo_mask = masks.at(to_int(FAMask::WO));
            fourier_analysis_comp_mask& comp_mask = masks.at(to_int(FAMask::Comp));
            diff_t max_index, lower, upper;
            std::tie(max_index, lower, upper) = wo_mask.find_max_index(msq_data, msq_size);
            if (max_index < 0) {
                return null_tone_results(comp.tag);
            }
            const bool cplx = msq_size == nfft;
            const real_t ffinal = max_index * fbin;
            const real_t freq = ffinal - fshift; // best guess
            diff_p lrbins = get_lrbins(nfft, cplx, static_cast<real_t>(max_index), comp.ssb);
            // Unlike FixedTone, MaxTone stops when it runs into another tone.  Since DC is always
            // found and is always at bin 0, we never have to worry about MaxTone wrapping.
            lrbins.first = std::max<diff_t>(lower, lrbins.first);
            lrbins.second = std::min<diff_t>(lrbins.second, upper);
            fourier_analysis_comp_mask m (cplx, msq_size);
            m.set_range(lrbins.first, lrbins.second);
            comp_mask |= m;                 // add this range to component mask
            wo_mask.unset_ranges(m);        // remove this range from WO search mask
            size_t i1, i2, nbins;
            std::tie(i1, i2, nbins) = m.get_indexes();
            real_t fwavg = 0.0; // FIXME
            real_t mag2 = m.sum(msq_data, msq_size);
            fa_tone_results results;
            results.set(FAToneResult::Tag    , static_cast<real_t>(comp.tag));
            results.set(FAToneResult::Freq   , freq);
            results.set(FAToneResult::FFinal , ffinal);
            results.set(FAToneResult::FWAvg  , fwavg);
            results.set(FAToneResult::InBand , 1.0); // by definition
            results.set(FAToneResult::I1     , static_cast<real_t>(i1));
            results.set(FAToneResult::I2     , static_cast<real_t>(i2));
            results.set(FAToneResult::NBins  , static_cast<real_t>(nbins));
            results.set_mag(mag2);
            return results;
        }

        void update_maxspur(
            size_t key_index,
            const fa_tone_results& results,
            bool dc_as_dist,
            const fourier_analysis::mask_map& masks,
            comp_index_mag_t& maxspur_im
            )
        {
            // A tone is a candidate for maxspur if:
            //      1. it is in-band and non-Signal (these are assumed)
            //      2. it is not DC, unless DC is treated as distortion (dc_as_dist is true)
            //      3. it does not overlap any Signal tones (this needs explanation)
            if (0.0 != results.get(FAToneResult::FFinal) || dc_as_dist) {
                const fourier_analysis_comp_mask& sig_mask = masks.at(to_int(FACompTag::Signal));
                if (!sig_mask.overlaps(results.i1, results.i2)) {
                    real_t mag2 = results.get(FAToneResult::Mag);
                    mag2 *= mag2;
                    if (maxspur_im.first < 0 || maxspur_im.second < mag2) {
                        maxspur_im.first = static_cast<diff_t>(key_index);
                        maxspur_im.second = mag2;
                    }
                }
            }
        }

        void tone_updates(
            const str_t& key,
            size_t key_index,
            FACompTag tag,
            const fa_tone_results& results,
            bool dc_as_dist,
            fourier_analysis::mask_map& masks,
            const std::set<str_t>& ilos_clk_keys,
            comp_index_mag_t& carrier_im,
            comp_index_mag_t& maxspur_im
            )
        {
            if (ilos_clk_keys.find(key) != ilos_clk_keys.end()) {
                masks.at(to_int(FACompTag::CLK)).set_range(results.i1, results.i2);
            }
            if (results.inband) {
                if (FACompTag::Signal == tag) {
                    real_t mag2 = results.get(FAToneResult::Mag);
                    mag2 *= mag2;
                    if (carrier_im.first < 0 || mag2 < carrier_im.second) {
                        carrier_im.first = static_cast<diff_t>(key_index);
                        carrier_im.second = mag2;
                    }
                } else {
                    update_maxspur(key_index, results, dc_as_dist, masks, maxspur_im);
                }
            }
        }

    } // namespace anonymous

    fourier_analysis::mask_map fourier_analysis::initialize_masks(bool cplx, size_t size)
    {
        mask_map masks {
            {to_int(FAMask::AB)    , fourier_analysis_comp_mask(cplx, size)},
            {to_int(FAMask::Comp)  , fourier_analysis_comp_mask(cplx, size)},
            {to_int(FAMask::WO)    , fourier_analysis_comp_mask(cplx, size)},
            {to_int(FAMask::NAD)   , fourier_analysis_comp_mask(cplx, size)},
            {to_int(FAMask::THD)   , fourier_analysis_comp_mask(cplx, size)},
            {to_int(FAMask::ILV)   , fourier_analysis_comp_mask(cplx, size)},
            {to_int(FAMask::Dist)  , fourier_analysis_comp_mask(cplx, size)},
            {to_int(FAMask::Noise) , fourier_analysis_comp_mask(cplx, size)}
        };
        for (const std::pair<int, str_t>& kv : fa_comp_tag_map) {
            masks.emplace(kv.first, fourier_analysis_comp_mask(cplx, size));
        }
        return masks;
    }

    fourier_analysis_results fourier_analysis::analyze_impl(
        const real_t* msq_data,
        const size_t msq_size,
        const size_t nfft,
        FreqAxisType axis_type,
        const cplx_t* fft_data,
        const size_t fft_size
        ) const
    {
        //
        // Setup
        //
        const bool cplx = check_args(msq_data, msq_size, nfft, fft_data, fft_size);
        mask_map masks = initialize_masks(cplx, msq_size);
        var_map vars = initialize_vars(nfft);
        const real_t fbin    = vars.at("fbin");
        const real_t fdata   = vars.at("fdata");
        const real_t fsample = vars.at("fs");
        const real_t fshift  = vars.at("fshift");
        fourier_analysis_results results;
        //
        // Analysis Band
        // 
        setup_analysis_band(cplx, masks.at(to_int(FAMask::AB)), vars);
        //
        // Component Generation
        //      CLK and ILOS components may overlap.  Since the ILOS tag has higher priority,
        //      generate_comps() creates a dedicated list of CLK keys.
        //
        comp_data_t comp_data                = generate_comps(cplx);
        const str_vector& keys               = std::get<0>(comp_data);
        const comp_map& comps                = std::get<1>(comp_data);
        const std::set<str_t>& ilos_clk_keys = std::get<2>(comp_data);
        //
        // Main Component Loop
        //
        comp_index_mag_t carrier_im {-1, 0.0};
        comp_index_mag_t maxspur_im {-1, 0.0};
        size_t key_index = 0;
        for (; key_index < keys.size(); ++key_index) {
            const str_t& key = keys[key_index];
            const fourier_analysis_component& comp = *comps.at(key);
            switch (comp.type)
            {
            case FACompType::DC : {
                auto& c = static_cast<const fa_dc&>(comp);
                fa_tone_results r = meas_dc(c, msq_data, msq_size, nfft, masks);
                if (r.inband && dc_as_dist) {
                    update_maxspur(key_index, r, dc_as_dist, masks, maxspur_im);
                }
                results.add_tone(key, std::move(r));
                break;
            } case FACompType::FixedTone : {
                auto& c = static_cast<const fa_fixed_tone&>(comp);
                fa_tone_results r = meas_fixed_tone(key, c, msq_data, msq_size, nfft, masks, vars);
                tone_updates(key, key_index, comp.tag, r, dc_as_dist, masks, ilos_clk_keys,
                    carrier_im, maxspur_im);
                results.add_tone(key, std::move(r));
                break;
            } case FACompType::MaxTone : {
                auto& c = static_cast<const fa_max_tone&>(comp);
                fa_tone_results r = meas_max_tone(key, c, msq_data, msq_size, nfft, masks, vars);
                tone_updates(key, key_index, comp.tag, r, dc_as_dist, masks, ilos_clk_keys,
                    carrier_im, maxspur_im);
                results.add_tone(key, std::move(r));
                break;
            } default :
                break;
            }
            if (FACompType::WOTone == comp.type) {
                break;
            }
        }
        //
        // Worst Others
        //      WOs are handled separately in order to guarantee max to min order
        //
        std::multimap<real_t, str_t> wo_mag_map;        // maps WO magnitude to WO key
        std::map<str_t, fa_tone_results> wo_res_map;    // maps WO key to WO results
        fourier_analysis_comp_mask& wo_mask = masks.at(to_int(FAMask::WO));
        wo_mask = masks.at(to_int(FAMask::AB));                   // Initialize WO mask with AB mask
        wo_mask.unset_ranges(masks.at(to_int(FAMask::Comp)));     // Then remove all components already found
        const size_t first_wo_index = key_index;
        for (; key_index < keys.size(); ++key_index) {
            const str_t& key = keys[key_index];
            auto& c = static_cast<const fa_wo_tone&>(*comps.at(key));
            fa_tone_results r = meas_wo_tone(c, msq_data, msq_size, nfft, fbin, fshift, masks);
            wo_mag_map.insert({r.get(FAToneResult::Mag), key});
            wo_res_map[key] = std::move(r);
        }
        // WOs are found.  Now add to results in order from max to min.  Update MaxSpur.
        int wo_num = 0;
        for (auto iter = wo_mag_map.crbegin(); iter != wo_mag_map.crend(); ++iter) {
            const str_t& key = iter->second;
            str_t new_key = "wo";
            if (1 < m_wo) {
                new_key += std::to_string(++wo_num);
            }
            if (1 == m_wo || 1 == wo_num) {
                update_maxspur(first_wo_index, wo_res_map.at(key), dc_as_dist, masks, maxspur_im);
            }
            results.add_tone(new_key, std::move(wo_res_map.at(key)));
        }
        //
        // Second pass for dBc, phase, and phase_c.
        //      If there are no in-band Signal components, there is no Carrier.
        //
        real_t carrier_phase = 0.0;
        if (0 < carrier_im.first) {
            str_t carrier_key = keys[carrier_im.first];
            fa_tone_results& carrier_results = results.tone_results.at(carrier_key);
            carrier_phase = fa_phase(carrier_results, fft_data, cplx, fdata, fshift);
        }
        for (size_t i = 0; i < keys.size(); ++i) {
            const str_t& key = keys[i];
            if (has_tone_results(*comps.at(key))) {
                fa_tone_results& r = results.tone_results.at(key);
                real_t mag = r.get(FAToneResult::Mag);
                real_t phase = fa_phase(r, fft_data, cplx, fdata, fshift);
                r.set(FAToneResult::OrderIndex, static_cast<real_t>(i));
                r.set(FAToneResult::Mag_dBc, fa_db10(mag * mag, carrier_im.second));
                r.set(FAToneResult::Phase, phase); 
                r.set(FAToneResult::Phase_c, phase - carrier_phase);
            }
        }
        //
        // ffinal adjustment (only for complex analysis)
        //
        if (cplx && FreqAxisType::DcCenter == axis_type) {
            for (std::pair<const str_t, fa_tone_results>& kv : results.tone_results) {
                double& ffinal = kv.second.results[FAToneResult::FFinal];
                if (fdata <= 2 * ffinal) {
                    ffinal -= fdata;
                }
            }
        }
        //
        // Components finished, now the rest of the results
        //
        finalize_masks(masks);
        std::tuple<size_t, size_t, size_t> ab_info = masks.at(to_int(FAMask::AB)).get_indexes();
        real_t ab_nbins    = static_cast<real_t>(std::get<2>(ab_info));
        real_t ab_width    = cplx ? ab_nbins * fbin : std::fmin(fdata / 2, ab_nbins * fbin);
        real_t nad_ss      = masks.at(to_int(FAMask::NAD)).sum(msq_data, msq_size);
        real_t noise_ss    = masks.at(to_int(FAMask::Noise)).sum(msq_data, msq_size);
        real_t noise_nbins = masks.at(to_int(FAMask::Noise)).count_r();
        real_t signal_ss   = masks.at(to_int(FACompTag::Signal)).sum(msq_data, msq_size);
        results.set(FAResult::AnalysisType   , static_cast<real_t>(AnalysisType::Fourier));
        results.set(FAResult::SignalType     , cplx ? 1.0 : 0.0);
        results.set(FAResult::NFFT           , static_cast<real_t>(nfft));
        results.set(FAResult::DataSize       , static_cast<real_t>(msq_size));
        results.set(FAResult::FBin           , fbin);
        results.set(FAResult::FData          , fdata);
        results.set(FAResult::FSample        , fsample);
        results.set(FAResult::FShift         , fshift);
        results.set(FAResult::FSNR           , fa_db10(1.0, noise_ss));
        results.set(FAResult::SNR            , fa_db10(signal_ss, noise_ss));
        results.set(FAResult::SINAD          , fa_db10(signal_ss, nad_ss));
        results.set(FAResult::SFDR           , fa_db10(carrier_im.second, maxspur_im.second));
        results.set(FAResult::ABN            , fa_db10(noise_ss / std::fmax(1.0, noise_nbins))); // avoid div by 0
        results.set(FAResult::NSD            , fa_db10(noise_ss / ab_width));
        results.set(FAResult::CarrierIndex   , static_cast<real_t>(carrier_im.first));
        results.set(FAResult::MaxSpurIndex   , static_cast<real_t>(maxspur_im.first));
        results.set(FAResult::AB_Width       , ab_width);
        results.set(FAResult::AB_I1          , static_cast<real_t>(std::get<0>(ab_info)));
        results.set(FAResult::AB_I2          , static_cast<real_t>(std::get<1>(ab_info)));
        results.set(FAResult::AB_NBins       , ab_nbins);
        results.set(FAResult::AB_RSS         , masks.at(to_int(FAMask::AB)).root_sum(msq_data, msq_size));
        results.set(FAResult::Signal_NBins   , masks.at(to_int(FACompTag::Signal)).count_r());
        results.set(FAResult::Signal_RSS     , std::sqrt(signal_ss));
        results.set(FAResult::CLK_NBins      , masks.at(to_int(FACompTag::CLK)).count_r());
        results.set(FAResult::CLK_RSS        , masks.at(to_int(FACompTag::CLK)).root_sum(msq_data, msq_size));
        results.set(FAResult::HD_NBins       , masks.at(to_int(FACompTag::HD)).count_r());
        results.set(FAResult::HD_RSS         , masks.at(to_int(FACompTag::HD)).root_sum(msq_data, msq_size));
        results.set(FAResult::ILOS_NBins     , masks.at(to_int(FACompTag::ILOS)).count_r());
        results.set(FAResult::ILOS_RSS       , masks.at(to_int(FACompTag::ILOS)).root_sum(msq_data, msq_size));
        results.set(FAResult::ILGT_NBins     , masks.at(to_int(FACompTag::ILGT)).count_r());
        results.set(FAResult::ILGT_RSS       , masks.at(to_int(FACompTag::ILGT)).root_sum(msq_data, msq_size));
        results.set(FAResult::IMD_NBins      , masks.at(to_int(FACompTag::IMD)).count_r());
        results.set(FAResult::IMD_RSS        , masks.at(to_int(FACompTag::IMD)).root_sum(msq_data, msq_size));
        results.set(FAResult::UserDist_NBins , masks.at(to_int(FACompTag::UserDist)).count_r());
        results.set(FAResult::UserDist_RSS   , masks.at(to_int(FACompTag::UserDist)).root_sum(msq_data, msq_size));
        results.set(FAResult::THD_NBins      , masks.at(to_int(FAMask::THD)).count_r());
        results.set(FAResult::THD_RSS        , masks.at(to_int(FAMask::THD)).root_sum(msq_data, msq_size));
        results.set(FAResult::ILV_NBins      , masks.at(to_int(FAMask::ILV)).count_r());
        results.set(FAResult::ILV_RSS        , masks.at(to_int(FAMask::ILV)).root_sum(msq_data, msq_size));
        results.set(FAResult::Dist_NBins     , masks.at(to_int(FAMask::Dist)).count_r());
        results.set(FAResult::Dist_RSS       , masks.at(to_int(FAMask::Dist)).root_sum(msq_data, msq_size));
        results.set(FAResult::Noise_NBins    , masks.at(to_int(FAMask::Noise)).count_r());
        results.set(FAResult::Noise_RSS      , std::sqrt(noise_ss));
        results.set(FAResult::NAD_NBins      , masks.at(to_int(FAMask::NAD)).count_r());
        results.set(FAResult::NAD_RSS        , std::sqrt(nad_ss));
        return results;
    }

    // This function needs a detailed explanation.  Don't edit this function unless you know what
    // you are doing!  Even the author wasn't quite sure what he was doing :)
    void fourier_analysis::finalize_masks(mask_map& masks) const
    {
        const fourier_analysis_comp_mask& ab_mask = masks.at(to_int(FAMask::AB));
        // Signal Mask: remove out-of-band Signals
        fourier_analysis_comp_mask& sig_mask = masks.at(to_int(FACompTag::Signal));
        sig_mask &= ab_mask;
        // Noise-And-Distortion Mask: start with AnalysisBand, remove DC and Signals
        fourier_analysis_comp_mask& nad_mask = masks.at(to_int(FAMask::NAD));
        nad_mask = ab_mask;
        if (!dc_as_dist) { // Do not unset DC ranges if DC is treated as distortion
            nad_mask.unset_ranges(masks.at(to_int(FACompTag::DC)));
        }
        nad_mask.unset_ranges(sig_mask);
        // Tag Masks: remove Signals and out-of-band components; by virtue of the NAD mask, also
        // remove DC if it is not treated as distortion
        for (const std::pair<int, str_t>& kv : fa_comp_tag_map) {
            if (to_int(FACompTag::Signal) != kv.first) { // Don't touch the Signal mask!
                masks.at(kv.first) &= nad_mask;
            }
        }
        // THD
        masks.at(to_int(FAMask::THD))   = masks.at(to_int(FACompTag::HD));
        masks.at(to_int(FAMask::THD))  |= masks.at(to_int(FACompTag::IMD));
        // Interleaving
        masks.at(to_int(FAMask::ILV))   = masks.at(to_int(FACompTag::ILOS));
        masks.at(to_int(FAMask::ILV))  |= masks.at(to_int(FACompTag::ILGT));
        // Total Distortion
        masks.at(to_int(FAMask::Dist))  = masks.at(to_int(FAMask::THD));
        masks.at(to_int(FAMask::Dist)) |= masks.at(to_int(FACompTag::UserDist));
        if (dc_as_dist) {
            masks.at(to_int(FAMask::Dist)) |= masks.at(to_int(FACompTag::DC));
        }
        if (!clk_as_noise) {
            masks.at(to_int(FAMask::Dist)) |= masks.at(to_int(FACompTag::CLK));
        }
        if (!ilv_as_noise) {
            masks.at(to_int(FAMask::Dist)) |= masks.at(to_int(FAMask::ILV));
        }
        // Noise
        masks.at(to_int(FAMask::Noise)) = nad_mask;
        masks.at(to_int(FAMask::Noise)).unset_ranges(masks.at(to_int(FAMask::Dist)));
    }

    fourier_analysis::var_map fourier_analysis::initialize_vars(size_t nfft) const
    {
        var_map vars (m_user_vars);
        real_t fsample = expression(m_fsample).evaluate(vars);
        assert_gt0("", "fs", fsample);
        vars["fs"] = fsample;
        real_t fdata = expression(m_fdata).evaluate(vars);
        assert_gt0("", "fdata", fdata);
        real_t fbin = fdata / static_cast<real_t>(nfft);
        vars["fdata"] = fdata;
        vars["fbin"] = fbin;
        real_t fshift = expression(m_fshift).evaluate(vars);
        vars["fshift"] = fshift;
        return vars;
    }

    void fourier_analysis::setup_analysis_band(
        bool cplx, fourier_analysis_comp_mask& mask, var_map& vars) const
    {
        real_t center = expression(m_ab_center).evaluate(vars);
        real_t width  = expression(m_ab_width).evaluate(vars);
        const real_t fbin = vars.at("fbin");
        const real_t fdata = vars.at("fdata");
        width = std::clamp(width, fbin, fdata);     // [ fbin, fdata ]
        width = std::round(width / fbin);           // [    1,  nfft ]
        if (mask.size() == static_cast<size_t>(width)) {
            mask.set_all();
            return;
        }
        center = alias(center, fdata, !cplx);
        center = std::round(2.0 * center / fbin) / 2.0; // nearest half cycle
        center = alias(center, fdata, !cplx);
        diff_t lbin = static_cast<diff_t>(std::ceil( center - width / 2));
        diff_t rbin = static_cast<diff_t>(std::floor(center + width / 2));
        mask.set_range(lbin, rbin);
    }

} // namespace dcanalysis_impl - Non-Public
