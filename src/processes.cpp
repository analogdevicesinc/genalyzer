/*
File        : $HeadURL: https://swdev.cld.analog.com/svn/projects/icdev/sandbox/users/pderouni/platform-upgrades/src/analysis/processes.cpp $
Originator  : pderouni
Revision    : $Revision: 12382 $
Last Commit : $Date: 2020-04-06 11:21:16 -0400 (Mon, 06 Apr 2020) $
Last Editor : $Author: pderouni $
*/

#include "processes.hpp"
#include "checks.hpp"
#include "constants.hpp"
#include "dft.hpp"
#include <algorithm>
#include <functional>
#include <random>

namespace analysis {

template <typename T>
void normalize(const T* in_data,
    size_t in_size,
    real_t* out_data,
    size_t out_size,
    int res,
    CodeFormat format)
{
    check_array(in_data, in_size, "input array");
    check_array(out_data, out_size, "output array");
    assert_eq(in_size, "input array size", out_size, "output array size");
    check_resolution<T>(res);
    bool osb = CodeFormat::OffsetBinary == format;
    const real_t norm_factor = 2.0 / (1 << res);
    const real_t fmt_offset = osb ? -1.0 : 0.0;
    for (size_t i = 0; i < in_size; ++i) {
        out_data[i] = norm_factor * in_data[i] + fmt_offset;
    }
}

void polyval(const real_t* in_data,
    size_t in_size,
    real_t* out_data,
    size_t out_size,
    std::vector<real_t> poco)
{
    check_array_pair(in_data, in_size, "input array",
        out_data, out_size, "output array");
    if (poco.empty()) {
        return;
    }
    while (1 < poco.size() && 0.0 == poco.back()) {
        poco.pop_back();
    }
    const real_t last_poco = poco.back();
    poco.pop_back();
    std::reverse(poco.begin(), poco.end());
    for (size_t i = 0; i < in_size; ++i) {
        real_t x = last_poco;
        for (real_t c : poco) {
            x = std::fma(x, in_data[i], c);
        }
        out_data[i] = x;
    }
}

template <typename T>
void quantize(const real_t* in_data,
    size_t in_size,
    T* out_data,
    size_t out_size,
    real_t fsr,
    int res,
    real_t noise,
    CodeFormat format,
    bool null_offset,
    int m,
    const std::vector<real_t>& offset,
    const std::vector<real_t>& gerror)
{
    check_array(in_data, in_size, "input array");
    check_array(out_data, out_size, "output array");
    assert_eq(in_size, "input array size", out_size, "output array size");
    check_fsr(fsr);
    check_resolution<T>(res);
    real_t lsb = fsr / (1 << res);
    real_t min_code = -std::pow(2.0, res - 1);
    real_t max_code = -1.0 - min_code;
    noise = std::fmax(noise, -180.0);
    real_t sd = (fsr / 2) * std::pow(10.0, noise / 20) / k_sqrt2;
    std::random_device rdev;
    std::mt19937 rgen(rdev());
    std::normal_distribution<real_t> rdist{};
    auto os_and_noise = std::bind(std::ref(rdist), rgen);
    double (*func)(double);
    if (null_offset) {
        func = std::round;
    } else {
        func = std::floor;
    }
    if (m < 2) {
        m = 1;
    } else if (256 < m) {
        throw base::exception("Number of interleaved channels exceeds "
                              "limit: 256 < "
            + std::to_string(m));
    }
    size_t nch = static_cast<size_t>(m);
    std::vector<real_t> vos(offset);
    std::vector<real_t> vge(gerror);
    vos.resize(nch, 0.0);
    vge.resize(nch, 0.0);
    for (size_t ch = 0; ch < nch; ++ch) {
        const real_t gn = vge[ch] + 1.0;
        const real_t os = vos[ch];
        rdist = std::normal_distribution<real_t>(os, sd);
        for (size_t i = ch; i < out_size; i += nch) {
            real_t c = std::fma(gn, in_data[i], os_and_noise()) / lsb;
            c = std::fmax(min_code, std::fmin(func(c), max_code));
            out_data[i] = static_cast<T>(c);
        }
    }
    if (CodeFormat::OffsetBinary == format) {
        const int64_t os = static_cast<int64_t>(1) << (res - 1);
        for (size_t i = 0; i < out_size; ++i) {
            int64_t code = static_cast<int64_t>(out_data[i]) + os;
            out_data[i] = static_cast<T>(code);
        }
    }
}

void window(const real_t* in_data,
    size_t in_size,
    real_t* out_data,
    size_t out_size,
    WindowType window)
{
    check_array_pair(in_data, in_size, "input array",
        out_data, out_size, "output array");
    switch (window) {
    case WindowType::BlackmanHarris: {
        const real_t k = 1.9688861870585801;
        const real_t k1 = 1.0 * k_2pi / in_size;
        const real_t k2 = 2.0 * k_2pi / in_size;
        const real_t k3 = 3.0 * k_2pi / in_size;
        for (size_t i = 0; i < in_size; ++i) {
            real_t x = k * (0.35875 - 0.48829 * std::cos(k1 * i) + 0.14128 * std::cos(k2 * i) - 0.01168 * std::cos(k3 * i));
            out_data[i] = in_data[i] * x;
        }
        break;
    }
    case WindowType::Hann: {
        const real_t k = 1.6329922791756648;
        const real_t k1 = k_2pi / in_size;
        for (size_t i = 0; i < in_size; ++i) {
            real_t x = k * (0.5 - 0.5 * std::cos(k1 * i));
            out_data[i] = in_data[i] * x;
        }
        break;
    }
    case WindowType::Rect:
        if (in_data != out_data) {
            std::copy(in_data, in_data + in_size, out_data);
        }
        break;
    default:
        throw base::exception("");
        break;
    }
}

} // namespace analysis

namespace analysis { // Allocating Functions

real_vector downsample(const real_vector& data,
    int m,
    diff_t start,
    size_t size)
{
    if (m < 1) {
        throw base::exception("Factor must be a positive integer");
    }
    auto data_size = static_cast<diff_t>(data.size());
    if (start < 0) {
        start += data_size;
    }
    if (start < 0 || data_size <= start) {
        throw base::exception("Start index out of range");
    }
    auto max_size = static_cast<size_t>(std::ceil(
        static_cast<real_t>(data_size - start) / m));
    size = (0 == size) ? max_size : std::min(size, max_size);
    real_vector wvf(size);
    for (size_t i = 0; i < size; ++i) {
        wvf[i] = data[static_cast<size_t>(start) + i * m];
    }
    return wvf;
}

real_vector_pair downsample(const real_vector_pair& data,
    int m,
    diff_t start,
    size_t size)
{
    const real_vector& idata = data.first;
    const real_vector& qdata = data.second;
    assert_eq(idata.size(), "in-phase array size",
        qdata.size(), "quadrature array size");
    if (m < 1) {
        throw base::exception("Factor must be a positive integer");
    }
    auto data_size = static_cast<diff_t>(idata.size());
    if (start < 0) {
        start += data_size;
    }
    if (start < 0 || data_size <= start) {
        throw base::exception("Start index out of range");
    }
    auto max_size = static_cast<size_t>(std::ceil(
        static_cast<real_t>(data_size - start) / m));
    size = (0 == size) ? max_size : std::min(size, max_size);
    real_vector_pair wvf{ real_vector(size), real_vector(size) };
    real_vector& iwvf = wvf.first;
    real_vector& qwvf = wvf.second;
    for (size_t i = 0; i < size; ++i) {
        size_t j = static_cast<size_t>(start) + i * m;
        iwvf[i] = idata[j];
        qwvf[i] = qdata[j];
    }
    return wvf;
}

void ftrans(real_vector& data, real_t fs, real_t freq)
{
    check_fs(fs);
    const real_t ft = freq / fs;
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] *= std::cos(k_2pi * std::fmod(i * ft, 1.0));
    }
}

void ftrans(real_vector_pair& data, real_t fs, real_t freq)
{
    check_fs(fs);
    real_vector& idata = data.first;
    real_vector& qdata = data.second;
    assert_eq(idata.size(), "in-phase array size",
        qdata.size(), "quadrature array size");
    const real_t ft = freq / fs;
    for (size_t i = 0; i < idata.size(); ++i) {
        cplx_t x{ idata[i], qdata[i] };
        x *= std::exp(cplx_t{ 0.0, k_2pi * std::fmod(i * ft, 1.0) });
        idata[i] = x.real();
        qdata[i] = x.imag();
    }
}

real_vector pshift(const real_vector& data, real_t phase)
{
    cplx_vector _fft = rfft(data);
    phase = std::fmod(phase, k_2pi);
    for (auto& x : _fft) {
        x *= std::exp(cplx_t{ 0.0, phase });
    }
    return irfft(_fft, data.size());
}

real_vector_pair pshift(const real_vector_pair& data, real_t phase)
{
    cplx_vector _fft = fft(data);
    phase = std::fmod(phase, k_2pi);
    for (auto& x : _fft) {
        x *= std::exp(cplx_t{ 0.0, phase });
    }
    return ifft(_fft);
}

} // namespace analysis - Allocating Functions

namespace analysis { // Template Instantiations

template ICD_ANALYSIS_DECL void normalize(const int16_t*, size_t, real_t*, size_t, int, CodeFormat);

template ICD_ANALYSIS_DECL void normalize(const int32_t*, size_t, real_t*, size_t, int, CodeFormat);

template ICD_ANALYSIS_DECL void normalize(const int64_t*, size_t, real_t*, size_t, int, CodeFormat);

template ICD_ANALYSIS_DECL void quantize(const real_t*, size_t, int16_t*, size_t,
    real_t, int, real_t, CodeFormat, bool,
    int, const std::vector<real_t>&, const std::vector<real_t>&);

template ICD_ANALYSIS_DECL void quantize(const real_t*, size_t, int32_t*, size_t,
    real_t, int, real_t, CodeFormat, bool,
    int, const std::vector<real_t>&, const std::vector<real_t>&);

template ICD_ANALYSIS_DECL void quantize(const real_t*, size_t, int64_t*, size_t,
    real_t, int, real_t, CodeFormat, bool,
    int, const std::vector<real_t>&, const std::vector<real_t>&);

} // namespace analyais
