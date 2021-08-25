/*
File        : $HeadURL: https://swdev.cld.analog.com/svn/projects/icdev/sandbox/users/pderouni/platform-upgrades/src/analysis/code_density.cpp $
Originator  : pderouni
Revision    : $Revision: 12382 $
Last Commit : $Date: 2020-04-06 11:21:16 -0400 (Mon, 06 Apr 2020) $
Last Editor : $Author: pderouni $
*/

#include "code_density.hpp"
#include "checks.hpp"
#include "constants.hpp"
#include "enum_maps.hpp"
#include <algorithm>
#include <numeric>

namespace analysis {

namespace {

    using size_p = std::pair<size_t, size_t>;
    using int64_p = std::pair<int64_t, int64_t>;

    // dnl_data expected to contain the cumulative histogram
    size_p xpoints_ramp(real_t* dnl_data, size_t size)
    {
        // DNL limits
        // - find index of median hits (the 2-quantile)
        const real_t median_hits = dnl_data[size - 1] * 0.5;
        size_t median_index = 0;
        while (dnl_data[median_index] < median_hits) {
            ++median_index;
        }
        // - find index of first half-percentile
        size_t i1 = median_index;
        const real_t h1 = dnl_data[size - 1] * 0.005;
        while (0 < i1 && h1 < dnl_data[i1]) {
            --i1;
        }
        // - find index of last half-percentile
        size_t i2 = median_index;
        const real_t h2 = dnl_data[size - 1] * 0.995;
        while (i2 < size - 1 && dnl_data[i2] < h2) {
            ++i2;
        }
        // Code transition points
        const real_t k = 1.0 / dnl_data[size - 1];
        for (size_t i = 0; i < size; ++i) {
            dnl_data[i] *= k;
        }
        return std::make_pair(i1 + 1, i2 - 1);
    }

    // dnl_data expected to contain the cumulative histogram
    size_p xpoints_tone(const int64_t* hits_data,
        real_t* dnl_data,
        size_t size)
    {
        // DNL limits
        // - find index of median hits
        size_t median_index = 0;
        const real_t median_hits = dnl_data[size - 1] * 0.5;
        while (dnl_data[median_index] < median_hits) {
            ++median_index;
        }
        // - find indexes of histogram peaks
        size_p peak_indexes{ median_index, median_index };
        int64_p peak_hits(hits_data[median_index],
            hits_data[median_index]);
        for (size_t i = 0; i < median_index; ++i) { // left peak
            if (peak_hits.first < hits_data[i]) {
                peak_hits.first = hits_data[i];
                peak_indexes.first = i;
            }
        }
        for (size_t i = median_index + 1; i < size; ++i) { // right peak
            if (peak_hits.second < hits_data[i]) {
                peak_hits.second = hits_data[i];
                peak_indexes.second = i;
            }
        }
        if (!(peak_indexes.first < median_index && median_index < peak_indexes.second)) {
            throw base::exception("Data does not match statistical "
                                  "profile of a sine wave");
        }
        // Code transition points
        const real_t k = k_pi / dnl_data[size - 1];
        for (size_t i = 0; i < size; ++i) {
            dnl_data[i] = -std::cos(k * dnl_data[i]);
        }
        return std::make_pair(peak_indexes.first + 1,
            peak_indexes.second - 1);
    }

} // namespace anonymous

void dnl(const int64_t* hits_data, size_t hits_size,
    real_t* dnl_data, size_t dnl_size,
    PmfType pmf)
{
    check_array(hits_data, hits_size, "code hits array");
    check_array(dnl_data, dnl_size, "DNL array");
    assert_eq(hits_size, "code hits array size",
        dnl_size, "DNL array size");
    try {
        // Histogram -> cumulative histogram
        std::partial_sum(hits_data, hits_data + hits_size, dnl_data);
        if (dnl_data[dnl_size - 1] <= 0.0) {
            throw base::exception("Histogram is invalid");
        }
        // Cumulative histogram -> code transition points
        size_p limits{};
        switch (pmf) {
        case PmfType::Ramp:
            limits = xpoints_ramp(dnl_data, dnl_size);
            break;
        case PmfType::Tone:
            limits = xpoints_tone(hits_data, dnl_data, dnl_size);
            break;
        default:
            throw base::exception("DNL not implemented for "
                + pmf_type_map.qual_name(pmf));
        }
        const size_t first = limits.first;
        const size_t last = limits.second;
        if (!(0 < first && first < last && last < dnl_size - 1)) {
            throw base::exception("Computed invalid DNL limits");
        }
        // Code transition points -> code widths
        // The 'width' of a code is the difference between adjacent
        // code transition points
        real_t sum = 0.0;
        for (size_t i = last; first <= i; --i) {
            dnl_data[i] -= dnl_data[i - 1];
            sum += dnl_data[i];
        }
        real_t num = static_cast<real_t>(last - first) + 1;
        const real_t avg_code_width = sum / num;
        if (avg_code_width <= 0.0) {
            throw base::exception("Error computing average code width");
        }
        // Zero out code widths in ranges [0, first) and (last, size)
        for (size_t i = 0; i < first; ++i) {
            dnl_data[i] = 0.0;
        }
        for (size_t i = last + 1; i < dnl_size; ++i) {
            dnl_data[i] = 0.0;
        }
        // Code widths -> DNL
        for (size_t i = 0; i < dnl_size; ++i) {
            dnl_data[i] = dnl_data[i] / avg_code_width - 1.0;
        }
    } catch (const std::exception&) {
        std::fill(dnl_data, dnl_data + dnl_size, -1.0);
        throw;
    }
}

template <typename T>
void histogram(const T* in_data, size_t in_size,
    int32_t* bins_data, size_t bins_size,
    int64_t* hits_data, size_t hits_size,
    int64_t min_code,
    int64_t max_code)
{
    check_array(in_data, in_size, "input array");
    check_array(bins_data, bins_size, "code bins array");
    check_array(hits_data, hits_size, "code hits array");
    size_t max_bins_size = static_cast<size_t>(1) << 31;
    int64_t abs_max_code = (static_cast<int64_t>(1) << 31) - 1;
    int64_t abs_min_code = -(static_cast<int64_t>(1) << 30);
    assert_eq(bins_size, "code bins array size",
        hits_size, "code hits array size");
    assert_le(bins_size, "code bins array size",
        max_bins_size, "library limit");
    assert_le(max_code, "max code", abs_max_code, "library limit");
    assert_le(abs_min_code, "library limit", min_code, "min code");
    assert_le(min_code, "min code", max_code, "max code");
    assert_eq(bins_size, "code bins array size",
        static_cast<size_t>(max_code - min_code) + 1,
        "max_code - min_code + 1");
    int32_t bin = static_cast<int32_t>(min_code);
    for (size_t i = 0; i < bins_size; ++i) {
        bins_data[i] = bin++;
        hits_data[i] = 0;
    }
    for (size_t i = 0; i < in_size; ++i) {
        int64_t code = static_cast<int64_t>(in_data[i]);
        if (code < min_code || max_code < code) {
            continue;
        }
        ++hits_data[static_cast<size_t>(code - min_code)];
    }
}

// Ordinary Least Squares:
// https://en.wikipedia.org/wiki/Ordinary_least_squares
void inl(const real_t* dnl_data, size_t dnl_size,
    real_t* inl_data, size_t inl_size,
    bool fit)
{
    check_array_pair(dnl_data, dnl_size, "DNL array",
        inl_data, inl_size, "INL array");
    size_t first = 0;
    size_t last = dnl_size - 1;
    // Find index of first non-missing code, searching left to right
    while (first < dnl_size && dnl_data[first] <= -1.0) {
        inl_data[first++] = 0.0;
    }
    // Find index of last non-missing code, search right to left
    while (first <= last && dnl_data[last] <= -1.0) {
        inl_data[last--] = 0.0;
    }
    if (first == last) {
        return;
    }
    // INL
    std::partial_sum(dnl_data + first,
        dnl_data + last + 1,
        inl_data + first);
    if (fit) {
        // Compute best fit line and adjust INL
        real_t n = static_cast<real_t>((last - first) + 1);
        real_t sx = 0.0, sy = 0.0, sxx = 0.0, sxy = 0.0;
        for (size_t i = first; i <= last; ++i) {
            real_t x = static_cast<real_t>(i);
            sx += x;
            sy += inl_data[i];
            sxx += x * x;
            sxy += x * inl_data[i];
        }
        real_t m = (n * sxy - sx * sy) / (n * sxx - sx * sx);
        real_t b = (sy - m * sx) / n;
        for (size_t i = first; i <= last; ++i) {
            inl_data[i] -= m * static_cast<real_t>(i) + b;
        }
    }
}

} // namespace analysis

namespace analysis { // Template Instantiations

template ICD_ANALYSIS_DECL void histogram(const int16_t*, size_t, int32_t*, size_t, int64_t*, size_t,
    int64_t, int64_t);

template ICD_ANALYSIS_DECL void histogram(const int32_t*, size_t, int32_t*, size_t, int64_t*, size_t,
    int64_t, int64_t);

template ICD_ANALYSIS_DECL void histogram(const int64_t*, size_t, int32_t*, size_t, int64_t*, size_t,
    int64_t, int64_t);

} // namespace analyais
