#include "processes.hpp"

#include "constants.hpp"
#include "utils.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <random>
#include <vector>

namespace genalyzer_impl {
    
    template<typename T>
    void downsample(
        const T* in_data,
        size_t in_size,
        T* out_data,
        size_t out_size,
        int ratio,
        bool interleaved
        )
    {
        check_array("", "input array", in_data, in_size);
        check_array("", "output array", out_data, out_size);
        size_t out_size_expected = downsample_size(in_size, ratio, interleaved);
        assert_eq("", "output array size", out_size, "expected", out_size_expected);
        size_t i = 0;
        if (interleaved) {
            // check for even in_size ??
            const size_t jump = ratio * 2;
            for (size_t j = 0; j < out_size; ++j) {
                out_data[j] = in_data[i];
                ++j;
                out_data[j] = in_data[i + 1];
                i += jump;
            }
        } else {
            for (size_t j = 0; j < out_size; ++j) {
                out_data[j] = in_data[i];
                i += ratio;
            }
        }
    }

    template void downsample(const real_t* , size_t, real_t* , size_t, int, bool);
    template void downsample(const int16_t*, size_t, int16_t*, size_t, int, bool);
    template void downsample(const int32_t*, size_t, int32_t*, size_t, int, bool);
    template void downsample(const int64_t*, size_t, int64_t*, size_t, int, bool);
    
    size_t downsample_size(size_t in_size, int ratio, bool interleaved)
    {
        assert_gt0("", "downsample ratio", ratio);
        size_t out_size = 0;
        if (interleaved) {
            if (is_odd(in_size)) {
                throw runtime_error("size of interleaved array must be even");
            }
            in_size /= 2;
            out_size = in_size / ratio;
            if (0 < in_size % ratio) {
                out_size += 1;
            }
            out_size *= 2;
        } else {
            out_size = in_size / ratio;
            if (0 < in_size % ratio) {
                out_size += 1;
            }
        }
        return out_size;
    }

    void fshift(
        const real_t* i_data,
        size_t i_size,
        const real_t* q_data,
        size_t q_size,
        real_t* out_data,
        size_t out_size,
        real_t fs,
        real_t _fshift
        )
    {
        assert_gt0("", "fs", fs);
        _fshift -= std::floor(_fshift / fs) * fs; // [0, fs)
        const real_t twopix = k_2pi * _fshift / fs;
        if (0 == q_size) {
            // Interleaved I/Q
            check_array_pair("", "input array", i_data, i_size, "output array", out_data, out_size, true);
            const size_t size = i_size / 2;
            const cplx_t* pin = reinterpret_cast<const cplx_t*>(i_data);
            cplx_t* pout = reinterpret_cast<cplx_t*>(out_data);
            const cplx_t jtwopix = {0.0, twopix};
            cplx_t jtwopix_n = 0.0;
            for (size_t i = 0; i < size; ++i) {
                pout[i] = pin[i] * std::exp(jtwopix_n);
                jtwopix_n += jtwopix;
            }
        } else {
            // Split I/Q
            check_array_pair("", "I array", i_data, i_size, "Q array", q_data, q_size);
            check_array("", "output array", out_data, out_size);
            assert_eq("", "output array size", out_size, "expected", i_size * 2);
            real_t twopix_n = 0.0;
            for (size_t i = 0, j = 0; i < i_size; ++i, j += 2) {
                real_t x = std::cos(twopix_n);
                real_t y = std::sin(twopix_n);
                out_data[j]   = i_data[i] * x - q_data[i] * y;
                out_data[j+1] = i_data[i] * y + q_data[i] * x;
                twopix_n += twopix;
            }
        }
    }

    template<typename T>
    void fshift(
        const T* i_data,
        size_t i_size,
        const T* q_data,
        size_t q_size,
        T* out_data,
        size_t out_size,
        int n,
        real_t fs,
        real_t _fshift,
        CodeFormat format
        )
    {
        resolution_to_minmax<T>(n, format);
        assert_gt0("", "fs", fs);
        _fshift -= std::floor(_fshift / fs) * fs; // [0, fs)
        const real_t twopix = k_2pi * _fshift / fs;
        size_t in_stride = 0;
        if (0 == q_size) {
            // Interleaved I/Q
            check_array("", "input array", i_data, i_size, true);
            i_size /= 2;
            q_size = i_size;
            q_data = i_data + 1;
            in_stride = 2;
        } else {
            // Split I/Q
            check_array_pair("", "I array", i_data, i_size, "Q array", q_data, q_size);
            in_stride = 1;
        }
        check_array("", "output array", out_data, out_size);
        assert_eq("", "output array size", out_size, "expected", i_size + q_size);
        real_t twopix_n = 0.0;
        const real_t min_code = -std::pow(2.0, n - 1);
        const real_t max_code = -1.0 - min_code;
        const real_t os = (CodeFormat::OffsetBinary == format) ? -min_code : 0.0;
        for (size_t i = 0, j = 0; j < out_size; i += in_stride, j += 2) {
            real_t x = std::cos(twopix_n);
            real_t y = std::sin(twopix_n);
            real_t itmp1 = static_cast<real_t>(i_data[i]) - os;
            real_t qtmp1 = static_cast<real_t>(q_data[i]) - os;
            real_t itmp2 = std::clamp(std::round(itmp1 * x - qtmp1 * y), min_code, max_code);
            real_t qtmp2 = std::clamp(std::round(itmp1 * y + qtmp1 * x), min_code, max_code);
            out_data[j]   = static_cast<T>(itmp2 + os);
            out_data[j+1] = static_cast<T>(qtmp2 + os);
            twopix_n += twopix;
        }
    }
    
    template void fshift(const int16_t*, size_t, const int16_t*, size_t, int16_t*, size_t, int, real_t, real_t, CodeFormat);
    template void fshift(const int32_t*, size_t, const int32_t*, size_t, int32_t*, size_t, int, real_t, real_t, CodeFormat);
    template void fshift(const int64_t*, size_t, const int64_t*, size_t, int64_t*, size_t, int, real_t, real_t, CodeFormat);
    
    size_t fshift_size(size_t i_size, size_t q_size)
    {
        if (0 == q_size) {
            // Input I contains Interleaved I/Q; Input Q is unused
            if (is_odd(i_size)) {
                throw runtime_error("size of interleaved array must be even");
            }
            return i_size;
        } else {
            // Split I/Q
            assert_eq("", "I size", i_size, "Q size", q_size);
            return i_size + q_size;
        }
    }
    
    template<typename T>
    void normalize(
        const T* in_data,
        size_t in_size,
        real_t* out_data,
        size_t out_size,
        int n,
        CodeFormat format
        )
    {
        check_array_pair("normalize : ", "input array", in_data, in_size, "output array", out_data, out_size);
        check_code_width("normalize : ", n);
        const real_t scalar = 2.0 / (1 << n);
        if (CodeFormat::OffsetBinary == format) {
            for (size_t i = 0; i < out_size; ++i) {
                out_data[i] = std::fma(scalar, static_cast<real_t>(in_data[i]), -1.0);
            }
        } else {
            for (size_t i = 0; i < out_size; ++i) {
                out_data[i] = scalar * static_cast<real_t>(in_data[i]);
            }
        }
    }

    template void normalize(const int16_t*, size_t, real_t*, size_t, int, CodeFormat);
    template void normalize(const int32_t*, size_t, real_t*, size_t, int, CodeFormat);
    template void normalize(const int64_t*, size_t, real_t*, size_t, int, CodeFormat);
    
    void polyval(
        const real_t* in_data,
        size_t in_size,
        real_t* out_data,
        size_t out_size,
        const real_t* c_data,
        size_t c_size
        )
    {
        check_array_pair("polyval : ", "input array", in_data, in_size, "output array", out_data, out_size);
        check_array("polyval : ", "coefficient array", c_data, c_size);
        std::vector<real_t> c (c_data, c_data + c_size);
        while (1 < c.size() && 0.0 == c.back()) {
            c.pop_back();
        }
        const real_t last_c = c.back();
        c.pop_back();
        if (c.empty()) {
            // y = c0
            std::fill(out_data, out_data + out_size, last_c);
        } else if (1 == c.size()) {
            // y = c1 * x + c0
            const real_t c0 = c[0];
            for (size_t i = 0; i < out_size; ++i) {
                out_data[i] = std::fma(last_c, in_data[i], c0);
            }
        } else {
            std::reverse(c.begin(), c.end());
            // Horner's method
            // 3rd degree example:
            // y = x * (x * (x * c3 + c2) + c1) + c0
            for (size_t i = 0; i < out_size; ++i) {
                real_t tmp = last_c;
                for (real_t cn : c) {
                    tmp = std::fma(in_data[i], tmp, cn);
                }
                out_data[i] = tmp;
            }
        }
    }

    template<typename T>
    void quantize(
        const real_t* in_data,
        size_t in_size,
        T* out_data,
        size_t out_size,
        real_t fsr,
        int n,
        real_t noise,
        CodeFormat format
        )
    {
        const char* trace = "quantize : ";
        check_array_pair(trace, "input array", in_data, in_size, "output array", out_data, out_size);
        assert_gt0(trace, "fsr", fsr);
        resolution_to_minmax<T>(n, format);
        const real_t lsb = fsr / (1 << n);
        const real_t min_code = -std::pow(2.0, n - 1);
        const real_t max_code = -1.0 - min_code;
        const real_t os = (CodeFormat::OffsetBinary == format) ? -min_code : 0.0;
        if (0.0 == noise) {
            for (size_t i = 0; i < out_size; ++i) {
                real_t c = std::floor(in_data[i] / lsb);
                c = std::clamp(c, min_code, max_code);
                out_data[i] = static_cast<T>(c + os);
            }
        } else {
            std::random_device rdev;
            std::mt19937 rgen (rdev());
            auto ngen = std::bind(std::normal_distribution<real_t>(0.0, std::fabs(noise)), rgen);
            for (size_t i = 0; i < out_size; ++i) {
                real_t c = std::floor((in_data[i] + ngen()) / lsb);
                c = std::clamp(c, min_code, max_code);
                out_data[i] = static_cast<T>(c + os);
            }
        }
    }

    template void quantize(const real_t*, size_t, int16_t*, size_t, real_t, int, real_t, CodeFormat);
    template void quantize(const real_t*, size_t, int32_t*, size_t, real_t, int, real_t, CodeFormat);
    template void quantize(const real_t*, size_t, int64_t*, size_t, real_t, int, real_t, CodeFormat);

} // namespace genalyzer_impl