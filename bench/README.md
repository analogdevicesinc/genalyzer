# Blackman-Harris Window Optimization-Ladder Benchmark

Standalone spike measuring the speed ceiling of the genalyzer Blackman-Harris
complex-FFT window on this AVX-256 (no-FMA) Xeon E5-1650, 6 cores.

## Build & run
    ./build.sh
    ./bh_window_bench

`build.sh` compiles with `g++ -O3 -mavx -std=c++17 -pthread` (g++ 13.3). The
`-O3 -mavx` flags matter: the "T2/T3 add nothing over T1" finding below assumes
the scalar tiers are already optimized and AVX is actually emitted. Numbers were
measured on an otherwise-idle Xeon E5-1650 @ 3.20 GHz.

## Tiers
- T0 baseline  — std::cos x3 per sample (copy of src/fourier_transforms.cpp:35)
- T1 no-trig   — precomputed window via phasor recurrence, scalar apply
- T2 avx-intr  — AVX-256 intrinsics apply
- T3 asm-1core — hand-tuned AVX-256 assembly apply
- T4 asm-6core — assembly apply split across 6 cores

## Results

```
tier              nfft   navg   Msamples/s    speedup  max-abs-err
T0 baseline       4096      1         16.0      1.00x     0.00e+00
T1 no-trig        4096      1        153.1      9.59x     3.60e-14
T2 avx-intr       4096      1        154.3      9.66x     3.60e-14
T3 asm-1core      4096      1        155.2      9.71x     3.60e-14
T4 asm-6core      4096      1         17.5      1.10x     3.60e-14

T0 baseline       4096      8         97.0      1.00x     0.00e+00
T1 no-trig        4096      8        487.2      5.02x     3.61e-14
T2 avx-intr       4096      8        488.2      5.04x     3.61e-14
T3 asm-1core      4096      8        491.3      5.07x     3.61e-14
T4 asm-6core      4096      8        219.0      2.26x     3.61e-14

T0 baseline      65536      1         18.7      1.00x     0.00e+00
T1 no-trig       65536      1        149.3      8.00x     3.35e-14
T2 avx-intr      65536      1        149.3      7.99x     3.35e-14
T3 asm-1core     65536      1        149.7      8.01x     3.35e-14
T4 asm-6core     65536      1        112.3      6.01x     3.35e-14

T0 baseline      65536      8         46.4      1.00x     0.00e+00
T1 no-trig       65536      8        315.5      6.80x     3.55e-14
T2 avx-intr      65536      8        304.8      6.57x     3.55e-14
T3 asm-1core     65536      8        310.8      6.70x     3.55e-14
T4 asm-6core     65536      8        478.3     10.31x     3.55e-14

T0 baseline    1048576      1         15.5      1.00x     0.00e+00
T1 no-trig     1048576      1        122.6      7.93x     5.67e-14
T2 avx-intr    1048576      1        122.6      7.93x     5.67e-14
T3 asm-1core   1048576      1        122.1      7.90x     5.67e-14
T4 asm-6core   1048576      1        139.8      9.04x     5.67e-14

T0 baseline    1048576      8         46.5      1.00x     0.00e+00
T1 no-trig     1048576      8        273.4      5.88x     5.79e-14
T2 avx-intr    1048576      8        269.6      5.80x     5.79e-14
T3 asm-1core   1048576      8        271.3      5.84x     5.79e-14
T4 asm-6core   1048576      8        473.6     10.19x     5.79e-14
```

## Attribution

Almost the entire single-core speedup comes from eliminating transcendentals: replacing three `std::cos` calls per sample with phasor recurrence (T0 → T1) delivers roughly 8–10x depending on nfft and navg. The AVX-256 intrinsics (T1 → T2) and hand-tuned assembly (T2 → T3) add virtually nothing beyond T1 on this hardware — both measure within noise of T1 at every configuration — which indicates the bottleneck after killing the transcendentals is memory bandwidth, not arithmetic throughput. The navg=1 vs. navg=8 contrast confirms this: at navg=1 the window-generation pass (still scalar, still memory-bound) dominates the total time and the apply-side tiers (T2, T3) cannot separate from T1 at all, while at navg=8 the single window array is reused across eight rows, shifting the apply phase toward compute-bound territory — yet even there T2 and T3 match T1 within a percent or two, showing the scalar loop is already bandwidth-saturated. The 6-core thread tier (T4) exposes the expected thread-spawn overhead at small nfft: at nfft=4096/navg=1 it measures only 1.10x vs T0 (slower than T1–T3) and at nfft=4096/navg=8 it is 2.26x vs T0 (well below the single-core T1 of 5.02x), because fresh thread creation on Linux costs ~10–30 µs each and overwhelms the tiny work quanta. At large nfft (65536 or 1048576) with navg=8 the thread overhead amortises and T4 achieves 10.3x and 10.2x respectively, approaching the theoretical 6-core memory-bandwidth ceiling relative to the scalar T0 baseline.
