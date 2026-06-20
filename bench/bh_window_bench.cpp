// Standalone benchmark for the genalyzer Blackman-Harris complex-FFT window.
// Build: ./build.sh   Run: ./bh_window_bench
#include <cstddef>
#include <cstdio>
#include <cmath>
#include <vector>
#include <random>
#include <chrono>
#include <string>

using std::size_t;

// ---- Blackman-Harris constants (from src/fourier_transforms.cpp) ----
static const double bh_kx = 1.9688861870585801;
static const double bh_k0 = 0.35875;
static const double bh_k1 = -0.48829;
static const double bh_k2 = 0.14128;
static const double bh_k3 = -0.01168;
static const double k_2pi = 6.283185307179586476925286766559;

using window_fn = void (*)(const double*, const double*, double*,
                           size_t, size_t, size_t);

// ---- T0: baseline, faithful to src/fourier_transforms.cpp:35 ----
// (navg>=2 branches in the original are an unrolled but behaviorally identical
// path; collapsed here into one general loop.)
void bh_window_t0(const double* i_data, const double* q_data, double* out_data,
                  size_t in_stride, size_t navg, size_t nfft) {
    const double scalar = bh_kx;
    const double k1 = k_2pi / (double)nfft;
    const double k2 = k1 * 2;
    const double k3 = k1 * 3;
    const size_t in_row_stride = nfft * in_stride;
    const size_t out_row_stride = nfft * 2;
    size_t i = 0;
    double x = 0.0;
    if (1 == navg) {
        for (size_t j = 0; j < out_row_stride; j += 2) {
            const double w = bh_k0 + bh_k1 * std::cos(k1 * x) +
                             bh_k2 * std::cos(k2 * x) + bh_k3 * std::cos(k3 * x);
            out_data[j] = w * scalar * i_data[i];
            out_data[j + 1] = w * scalar * q_data[i];
            i += in_stride;
            x += 1.0;
        }
    } else {
        for (size_t j = 0; j < out_row_stride; j += 2) {
            const double w = bh_k0 + bh_k1 * std::cos(k1 * x) +
                             bh_k2 * std::cos(k2 * x) + bh_k3 * std::cos(k3 * x);
            const double* pi = i_data;
            const double* pq = q_data;
            double* po = out_data;
            for (size_t k = 0; k < navg; ++k) {
                po[j] = w * scalar * pi[i];
                po[j + 1] = w * scalar * pq[i];
                pi += in_row_stride;
                pq += in_row_stride;
                po += out_row_stride;
            }
            i += in_stride;
            x += 1.0;
        }
    }
}

// ---- Independent reference (different arrangement) to anchor T0 ----
void bh_window_ref(const double* i_data, const double* q_data, double* out,
                   size_t in_stride, size_t navg, size_t nfft) {
    for (size_t k = 0; k < navg; ++k) {
        for (size_t i = 0; i < nfft; ++i) {
            const double x = (double)i;
            const double w = bh_k0 + bh_k1 * std::cos(k_2pi * 1.0 * x / nfft) +
                             bh_k2 * std::cos(k_2pi * 2.0 * x / nfft) +
                             bh_k3 * std::cos(k_2pi * 3.0 * x / nfft);
            const double si = i_data[k * nfft * in_stride + i * in_stride];
            const double sq = q_data[k * nfft * in_stride + i * in_stride];
            out[k * nfft * 2 + 2 * i] = w * bh_kx * si;
            out[k * nfft * 2 + 2 * i + 1] = w * bh_kx * sq;
        }
    }
}

double max_abs_err(const double* a, const double* b, size_t n) {
    double m = 0.0;
    for (size_t i = 0; i < n; ++i) {
        double d = std::fabs(a[i] - b[i]);
        if (d > m) m = d;
    }
    return m;
}

int main() {
    // Tiny anchored self-test: T0 vs the independent reference.
    const size_t nfft = 16, navg = 2, in_stride = 1;
    std::mt19937_64 rng(12345);
    std::uniform_real_distribution<double> dist(-1.0, 1.0);
    std::vector<double> I(nfft * navg), Q(nfft * navg);
    for (size_t n = 0; n < I.size(); ++n) { I[n] = dist(rng); Q[n] = dist(rng); }
    std::vector<double> out0(nfft * navg * 2), outr(nfft * navg * 2);
    bh_window_t0(I.data(), Q.data(), out0.data(), in_stride, navg, nfft);
    bh_window_ref(I.data(), Q.data(), outr.data(), in_stride, navg, nfft);
    double err = max_abs_err(out0.data(), outr.data(), out0.size());
    std::printf("T0 anchor max-abs-err = %.3e -> %s\n", err,
                err < 1e-12 ? "PASS" : "FAIL");
    return err < 1e-12 ? 0 : 1;
}
