// Standalone benchmark for the genalyzer Blackman-Harris complex-FFT window.
// Build: ./build.sh   Run: ./bh_window_bench
#include <cstddef>
#include <cstdio>
#include <cmath>
#include <vector>
#include <random>
#include <chrono>
#include <string>
#include <algorithm>

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

// Generate sw[i] = bh_kx * window(i), i = 0..nfft-1, with NO per-sample
// transcendentals: advance three complex phasors (for the 1st/2nd/3rd harmonic)
// and renormalize every 1024 samples to bound floating-point drift.
void gen_scaled_window(double* sw, size_t nfft) {
    const double th = k_2pi / (double)nfft;
    const double cz1 = std::cos(th),     sz1 = std::sin(th);
    const double cz2 = std::cos(2 * th), sz2 = std::sin(2 * th);
    const double cz3 = std::cos(3 * th), sz3 = std::sin(3 * th);
    double c1 = 1, s1 = 0, c2 = 1, s2 = 0, c3 = 1, s3 = 0;
    for (size_t i = 0; i < nfft; ++i) {
        sw[i] = bh_kx * (bh_k0 + bh_k1 * c1 + bh_k2 * c2 + bh_k3 * c3);
        double nc1 = c1 * cz1 - s1 * sz1, ns1 = c1 * sz1 + s1 * cz1; c1 = nc1; s1 = ns1;
        double nc2 = c2 * cz2 - s2 * sz2, ns2 = c2 * sz2 + s2 * cz2; c2 = nc2; s2 = ns2;
        double nc3 = c3 * cz3 - s3 * sz3, ns3 = c3 * sz3 + s3 * cz3; c3 = nc3; s3 = ns3;
        if (((i + 1) & 1023) == 0) {
            double x = (double)(i + 1);
            c1 = std::cos(th * x);     s1 = std::sin(th * x);
            c2 = std::cos(2 * th * x); s2 = std::sin(2 * th * x);
            c3 = std::cos(3 * th * x); s3 = std::sin(3 * th * x);
        }
    }
}

// T1: precompute scaled window once, then scalar apply across all navg rows.
void bh_window_t1(const double* i_data, const double* q_data, double* out_data,
                  size_t in_stride, size_t navg, size_t nfft) {
    static thread_local std::vector<double> sw;
    sw.resize(nfft);
    gen_scaled_window(sw.data(), nfft);
    const size_t in_row_stride = nfft * in_stride;
    const size_t out_row_stride = nfft * 2;
    for (size_t k = 0; k < navg; ++k) {
        const double* pi = i_data + k * in_row_stride;
        const double* pq = q_data + k * in_row_stride;
        double* po = out_data + k * out_row_stride;
        for (size_t i = 0; i < nfft; ++i) {
            po[2 * i] = sw[i] * pi[i * in_stride];
            po[2 * i + 1] = sw[i] * pq[i * in_stride];
        }
    }
}

int main() {
    const size_t in_stride = 1;
    struct Tier { const char* name; window_fn fn; };
    Tier tiers[] = {
        {"T0 baseline", bh_window_t0},
        {"T1 no-trig ", bh_window_t1},
    };

    // Anchor T0 against the independent reference on a small case.
    {
        const size_t nfft = 16, navg = 2;
        std::mt19937_64 rng(12345);
        std::uniform_real_distribution<double> dist(-1.0, 1.0);
        std::vector<double> I(nfft * navg), Q(nfft * navg);
        for (size_t n = 0; n < I.size(); ++n) { I[n] = dist(rng); Q[n] = dist(rng); }
        std::vector<double> a(nfft * navg * 2), b(nfft * navg * 2);
        bh_window_t0(I.data(), Q.data(), a.data(), in_stride, navg, nfft);
        bh_window_ref(I.data(), Q.data(), b.data(), in_stride, navg, nfft);
        double e = max_abs_err(a.data(), b.data(), a.size());
        std::printf("anchor: T0 vs ref max-abs-err = %.3e -> %s\n", e,
                    e < 1e-12 ? "PASS" : "FAIL");
        if (!(e < 1e-12)) return 1;
    }

    // Correctness: every tier vs T0 on a representative case.
    {
        const size_t nfft = 65536, navg = 8;
        std::mt19937_64 rng(777);
        std::uniform_real_distribution<double> dist(-1.0, 1.0);
        std::vector<double> I(nfft * navg), Q(nfft * navg);
        for (size_t n = 0; n < I.size(); ++n) { I[n] = dist(rng); Q[n] = dist(rng); }
        std::vector<double> ref(nfft * navg * 2), got(nfft * navg * 2);
        bh_window_t0(I.data(), Q.data(), ref.data(), in_stride, navg, nfft);
        for (const Tier& t : tiers) {
            std::fill(got.begin(), got.end(), 0.0);
            t.fn(I.data(), Q.data(), got.data(), in_stride, navg, nfft);
            double e = max_abs_err(ref.data(), got.data(), ref.size());
            std::printf("correctness: %s max-abs-err = %.3e -> %s\n",
                        t.name, e, e < 1e-9 ? "PASS" : "FAIL");
            if (!(e < 1e-9)) return 1;
        }
    }
    return 0;
}
