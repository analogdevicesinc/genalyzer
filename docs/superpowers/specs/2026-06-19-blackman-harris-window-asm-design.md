# Blackman-Harris Window Optimization-Ladder Benchmark — Design

**Date:** 2026-06-19
**Status:** Approved design, ready for implementation plan
**Goal:** Exploration / "how fast can we make it" — a research spike to find the
speed ceiling of the spectral-analysis window function via hand-tuned assembly.
Throwaway, non-portable code is acceptable; the deliverable is *knowledge*
(an upper-bound speedup number with per-technique attribution), not merged
library code.

## Scope

In scope: the **Blackman-Harris window, complex-FFT path** only — the function at
`src/fourier_transforms.cpp:35`:

```cpp
void blackman_harris(const real_t *i_data, const real_t *q_data,
        real_t *out_data, size_t in_stride, size_t navg, size_t nfft);
```

It produces interleaved I/Q output where, for sample index `i` and output index
`j = 2i`:

```
w        = bh_k0 + bh_k1*cos(k1*x) + bh_k2*cos(2*k1*x) + bh_k3*cos(3*k1*x)
out[j]   = w * scalar * i_data[i*in_stride]
out[j+1] = w * scalar * q_data[i*in_stride]
```

with `x = 0,1,...,nfft-1`, `k1 = 2*pi/nfft`, and the window coefficients/scalar
defined in `fourier_transforms.cpp`. The same `w` multiplies both I and Q. For
`navg > 1` the *same* window vector applies to every averaged row.

Out of scope: the real-FFT path, Hann, normalized/quantized window variants, the
magnitude-reduction loop, the FFT itself (FFTW), and the analysis metrics. The
harness is structured so the magnitude-reduction loop could be added later, but
that is not part of this spike.

## Key insight driving the design

The window coefficients depend **only** on the bin index and `nfft`. They are
independent of the input data and identical across all `navg` rows. The baseline
nonetheless recomputes three `std::cos` calls per sample on every call. So the
optimization ladder has real rungs *before* assembly: eliminating the
transcendentals and precomputing the window vector once. The assembly tier is the
top rung, and the harness attributes the speedup to each technique.

## Hardware target

Intel Xeon E5-1650 (Sandy Bridge-E): **AVX-256 (4 doubles wide), no AVX2, no FMA,
no AVX-512**, 6 physical cores / 12 threads. All SIMD and assembly tiers target
AVX-256 without FMA — that is this machine's ceiling. `real_t = double`.

## Architecture

A standalone benchmark executable in a new top-level `bench/` directory, built
**independently** of the library via a `build.sh` one-liner (no CMake wiring, no
changes to `src/`). Each optimization tier is a self-contained function with the
identical signature to the real one. The harness:

1. Generates random I/Q input once (fixed seed) plus the output buffers.
2. Runs every tier on identical input.
3. Verifies each tier's output against T0 (max abs/rel error).
4. Times each tier (best-of-N, warm-up discarded).
5. Prints a results table and a short attribution summary.

## Optimization tiers

- **T0 — Baseline.** Current `src/` code copied verbatim (`std::cos` x3 per
  sample). Reference for correctness and speedup.
- **T1 — Algorithmic, no transcendentals.** Generate the length-`nfft` window
  vector once via a cosine recurrence
  (`cos((n+1)θ) = 2cosθ·cos(nθ) − cos((n−1)θ)`, tracking the three harmonics),
  then apply it. Reuse the window vector across all `navg` rows. Pure portable
  C++.
- **T2 — SIMD intrinsics.** AVX-256 (`__m256d`, 4 doubles/iteration, no FMA).
  Vectorized window-apply (`w*scalar*data`); window generation precomputed
  (T1-style) or polynomial-vectorized.
- **T3 — Hand-tuned assembly, single core.** AVX-256 in a separate `.S`
  translation unit — manual register allocation, loop unrolling, addressing
  modes. The single-core ceiling.
- **T4 — Hand-tuned assembly, multithreaded.** T3 partitioned across the 6
  physical cores (e.g. `std::thread` splitting the sample range, calling the T3
  asm kernel per chunk). The overall ceiling.

## Workload

Sweep over representative configurations:

- `nfft ∈ {4096, 65536, 1048576}`
- `navg ∈ {1, 8}` (navg=1 isolates per-sample window cost; navg=8 exercises the
  window-vector reuse-across-rows win)
- `in_stride = 1`

Each (tier, config) runs enough iterations to total ≥ ~0.5 s of wall time;
best-of-N reported to suppress noise.

## Correctness

Each tier's output is compared elementwise against T0. Report **max absolute
error** and **max relative error** per config. A tier passes if max-abs-error
< `1e-9`. Bit-exactness is neither expected nor required — the recurrence and
SIMD/asm reorderings introduce sub-ULP-scale floating-point differences. (T4 must
produce the same result as T3 regardless of thread count / partition boundaries.)

## Measurement & reporting

- Monotonic clock (`std::chrono::steady_clock` or `clock_gettime(CLOCK_MONOTONIC)`).
- One warm-up iteration per (tier, config), discarded.
- Single-core tiers (T0–T3) pinned to one core for stable numbers; T4 uses all 6.
- Output table columns: `tier | nfft | navg | Msamples/s | speedup vs T0 |
  max-abs-err`.
- Followed by a one-paragraph attribution summary: how much speed came from
  killing transcendentals (T1), from SIMD (T2), from hand assembly (T3), and from
  threads (T4).

## File layout

```
bench/
  build.sh              # standalone g++ build, no CMake, no src/ changes
  bh_window_bench.cpp   # harness + T0, T1, T2 tiers, correctness, timing, report
  bh_window_asm.S       # T3 / T4 AVX-256 assembly kernel (separate TU)
  README.md             # how to build/run + captured results table
```

## Success criteria

- All tiers produce correct output (max-abs-err < 1e-9 vs T0) across all configs.
- A results table with per-tier Msamples/s and speedup-vs-baseline is produced.
- The attribution summary identifies where the speed came from at each rung.
- A headline number: the maximum single-core (T3) and multicore (T4) speedup over
  the current baseline for the Blackman-Harris complex-FFT window.

## Non-goals

- Merging any of this into the library or modifying `src/`.
- Portability beyond the AVX-256 Sandy Bridge target.
- Optimizing any function other than the Blackman-Harris complex-FFT window.
