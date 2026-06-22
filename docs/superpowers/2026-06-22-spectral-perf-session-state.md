# Session state — genalyzer spectral-analysis performance exploration

**Purpose:** hand-off so a fresh session can resume cold after a reboot.
**Last updated:** 2026-06-22. **Branch:** `bench/bh-window-asm` (off `main`).

---

## Goal

Explore how fast genalyzer's spectral-analysis math can go, down to hand-tuned
assembly — a research spike (throwaway, non-portable OK), not a production
refactor. Nothing in `src/` is modified; all work lives under `bench/`.

## Hardware / environment quirks (IMPORTANT for any rebuild)

- CPU: Intel Xeon E5-1650 (Sandy Bridge-E). **AVX-256, NO FMA, NO AVX2, NO
  AVX-512.** 6 physical cores / 12 threads. `real_t = double`.
- **Shadowed assembler:** `~/.local/bin/as` is an "agent session launcher" that
  hijacks the real `/usr/bin/as`, breaking g++/CMake assembly. Workaround used
  everywhere: prepend system paths first —
  `export PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:$PATH`
  (`bench/build.sh` already does this).
- **perf is blocked:** `perf_event_paranoid=4`, no CAP_PERFMON, no sudo. perf
  sampling unavailable — profiling was done via temporary in-source stage timers
  (since reverted). If a reboot lowers paranoid, perf becomes an option.
- FFTW3 + headers installed; CMake + g++ 13.3 present.

## Work completed

### 1. Window optimization-ladder spike (DONE, all reviewed)
- Spec: `docs/superpowers/specs/2026-06-19-blackman-harris-window-asm-design.md`
- Plan: `docs/superpowers/plans/2026-06-19-blackman-harris-window-asm-benchmark.md`
- Code: `bench/bh_window_bench.cpp`, `bench/bh_window_asm.S`, `bench/build.sh`,
  `bench/README.md`, `bench/results.txt`. Build+run: `bench/build.sh &&
  bench/bh_window_bench`.
- Tiers T0 baseline (std::cos x3/sample) -> T1 no-transcendentals (phasor
  recurrence) -> T2 AVX-256 intrinsics -> T3 hand-tuned asm -> T4 multithreaded.
- **Finding:** ~8-10x single-core gain came ENTIRELY from T0->T1 (killing the
  transcendentals + precomputing the data-independent window). T2/T3 (SIMD/asm)
  added ~nothing — once transcendentals are gone the apply loop is
  memory-bandwidth-bound. T4 only wins at large nfft+navg (~10x), slower than T3
  at small nfft (thread-spawn overhead). Lesson: **the window's transcendentals
  are data-independent, so the win is algorithmic, not asm.**

### 2. Real-pipeline profile (DONE)
- `bench/profile/PROFILE.md` (full method + numbers), `profile_pipeline.cpp`.
- Method: temporary `std::chrono` stage timers inside the real
  `genalyzer_impl::fft()` (window / FFTW / reduce_and_scale), env-gated print at
  exit, **reverted after measuring (not committed to src/)**. Driver calls the
  real `fft()` on a complex tone, one (nfft,navg) per process.
- **Finding (confirmed hypothesis):** `reduce_and_scale` is the SINGLE LARGEST
  stage whenever navg>1 — **47-68% of the whole transform, 1.5-4.6x the FFT
  itself.** It is `std::arg` (atan2) called `navg*nfft` times + `std::polar`
  (sincos) `nfft` times. navg==1 uses cheap `scale_fft` (no atan2) and the
  window dominates there instead.

  | nfft | navg | window | FFTW | reduce_and_scale |
  |------|------|--------|------|------------------|
  | 4096 | 8 | 16.6% | 14.9% | 68.5% |
  | 65536 | 8 | 23.2% | 19.0% | 57.8% |
  | 1048576 | 8 | 21.6% | 31.4% | 47.0% |

## Key insight / the next lever

`reduce_and_scale_fft`/`_rfft` live in `src/fourier_transforms.cpp` (~lines
635-689). They store `(std::norm, std::arg)` per bin, accumulate over navg, then
`std::polar(sqrt(...), phase)`.

The analysis path consumes **mean-square magnitude only**
(`analyze_impl(const real_t *msq_data ...)` in `src/fourier_analysis.cpp:~400`;
comment at ~line 219: "phase not available"). Phase is pulled ON-DEMAND at a
handful of tone bins via `fa_phase` -> `std::arg(fft_data[index])`
(`fourier_analysis.cpp:~1149`). So the `navg*nfft` per-bin atan2 calls that
retain phase for EVERY bin look largely wasted.

**Unlike the window, these transcendentals are data-dependent (can't precompute)
-> genuine compute-bound SIMD/asm target. But the bigger win is likely
algorithmic: drop/defer per-bin phase.**

## Planned next steps (decided, not yet started)

1. **Investigate phase-deferral first** (analysis, low-risk, likely biggest win):
   trace whether per-bin `std::arg` can be safely deferred to only the tone bins
   that `fa_phase` reads. Watch out: the stored phase is the AVERAGE arg across
   navg rows — deferring requires the raw per-row FFT data still be available at
   the point phase is needed (it currently is, in the navg>1 `tmp` buffer inside
   `fft()`, but that buffer is local and freed). Determine feasibility +
   correctness impact on existing metrics/tests.
2. **Then build a reduce_and_scale optimization-ladder spike** (mirror the window
   one): T0 baseline -> T1 drop/defer phase (algorithmic) -> T2 vectorized
   atan2/sincos (AVX-256 polynomial, SLEEF-style) -> T3 hand-tuned asm -> T4
   multicore. Verify each vs T0 within tolerance; report Msamples/s + attribution.

## Branch / git status

- Branch `bench/bh-window-asm`, ~12 commits (window spike + profiling commit
  `97741bd`). Working tree clean; `src/` unmodified; `build/` gitignored;
  `bench/profile/profile_pipeline` binary gitignored.
- **PR:** being pushed/opened for the window spike + profiling (per decision
  2026-06-22). Check `gh pr list` / the branch on the remote.

## How to rebuild from cold

```sh
export PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:$PATH
# window spike:
bench/build.sh && bench/bh_window_bench
# real library (for profiling driver):
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON
cmake --build build -j$(nproc)
# profiling driver (needs the stage-timer instrumentation re-added to
# src/fourier_transforms.cpp fft() if you want the per-stage breakdown again —
# see bench/profile/PROFILE.md for exactly what to wrap):
g++ -O3 -std=c++17 -Iinclude bench/profile/profile_pipeline.cpp \
    -o bench/profile/profile_pipeline \
    -Lbuild/bindings/c/src -lgenalyzer -Wl,-rpath,$PWD/build/bindings/c/src
LD_LIBRARY_PATH=build/bindings/c/src GEN_PROFILE=1 \
    bench/profile/profile_pipeline 65536 8 1.0
```
