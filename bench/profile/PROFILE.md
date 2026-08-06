# Real-pipeline profile: where does genalyzer's complex FFT spend its time?

Goal: before optimizing `reduce_and_scale` (the per-bin `std::arg`/atan2 +
`std::polar`/sincos stage), confirm on the REAL pipeline that it actually
dominates — measure, don't assume.

## Method

`perf` is unavailable here (`perf_event_paranoid=4`, no CAP_PERFMON, can't sudo),
so instead of sampling, the three stages inside the real `genalyzer_impl::fft()`
complex/real_t path were temporarily wrapped with `std::chrono::steady_clock`
accumulators (window apply / FFTW execute / reduce_and_scale), printed at exit.
That instrumentation was reverted after measuring — it is NOT committed to
`src/`. `profile_pipeline.cpp` (this dir) drives the real `fft()` on a
representative complex tone + noise, one (nfft, navg) per process so the at-exit
breakdown is cleanly attributed.

Build/run (library built Release `-O3 -march=native`):

    # build lib with stage timers, then:
    g++ -O3 -std=c++17 -Iinclude bench/profile/profile_pipeline.cpp \
        -o bench/profile/profile_pipeline \
        -Lbuild/bindings/c/src -lgenalyzer -Wl,-rpath,$PWD/build/bindings/c/src
    LD_LIBRARY_PATH=build/bindings/c/src \
        GEN_PROFILE=1 bench/profile/profile_pipeline <nfft> <navg> 1.0

Machine: Xeon E5-1650 @ 3.20 GHz, AVX-256 no-FMA. Measured 2026-06-19.

## Result — `reduce_and_scale` dominates, and is bigger than the FFT itself

Stage share of total `fft()` wall time:

| nfft    | navg | window | FFTW  | reduce_and_scale | reduce vs FFTW |
|---------|------|--------|-------|------------------|----------------|
| 4096    | 8    | 16.6%  | 14.9% | **68.5%**        | 4.6x           |
| 65536   | 8    | 23.2%  | 19.0% | **57.8%**        | 3.0x           |
| 1048576 | 8    | 21.6%  | 31.4% | **47.0%**        | 1.5x           |
| 65536   | 1    | 82.8%  | 17.2% | 0% (scale only)  | —              |
| 1048576 | 1    | 76.5%  | 35.5% | 0% (scale only)  | —              |

Raw per-config stage totals (summed over the timed iterations):

    nfft=4096    navg=8  window=0.1656s fftw=0.1487s reduce=0.6838s (461 calls)
    nfft=65536   navg=8  window=0.2421s fftw=0.1984s reduce=0.6026s (25 calls)
    nfft=1048576 navg=8  window=0.5359s fftw=0.7793s reduce=1.1642s (3 calls)
    nfft=65536   navg=1  window=0.8294s fftw=0.1722s reduce=0.0000s (168 calls)
    nfft=1048576 navg=1  window=0.7653s fftw=0.3555s reduce=0.0000s (11 calls)

## Conclusions

1. **`reduce_and_scale` is the single largest stage whenever navg > 1** — 47-68%
   of the whole transform, i.e. 1.5x-4.6x the cost of the FFT itself. It is only
   present for averaging (navg > 1); navg == 1 uses the cheap `scale_fft` and the
   window becomes the top cost (the stage the earlier window spike attacked).
2. The cost is `std::arg` (atan2) computed `navg * nfft` times plus `std::polar`
   (sincos) `nfft` times — data-dependent transcendentals that, unlike the
   window's, cannot be precomputed away. So this is a genuine compute-bound
   transcendental target where SIMD/asm (vectorized atan2/sincos) would pay off.
3. **Bigger lever first (algorithmic):** the analysis path consumes mean-square
   magnitude (`analyze_impl(const real_t *msq_data ...)`, "phase not available")
   and only pulls phase on-demand at a handful of tone bins via `fa_phase` ->
   `std::arg(fft_data[index])`. So the `navg * nfft` atan2 calls that retain
   phase for every bin look largely wasted. Eliminating/deferring per-bin phase
   would likely beat any SIMD on atan2.

Next step options: (a) extend the optimization-ladder spike to `reduce_and_scale`
(T0 baseline -> drop-phase algorithmic -> vectorized atan2/sincos -> asm ->
multicore); (b) trace whether per-bin phase can be safely deferred to tone bins.
