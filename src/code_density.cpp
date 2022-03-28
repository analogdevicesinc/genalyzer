#include "code_density.hpp"

#include "constants.hpp"
#include "exceptions.hpp"
#include "reductions.hpp"
#include "utils.hpp"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <utility>

namespace dcanalysis_impl {
    
    namespace {

        // Returns first and last non-zero indices
        size_p first_and_last_nz(const uint64_t* data, size_t size)
        {
            size_t first = 0;
            while (0 == data[first]) {
                ++first;
                if (first == size) {
                    return size_p(size, size); // no nz bins
                }
            }
            size_t last = size - 1;
            while (0 == data[last] && first < last) {
                --last;
            }
            return size_p(first, last);
        }

        void dnl_ramp(const uint64_t* hist_data, real_t* dnl_data, size_t size)
        {
            const size_p nz = first_and_last_nz(hist_data, size);
            real_t sum = 0.0;
            for (size_t i = nz.first + 1; i < nz.second; ++i) { // exclude first and last NZ bins
                sum += static_cast<real_t>(hist_data[i]);
            }
            if (0.0 == sum) {
                std::fill(dnl_data, dnl_data + size, -1.0);
                return;
            }
            const real_t avg_count = sum / (static_cast<real_t>(nz.second - nz.first) - 1.0);
            std::fill(dnl_data, dnl_data + nz.first + 1, -1.0);
            for (size_t i = nz.first + 1; i < nz.second; ++i) {
                dnl_data[i] = hist_data[i] / avg_count - 1.0;
            }
            std::fill(dnl_data + nz.second, dnl_data + size, -1.0);
        }

        void dnl_tone(const uint64_t* hist_data, real_t* dnl_data, size_t size)
        {
            // 0. Check for minimum data
            const size_p nz = first_and_last_nz(hist_data, size);
            if (!(nz.first + 1 < nz.second)) {
                std::fill(dnl_data, dnl_data + size, -1.0);
                return;
            }
            // dnl_data stores the result of each of the following steps.
            // 1a. Cumulative histogram
            dnl_data[0] = static_cast<real_t>(hist_data[0]);
            for (size_t i = 1; i < size; ++i) {
                dnl_data[i] = dnl_data[i - 1] + static_cast<real_t>(hist_data[i]);
            }
            const real_t total_count = dnl_data[size - 1];
            // 1b. Find histogram peaks
            size_t median_index = 0;
            const real_t median_count = total_count * 0.5;
            while (dnl_data[median_index] < median_count) {
                ++median_index;
            }
            // find left peak in [nz.first, median_index]
            size_t left_peak_index = nz.first;
            uint64_t peak = hist_data[left_peak_index];
            for (size_t i = nz.first + 1; i <= median_index; ++i) {
                if (peak < hist_data[i]) {  // Note the less-than: if multiple bins contain the
                    peak = hist_data[i];    // peak count, get the leftmost
                    left_peak_index = i;
                }
            }
            // find right peak in [median_index, nz.second]
            size_t right_peak_index = median_index;
            peak = hist_data[right_peak_index];
            for (size_t i = median_index; i <= nz.second; ++i) {
                if (peak <= hist_data[i]) { // Note the less-than-or-equal: if multiple bins
                    peak = hist_data[i];    // contain the same peak count, get the rightmost
                    right_peak_index = i;
                }
            }
            // Ensure plausible results:
            if (!(left_peak_index < median_index && median_index < right_peak_index)) {
                std::fill(dnl_data, dnl_data + size, -1.0);
                throw runtime_error("dnl : unable to locate histogram peaks");
            }
            // 2. Code transition points
            const real_t k1 = k_pi / total_count;
            for (size_t i = 0; i < size; ++i) {
                dnl_data[i] = -std::cos(k1 * dnl_data[i]);
            }
            // 3. Code widths
            size_t left_dnl_index = left_peak_index;
            if (nz.first == left_dnl_index) {   // exclude first NZ bin
                left_dnl_index += 1;            // certain: 0 < left_dnl_index <= median_index
            }
            size_t right_dnl_index = right_peak_index;
            if (nz.second == right_dnl_index) { // exclude last NZ bin
                right_dnl_index -= 1;           // certain: median_index <= right_peak_index < size - 1
            }
            real_t code_width_sum = 0.0;
            // width of code n is the difference between transition points of n and n-1
            for (size_t i = right_dnl_index; left_dnl_index <= i; --i) {
                dnl_data[i] -= dnl_data[i - 1];
                code_width_sum += dnl_data[i];
            }
            size_t num_dnl_codes = (right_dnl_index - left_dnl_index) + 1;
            const real_t avg_code_width = code_width_sum / static_cast<real_t>(num_dnl_codes);
            if (avg_code_width <= 0.0) { // not sure how this can happen, but just in case...
                throw runtime_error("dnl : avg_code_width <= 0.0");
            }
            std::fill(dnl_data, dnl_data + left_dnl_index, 0.0);
            std::fill(dnl_data + right_dnl_index + 1, dnl_data + size, 0.0);
            // 4. DNL
            const real_t k2 = 1 / avg_code_width;
            for (size_t i = 0; i < size; ++i) {
                dnl_data[i] = std::fma(k2, dnl_data[i], -1.0);
            }
        }

    } // namespace anonymous

    size_t code_density_size(int n, CodeFormat format)
    {
        std::pair<int64_t, int64_t> mm = resolution_to_minmax<int64_t>(n, format);
        return code_densityx_size(mm.first, mm.second);
    }

    size_t code_densityx_size(int64_t min, int64_t max)
    {
        str_t trace = "code_densityx_size : ";
        if (max < min) {
            throw runtime_error(trace + "max < min");
        }
        const int64_t abs_max_range = static_cast<int64_t>(1) << k_abs_max_code_width;
        const int64_t abs_min = -(abs_max_range >> 1);
        const int64_t abs_max = abs_max_range - 1;
        if (min < abs_min) {
            throw runtime_error(trace + "min < absolute min");
        }
        if (abs_max < max) {
            throw runtime_error(trace + "absolute max < max");
        }
        // now it is safe to calculate range
        const int64_t range = (max - min) + 1;
        if (abs_max_range < range) {
            throw runtime_error(trace + "range exceeds absolute max");
        }
        return static_cast<size_t>(range);
    }

    void code_axis(real_t* data, size_t size, int n, CodeFormat format)
    {
        std::pair<int64_t, int64_t> mm = resolution_to_minmax<int64_t>(n, format);
        code_axisx(data, size, mm.first, mm.second);
    }

    void code_axisx(real_t* data, size_t size, int64_t min, int64_t max)
    {
        const char* trace = "code_axisx : ";
        check_array(trace, "array", data, size);
        size_t size_expected = code_densityx_size(min, max);
        assert_eq(trace, "array size", size, "expected", size_expected);
        real_t x = static_cast<real_t>(min);
        for (size_t i = 0; i < size; ++i) {
            data[i] = x;
            x += 1.0;
        }
    }

    void dnl(
        real_t* dnl_data,
        size_t dnl_size,
        const uint64_t* hist_data,
        size_t hist_size,
        DnlSignal type
        )
    {
        check_array_pair("dnl : ", "hist array", hist_data, hist_size, "dnl array", dnl_data, dnl_size);
        switch (type)
        {
            case DnlSignal::Ramp:
                dnl_ramp(hist_data, dnl_data, dnl_size);
                return;
            case DnlSignal::Tone:
                dnl_tone(hist_data, dnl_data, dnl_size);
                return;
            default:
                throw runtime_error("dnl : DNL not implemented for signal type");
        }
    }

    std::map<str_t, real_t> dnl_analysis(const real_t* data, size_t size)
    {
        check_array("", "dnl array", data, size);
        // First and last non-missing codes
        size_t first_nm_index = 0;
        while (first_nm_index < size) {
            if (-1.0 < data[first_nm_index++]) {
                break;
            }
        }
        size_t last_nm_index = size - 1;
        real_t num_codes = 0.0;
        if (first_nm_index < size) { // if there are any non-missing codes
            while (first_nm_index < last_nm_index) {
                if (-1.0 < data[last_nm_index--]) {
                    break;
                }
            }
            num_codes = 1.0 + static_cast<real_t>(last_nm_index - first_nm_index);
        } else {
            last_nm_index = first_nm_index;
        }
        // Results
        std_reduce_t r (size);
        real_t avg = 0.0;
        real_t rms = 0.0;
        if (0.0 < num_codes) {
            r = std_reduce(data, size, first_nm_index, last_nm_index + 1);
            avg = r.sum / num_codes;
            rms = std::sqrt(r.sumsq / num_codes);
        } else {
            r.min = -1.0;
            r.max = -1.0;
            avg = -1.0;
            rms = 1.0;
        }
        std::vector<str_t> keys = dnl_analysis_ordered_keys();
        return std::map<str_t, real_t> {
            { keys[0] , r.min },
            { keys[1] , r.max },
            { keys[2] , avg },
            { keys[3] , rms },
            { keys[4] , static_cast<real_t>(r.min_index) },
            { keys[5] , static_cast<real_t>(r.max_index) },
            { keys[6] , static_cast<real_t>(first_nm_index) },
            { keys[7] , static_cast<real_t>(last_nm_index) },
            { keys[8] , num_codes }};
    }

    const std::vector<str_t>& dnl_analysis_ordered_keys()
    {
        // first_nm_index, last_nm_index
        static const std::vector<str_t> keys {
            "min",                  // min DNL value
            "max",                  // max DNL value
            "avg",                  // average DNL value
            "rms",                  // RMS DNL value
            "min_index",            // index of min DNL value
            "max_index",            // index of max DNL value
            "first_nm_index",       // index of first non-missing code
            "last_nm_index",        // index of last non-missing code
            "nm_range"              // non-missing code range
            };
        return keys;
    }

    template<typename T>
    void hist(
        uint64_t* hist_data,
        size_t hist_size,
        const T* wf_data,
        size_t wf_size,
        int n,
        CodeFormat format,
        bool preserve
        )
    {
        std::pair<int64_t, int64_t> mm = resolution_to_minmax<int64_t>(n, format);
        histx(hist_data, hist_size, wf_data, wf_size, mm.first, mm.second, preserve);
    }

    template void hist(uint64_t*, size_t, const int16_t*, size_t, int, CodeFormat, bool);
    template void hist(uint64_t*, size_t, const int32_t*, size_t, int, CodeFormat, bool);
    template void hist(uint64_t*, size_t, const int64_t*, size_t, int, CodeFormat, bool);

    template<typename T>
    void histx(
        uint64_t* hist_data,
        size_t hist_size,
        const T* wf_data,
        size_t wf_size,
        int64_t min,
        int64_t max,
        bool preserve
        )
    {
        check_array("histx : ", "hist array", hist_data, hist_size);
        check_array("histx : ", "waveform array", wf_data, wf_size);
        size_t size_expected = code_densityx_size(min, max);
        assert_eq("histx : ", "hist array size", hist_size, "expected", size_expected);
        if (!preserve) {
            std::fill(hist_data, hist_data + hist_size, 0);
        }
        for (size_t i = 0; i < wf_size; ++i) {
            if (min <= wf_data[i] && wf_data[i] <= max) {
                ++hist_data[wf_data[i] - min];
            }
        }
    }

    template void histx(uint64_t*, size_t, const int16_t*, size_t, int64_t, int64_t, bool);
    template void histx(uint64_t*, size_t, const int32_t*, size_t, int64_t, int64_t, bool);
    template void histx(uint64_t*, size_t, const int64_t*, size_t, int64_t, int64_t, bool);

    std::map<str_t, real_t> hist_analysis(const uint64_t* data, size_t size)
    {
        check_array("", "hist array", data, size);
        // Cummulative Histogram
        std::vector<real_t> chist (size);
        chist[0] = static_cast<real_t>(data[0]);
        for (size_t i = 1; i < size; ++i) {
            chist[i] = chist[i - 1] + static_cast<real_t>(data[i]);
        }
        // First and last non-zero indexes
        size_t first_nz_index = 0;
        while (first_nz_index < size) {
            if (0 < data[first_nz_index++]) {
                break;
            }
        }
        size_t last_nz_index = size - 1;
        real_t num_bins = 0.0;
        if (first_nz_index < size) { // if there are any non-zero bins
            while (first_nz_index < last_nz_index) {
                if (0 < data[last_nz_index--]) {
                    break;
                }
            }
            num_bins = 1.0 + static_cast<real_t>(last_nz_index - first_nz_index);
        } else {
            last_nz_index = first_nz_index;
        }
        // Results
        std::vector<str_t> keys = hist_analysis_ordered_keys();
        return std::map<str_t, real_t> {
            { keys[0] , chist.back() },
            { keys[1] , static_cast<real_t>(first_nz_index) },
            { keys[2] , static_cast<real_t>(last_nz_index) },
            { keys[3] , num_bins }};
    }

    const std::vector<str_t>& hist_analysis_ordered_keys()
    {
        static const std::vector<str_t> keys {
            "sum",                  // total histogram hits
            "first_nz_index",       // index of first non-zero bin
            "last_nz_index",        // index of last non-zero bin
            "nz_range"              // non-zero bin range
            };
        return keys;
    }

    void inl(
        real_t* inl_data,
        size_t inl_size,
        const real_t* dnl_data,
        size_t dnl_size,
        InlLineFit fit
        )
    {
        check_array_pair("inl : ", "dnl array", dnl_data, dnl_size, "inl array", inl_data, inl_size);
        size_t first = 0;
        while (dnl_data[first] <= -1.0) {
            inl_data[first++] = 0.0;
            if (first == dnl_size) {
                return; // all missing codes
            }
        }
        size_t last = dnl_size - 1;
        while (dnl_data[last] <= -1.0 && first < last) {
            inl_data[last--] = 0.0;
        }
        std::partial_sum(dnl_data + first, dnl_data + last + 1, inl_data + first);
        real_t m = 0.0;
        real_t b = 0.0;
        switch (fit)
        {
            case InlLineFit::NoFit:
                return;
            case InlLineFit::BestFit: {
                // https://en.wikipedia.org/wiki/Ordinary_least_squares
                real_t n = static_cast<real_t>(last - first) + 1.0;
                real_t sx = 0.0;
                real_t sy = 0.0;
                real_t sxx = 0.0;
                real_t sxy = 0.0;
                for (size_t i = first; i <= last; ++i) {
                    real_t x = static_cast<real_t>(i);
                    sx += x;
                    sy += inl_data[i];
                    sxx += x * x;
                    sxy += x * inl_data[i];
                }
                m = (n * sxy - sx * sy) / (n * sxx - sx * sx);
                b = (sy - m * sx) / n;
                break;
            } case InlLineFit::EndFit: {
                m = (inl_data[last] - inl_data[first]) / static_cast<real_t>(last - first);
                b = inl_data[first] - m * static_cast<real_t>(first);
                break;
            } default:
                throw runtime_error("inl : line fit not implemented");
        }
        for (size_t i = first; i <= last; ++i) {
            inl_data[i] -= m * static_cast<real_t>(i) + b;
        }
    }

    std::map<str_t, real_t> inl_analysis(const real_t* data, size_t size)
    {
        check_array("", "inl array", data, size);
        std_reduce_t r = std_reduce(data, size, 0, size);
        std::vector<str_t> keys = inl_analysis_ordered_keys();
        return std::map<str_t, real_t> {
            { keys[0] , r.min },
            { keys[1] , r.max },
            { keys[2] , static_cast<real_t>(r.min_index) },
            { keys[3] , static_cast<real_t>(r.max_index) }};
    }

    const std::vector<str_t>& inl_analysis_ordered_keys()
    {
        static const std::vector<str_t> keys {
            "min",          // min INL value
            "max",          // max INL value
            "min_index",    // index of min INL value
            "max_index"     // index of max INL value
            };
        return keys;
    }

} // namespace dcanalysis_impl