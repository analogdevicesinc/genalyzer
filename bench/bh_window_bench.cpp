// Standalone benchmark for the genalyzer Blackman-Harris complex-FFT window.
// Build: ./build.sh   Run: ./bh_window_bench
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cmath>
#include <vector>
#include <random>
#include <chrono>
#include <string>
#include <algorithm>
#include <thread>
#include <immintrin.h>
#ifndef __AVX__
#error "This benchmark requires AVX (build with -mavx)."
#endif

using std::size_t;

extern "C" void bh_apply_asm(const double* sw, const double* I,
                             const double* Q, double* out, size_t nfft);

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
    // Scratch buffer reused only to avoid reallocation; the window is
    // regenerated every call by design (we measure per-call generate+apply
    // cost, as the library does). NOT a cross-call cache.
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

// T2: shared precomputed window, AVX-256 apply. Processes 4 samples/iter.
// For samples i..i+3: ri = sw*I, rq = sw*Q, then interleave to
// out = r0,q0,r1,q1,r2,q2,r3,q3. nfft assumed multiple of 4.
void bh_window_t2(const double* i_data, const double* q_data, double* out_data,
                  size_t in_stride, size_t navg, size_t nfft) {
    assert(in_stride == 1 && "T2 AVX path assumes unit stride");
    static thread_local std::vector<double> sw;
    sw.resize(nfft);
    gen_scaled_window(sw.data(), nfft);
    const size_t in_row_stride = nfft * in_stride;  // in_stride==1 in workload
    const size_t out_row_stride = nfft * 2;
    for (size_t k = 0; k < navg; ++k) {
        const double* pi = i_data + k * in_row_stride;
        const double* pq = q_data + k * in_row_stride;
        double* po = out_data + k * out_row_stride;
        for (size_t i = 0; i < nfft; i += 4) {
            __m256d w  = _mm256_loadu_pd(sw.data() + i);
            __m256d vi = _mm256_loadu_pd(pi + i);
            __m256d vq = _mm256_loadu_pd(pq + i);
            __m256d ri = _mm256_mul_pd(w, vi);   // r0 r1 r2 r3
            __m256d rq = _mm256_mul_pd(w, vq);   // q0 q1 q2 q3
            __m256d lo = _mm256_unpacklo_pd(ri, rq);  // r0 q0 r2 q2
            __m256d hi = _mm256_unpackhi_pd(ri, rq);  // r1 q1 r3 q3
            __m256d o0 = _mm256_permute2f128_pd(lo, hi, 0x20); // r0 q0 r1 q1
            __m256d o1 = _mm256_permute2f128_pd(lo, hi, 0x31); // r2 q2 r3 q3
            _mm256_storeu_pd(po + 2 * i, o0);
            _mm256_storeu_pd(po + 2 * i + 4, o1);
        }
    }
}

// T3: shared precomputed window, assembly apply kernel, single core.
void bh_window_t3(const double* i_data, const double* q_data, double* out_data,
                  size_t in_stride, size_t navg, size_t nfft) {
    assert(in_stride == 1 && "T3 asm path assumes unit stride");
    static thread_local std::vector<double> sw;
    sw.resize(nfft);
    gen_scaled_window(sw.data(), nfft);
    const size_t in_row_stride = nfft * in_stride;
    const size_t out_row_stride = nfft * 2;
    for (size_t k = 0; k < navg; ++k) {
        bh_apply_asm(sw.data(), i_data + k * in_row_stride,
                     q_data + k * in_row_stride, out_data + k * out_row_stride,
                     nfft);
    }
}

// T4: precompute window once, then run the asm apply kernel on disjoint
// row-chunks across NTHREADS physical cores.
// NOTE: T4 spawns fresh std::threads per call (no thread pool). Thread creation
// is ~10-30us each on Linux, so for small nfft (e.g. 4096) with navg=1 the
// spawn overhead can exceed the actual apply work and make T4 measure SLOWER
// than the single-core T3. The multicore win shows up at large nfft / large navg.
static const unsigned NTHREADS = 6;  // E5-1650 physical cores
void bh_window_t4(const double* i_data, const double* q_data, double* out_data,
                  size_t in_stride, size_t navg, size_t nfft) {
    assert(in_stride == 1 && "T4 asm path assumes unit stride");
    static thread_local std::vector<double> sw;
    sw.resize(nfft);
    gen_scaled_window(sw.data(), nfft);
    const size_t in_row_stride = nfft * in_stride;
    const size_t out_row_stride = nfft * 2;
    // Snapshot sw.data() before spawning threads: thread_local statics are NOT
    // captured by [&] -- each thread resolves them via its own TLS slot, so
    // worker threads would see their own (empty) sw.  Use a plain pointer.
    const double* sw_ptr = sw.data();

    auto apply_rows = [&, sw_ptr](size_t k0, size_t k1) {
        for (size_t k = k0; k < k1; ++k)
            bh_apply_asm(sw_ptr, i_data + k * in_row_stride,
                         q_data + k * in_row_stride,
                         out_data + k * out_row_stride, nfft);
    };

    if (navg >= NTHREADS) {
        // Partition by rows.
        std::vector<std::thread> ts;
        size_t per = (navg + NTHREADS - 1) / NTHREADS;
        for (unsigned t = 0; t < NTHREADS; ++t) {
            size_t k0 = t * per, k1 = std::min(navg, k0 + per);
            if (k0 >= k1) break;
            ts.emplace_back(apply_rows, k0, k1);
        }
        for (auto& th : ts) th.join();
    } else {
        // Few rows: partition the single/large row's sample range instead.
        std::vector<std::thread> ts;
        size_t chunk = ((nfft / NTHREADS) / 4) * 4;  // keep multiple of 4
        if (chunk == 0) chunk = nfft;
        for (size_t k = 0; k < navg; ++k) {
            for (size_t i0 = 0; i0 < nfft; i0 += chunk) {
                size_t n = std::min(chunk, nfft - i0);
                const double* pi = i_data + k * in_row_stride + i0 * in_stride;
                const double* pq = q_data + k * in_row_stride + i0 * in_stride;
                double* po = out_data + k * out_row_stride + 2 * i0;
                const double* swp = sw_ptr + i0;
                ts.emplace_back([=] { bh_apply_asm(swp, pi, pq, po, n); });
                if (ts.size() == (size_t)NTHREADS) { for (auto& th : ts) th.join(); ts.clear(); }
            }
        }
        for (auto& th : ts) th.join();
    }
}

// Best Msamples/s over repeated runs totaling >= min_sec (warm-up discarded).
double bench_msps(window_fn fn, const double* I, const double* Q, double* out,
                  size_t in_stride, size_t navg, size_t nfft, double min_sec) {
    fn(I, Q, out, in_stride, navg, nfft);  // warm-up
    using clk = std::chrono::steady_clock;
    double best = 0.0, elapsed = 0.0;
    auto t0 = clk::now();
    const double samples = (double)nfft * (double)navg;
    while (elapsed < min_sec) {
        auto a = clk::now();
        fn(I, Q, out, in_stride, navg, nfft);
        auto b = clk::now();
        double dt = std::chrono::duration<double>(b - a).count();
        double msps = samples / dt / 1e6;
        if (msps > best) best = msps;
        elapsed = std::chrono::duration<double>(b - t0).count();
    }
    return best;
}

int main() {
    const size_t in_stride = 1;
    struct Tier { const char* name; window_fn fn; };
    Tier tiers[] = {
        {"T0 baseline", bh_window_t0},
        {"T1 no-trig ", bh_window_t1},
        {"T2 avx-intr", bh_window_t2},
        {"T3 asm-1core", bh_window_t3},
        {"T4 asm-6core", bh_window_t4},
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

    // Correctness (renorm coverage): nfft not a multiple of 1024 exercises the
    // phasor renormalization branch and its partial tail.
    {
        const size_t nfft = 3000, navg = 1;
        std::mt19937_64 rng(99);
        std::uniform_real_distribution<double> dist(-1.0, 1.0);
        std::vector<double> I(nfft * navg), Q(nfft * navg);
        for (size_t n = 0; n < I.size(); ++n) { I[n] = dist(rng); Q[n] = dist(rng); }
        std::vector<double> ref(nfft * navg * 2), got(nfft * navg * 2);
        bh_window_t0(I.data(), Q.data(), ref.data(), in_stride, navg, nfft);
        for (const Tier& t : tiers) {
            std::fill(got.begin(), got.end(), 0.0);
            t.fn(I.data(), Q.data(), got.data(), in_stride, navg, nfft);
            double e = max_abs_err(ref.data(), got.data(), ref.size());
            std::printf("renorm-cov:  %s max-abs-err = %.3e -> %s\n",
                        t.name, e, e < 1e-9 ? "PASS" : "FAIL");
            if (!(e < 1e-9)) return 1;
        }
    }

    // ---- Timing sweep ----
    struct Cfg { size_t nfft, navg; };
    Cfg cfgs[] = {
        {4096, 1}, {4096, 8},
        {65536, 1}, {65536, 8},
        {1048576, 1}, {1048576, 8},
    };
    std::printf("\n%-12s %9s %6s %12s %10s %12s\n",
                "tier", "nfft", "navg", "Msamples/s", "speedup", "max-abs-err");
    std::mt19937_64 rng(2024);
    std::uniform_real_distribution<double> dist(-1.0, 1.0);
    for (const Cfg& c : cfgs) {
        std::vector<double> I(c.nfft * c.navg), Q(c.nfft * c.navg);
        for (size_t n = 0; n < I.size(); ++n) { I[n] = dist(rng); Q[n] = dist(rng); }
        std::vector<double> ref(c.nfft * c.navg * 2), got(c.nfft * c.navg * 2);
        bh_window_t0(I.data(), Q.data(), ref.data(), in_stride, c.navg, c.nfft);
        double base_msps = 0.0;
        for (const Tier& t : tiers) {
            std::fill(got.begin(), got.end(), 0.0);
            t.fn(I.data(), Q.data(), got.data(), in_stride, c.navg, c.nfft);
            double e = max_abs_err(ref.data(), got.data(), ref.size());
            double msps = bench_msps(t.fn, I.data(), Q.data(), got.data(),
                                     in_stride, c.navg, c.nfft, 0.5);
            if (&t == &tiers[0]) base_msps = msps;
            std::printf("%-12s %9zu %6zu %12.1f %9.2fx %12.2e\n",
                        t.name, c.nfft, c.navg, msps, msps / base_msps, e);
        }
        std::printf("\n");
    }
    return 0;
}
