// Profiling driver for the REAL genalyzer fft() pipeline.
// Calls genalyzer_impl::fft (complex, real_t path: window -> FFTW ->
// reduce_and_scale) on a representative complex tone, timing the whole
// transform. The library is built with TEMP stage-instrumentation that prints
// per-stage wall-clock at process exit when env GEN_PROFILE is set.
//
// Run ONE config per process so the at-exit stage breakdown is attributed to
// exactly that (nfft, navg).
//
//   usage: profile_pipeline <nfft> <navg> [seconds]
#include "enums.hpp"
#include "fourier_transforms.hpp"

#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <vector>

using namespace genalyzer_impl;

int main(int argc, char **argv) {
    if (argc < 3) {
        std::fprintf(stderr, "usage: %s <nfft> <navg> [seconds]\n", argv[0]);
        return 2;
    }
    const size_t nfft = std::strtoull(argv[1], nullptr, 10);
    const size_t navg = std::strtoull(argv[2], nullptr, 10);
    const double min_sec = (argc > 3) ? std::strtod(argv[3], nullptr) : 1.0;

    const size_t isize = nfft * navg;
    std::vector<double> I(isize), Q(isize), out(nfft * 2);
    std::mt19937_64 rng(7);
    std::uniform_real_distribution<double> noise(-1e-2, 1e-2);
    const double f = 0.123; // a tone, normalized freq
    for (size_t n = 0; n < isize; ++n) {
        I[n] = std::cos(2 * M_PI * f * (double)n) + noise(rng);
        Q[n] = std::sin(2 * M_PI * f * (double)n) + noise(rng);
    }

    using clk = std::chrono::steady_clock;
    // warm-up (also builds FFTW plan, primes caches) -- not timed
    fft(I.data(), isize, Q.data(), isize, out.data(), nfft * 2, navg, nfft,
        Window::BlackmanHarris);

    long iters = 0;
    double elapsed = 0.0;
    auto t0 = clk::now();
    while (elapsed < min_sec) {
        fft(I.data(), isize, Q.data(), isize, out.data(), nfft * 2, navg, nfft,
            Window::BlackmanHarris);
        ++iters;
        elapsed = std::chrono::duration<double>(clk::now() - t0).count();
    }
    const double per_ms = elapsed / iters * 1e3;
    const double msps = (double)nfft * navg / (elapsed / iters) / 1e6;
    std::printf("nfft=%zu navg=%zu  iters=%ld  per_call=%.4f ms  %.1f Msamp/s\n",
                nfft, navg, iters, per_ms, msps);
    // The TEMP stage-profile line prints to stderr at exit (GEN_PROFILE set).
    return 0;
}
