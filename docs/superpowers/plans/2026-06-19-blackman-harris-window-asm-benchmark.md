# Blackman-Harris Window Optimization-Ladder Benchmark Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build a standalone benchmark that reimplements the genalyzer Blackman-Harris complex-FFT window function at five escalating optimization tiers (baseline → no-transcendentals → AVX intrinsics → hand-tuned assembly → multithreaded assembly) and reports the per-tier speedup ceiling on this AVX-256 machine.

**Architecture:** A self-contained `bench/` directory built by a `build.sh` one-liner — no CMake, no changes to `src/`. One C++ translation unit holds the harness plus the C++ tiers (T0–T2, T4 driver); one `.S` file holds the AVX-256 assembly apply kernel (T3/T4). The harness generates fixed-seed random I/Q input, runs every tier on identical data, checks each tier's output against the T0 baseline (max-abs-err < 1e-9), times each (best-of-N, warm-up discarded), and prints a results table plus an attribution summary.

**Tech Stack:** C++17, g++ 13.3, AVX-256 intrinsics (`<immintrin.h>`, `-mavx`, **no FMA/AVX2**), GAS AT&T-syntax assembly, `std::thread`, `std::chrono`.

---

## Background facts the implementer needs

- Target function (faithful reference): `src/fourier_transforms.cpp:35` — the **complex-FFT** Blackman-Harris path. Per sample index `i` (`x = (double)i`):
  ```
  w        = bh_k0 + bh_k1*cos(k1*x) + bh_k2*cos(2*k1*x) + bh_k3*cos(3*k1*x)
  out[2i]   = w * scalar * i_data[i*in_stride]
  out[2i+1] = w * scalar * q_data[i*in_stride]
  ```
  where `scalar = bh_kx`, `k1 = 2*pi/nfft`. For `navg > 1` the *same* window applies to every row; row `k`'s input sample is `i_data[k*nfft*in_stride + i*in_stride]` and output is `out[k*nfft*2 + 2i]`.
- Constants (copied from `src/fourier_transforms.cpp`):
  `bh_kx = 1.9688861870585801`, `bh_k0 = 0.35875`, `bh_k1 = -0.48829`, `bh_k2 = 0.14128`, `bh_k3 = -0.01168`.
- `real_t` in the library is `double`. The benchmark uses `double` directly and includes **no** library headers — it is fully standalone.
- The workload always uses `in_stride = 1`, and input is laid out as `navg` contiguous rows of `nfft` samples (so row `k`, sample `i` = `data[k*nfft + i]`).
- **Key optimization the ladder exploits:** the window vector is data-independent and identical across all `navg` rows, yet T0 recomputes 3 `std::cos` per sample. T1+ precompute a **scaled** window vector `sw[i] = bh_kx * w(i)` once, then the apply is just `out[2i] = sw[i]*I[i]`, `out[2i+1] = sw[i]*Q[i]`.
- All benchmarked `nfft` values (4096, 65536, 1048576) are multiples of 4, so the SIMD/asm kernels may assume `nfft % 4 == 0`.

## File structure

```
bench/
  build.sh              # standalone g++ build (T0–T2 in .cpp, T3/T4 kernel in .S)
  bh_window_bench.cpp   # harness: constants, reference, tiers T0/T1/T2/T4-driver,
                        #          correctness check, timing, config sweep, reporting
  bh_window_asm.S       # AVX-256 assembly apply kernel: bh_apply_asm (used by T3 and T4)
  README.md             # how to build/run + captured results table + attribution
```

All tiers share one apply contract once the window exists. The signature every tier exposes to the harness is:

```cpp
using window_fn = void(*)(const double* I, const double* Q, double* out,
                          size_t in_stride, size_t navg, size_t nfft);
```

---

## Task 1: Scaffold harness + T0 baseline + build

**Files:**
- Create: `bench/bh_window_bench.cpp`
- Create: `bench/build.sh`
- Create: `bench/bh_window_asm.S` (empty-but-valid stub so the build links from the start)

- [ ] **Step 1: Create the assembly stub so the build links**

`bench/bh_window_asm.S`:
```asm
# AVX-256 apply kernel lives here (filled in Task 4). Empty stub for now.
.section .note.GNU-stack,"",@progbits
```

- [ ] **Step 2: Write the harness with T0 baseline, an independent reference, correctness check, and a tiny self-test in main**

`bench/bh_window_bench.cpp`:
```cpp
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
```

- [ ] **Step 3: Write the build script**

`bench/build.sh`:
```sh
#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
g++ -O3 -mavx -std=c++17 -pthread -c bh_window_bench.cpp -o bh_window_bench.o
g++ -c bh_window_asm.S -o bh_window_asm.o
g++ bh_window_bench.o bh_window_asm.o -o bh_window_bench -pthread
echo "built ./bench/bh_window_bench"
```

- [ ] **Step 4: Build and run the anchor self-test**

Run: `chmod +x bench/build.sh && bench/build.sh && bench/bh_window_bench`
Expected: prints `T0 anchor max-abs-err = ...e-16 -> PASS` and exits 0.

- [ ] **Step 5: Commit**

```bash
git add bench/bh_window_bench.cpp bench/build.sh bench/bh_window_asm.S
git commit -m "bench: scaffold BH window harness with T0 baseline + anchor"
```

---

## Task 2: T1 — no-transcendentals (precomputed window via phasor recurrence)

**Files:**
- Modify: `bench/bh_window_bench.cpp` (add `gen_scaled_window`, `bh_window_t1`; register in a tiers table and a correctness loop in `main`)

- [ ] **Step 1: Add a stubbed T1 and a correctness loop that FAILS**

Add above `main` in `bench/bh_window_bench.cpp`:
```cpp
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
```

Replace the body of `main` (keep the anchor test, then add a tier-correctness loop) with:
```cpp
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
```
Add `#include <algorithm>` near the top includes for `std::fill`.

- [ ] **Step 2: Build and run — confirm T1 PASSES correctness**

Run: `bench/build.sh && bench/bh_window_bench`
Expected: `anchor ... PASS`, `correctness: T0 baseline ... PASS`, `correctness: T1 no-trig ... PASS` (T1 err ~1e-13 or smaller).

- [ ] **Step 3: Commit**

```bash
git add bench/bh_window_bench.cpp
git commit -m "bench: add T1 no-transcendentals tier (phasor recurrence)"
```

---

## Task 3: T2 — AVX-256 intrinsics apply

**Files:**
- Modify: `bench/bh_window_bench.cpp` (add `bh_window_t2` using `<immintrin.h>`; register in `tiers`)

- [ ] **Step 1: Add T2 (vectorized interleaved apply) and register it**

Add `#include <immintrin.h>` to the includes. Add above `main`:
```cpp
// T2: shared precomputed window, AVX-256 apply. Processes 4 samples/iter.
// For samples i..i+3: ri = sw*I, rq = sw*Q, then interleave to
// out = r0,q0,r1,q1,r2,q2,r3,q3. nfft assumed multiple of 4.
void bh_window_t2(const double* i_data, const double* q_data, double* out_data,
                  size_t in_stride, size_t navg, size_t nfft) {
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
```
Add `{"T2 avx-intr", bh_window_t2},` to the `tiers[]` array in `main`.

- [ ] **Step 2: Build and run — confirm T2 PASSES correctness**

Run: `bench/build.sh && bench/bh_window_bench`
Expected: `correctness: T2 avx-intr max-abs-err = ...e-13 -> PASS`.

- [ ] **Step 3: Commit**

```bash
git add bench/bh_window_bench.cpp
git commit -m "bench: add T2 AVX-256 intrinsics tier"
```

---

## Task 4: T3 — hand-tuned AVX-256 assembly apply (single core)

**Files:**
- Modify: `bench/bh_window_asm.S` (implement `bh_apply_asm`)
- Modify: `bench/bh_window_bench.cpp` (declare `extern "C"` kernel, add `bh_window_t3`, register it)

- [ ] **Step 1: Implement the assembly apply kernel**

Replace `bench/bh_window_asm.S` with:
```asm
    .text
    .globl bh_apply_asm
    .type  bh_apply_asm, @function
# void bh_apply_asm(const double* sw, const double* I, const double* Q,
#                   double* out, size_t nfft)   // nfft multiple of 4
# rdi=sw  rsi=I  rdx=Q  rcx=out  r8=nfft
bh_apply_asm:
    xor     %rax, %rax                 # rax = sample index i
.Lloop:
    cmp     %r8, %rax
    jae     .Ldone
    vmovupd (%rdi,%rax,8), %ymm0        # sw[i..i+3]
    vmovupd (%rsi,%rax,8), %ymm1        # I[i..i+3]
    vmovupd (%rdx,%rax,8), %ymm2        # Q[i..i+3]
    vmulpd  %ymm1, %ymm0, %ymm1         # ri = sw*I  (r0 r1 r2 r3)
    vmulpd  %ymm2, %ymm0, %ymm2         # rq = sw*Q  (q0 q1 q2 q3)
    vunpcklpd %ymm2, %ymm1, %ymm3       # r0 q0 r2 q2
    vunpckhpd %ymm2, %ymm1, %ymm4       # r1 q1 r3 q3
    vperm2f128 $0x20, %ymm4, %ymm3, %ymm5   # r0 q0 r1 q1
    vperm2f128 $0x31, %ymm4, %ymm3, %ymm6   # r2 q2 r3 q3
    lea     (%rax,%rax,1), %r9          # r9 = 2*i  (out is interleaved)
    vmovupd %ymm5, (%rcx,%r9,8)         # out[2i .. 2i+3]
    vmovupd %ymm6, 32(%rcx,%r9,8)       # out[2i+4 .. 2i+7]
    add     $4, %rax
    jmp     .Lloop
.Ldone:
    vzeroupper
    ret
    .size  bh_apply_asm, .-bh_apply_asm
.section .note.GNU-stack,"",@progbits
```

- [ ] **Step 2: Declare the kernel and add T3 driver in the harness**

Add near the top of `bench/bh_window_bench.cpp` (after includes):
```cpp
extern "C" void bh_apply_asm(const double* sw, const double* I,
                             const double* Q, double* out, size_t nfft);
```
Add above `main`:
```cpp
// T3: shared precomputed window, assembly apply kernel, single core.
void bh_window_t3(const double* i_data, const double* q_data, double* out_data,
                  size_t in_stride, size_t navg, size_t nfft) {
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
```
Add `{"T3 asm-1core", bh_window_t3},` to `tiers[]`.

- [ ] **Step 3: Build and run — confirm T3 PASSES correctness**

Run: `bench/build.sh && bench/bh_window_bench`
Expected: `correctness: T3 asm-1core max-abs-err = ...e-13 -> PASS`.

- [ ] **Step 4: Commit**

```bash
git add bench/bh_window_asm.S bench/bh_window_bench.cpp
git commit -m "bench: add T3 hand-tuned AVX-256 assembly tier (single core)"
```

---

## Task 5: T4 — multithreaded assembly apply

**Files:**
- Modify: `bench/bh_window_bench.cpp` (add `bh_window_t4` using `std::thread`; register it)

- [ ] **Step 1: Add T4 (window generated once, apply split across 6 threads)**

Add `#include <thread>` to includes. Add above `main`:
```cpp
// T4: precompute window once, then run the asm apply kernel on disjoint
// row-chunks across NTHREADS physical cores.
static const unsigned NTHREADS = 6;  // E5-1650 physical cores
void bh_window_t4(const double* i_data, const double* q_data, double* out_data,
                  size_t in_stride, size_t navg, size_t nfft) {
    static thread_local std::vector<double> sw;
    sw.resize(nfft);
    gen_scaled_window(sw.data(), nfft);
    const size_t in_row_stride = nfft * in_stride;
    const size_t out_row_stride = nfft * 2;

    auto apply_rows = [&](size_t k0, size_t k1) {
        for (size_t k = k0; k < k1; ++k)
            bh_apply_asm(sw.data(), i_data + k * in_row_stride,
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
                const double* swp = sw.data() + i0;
                ts.emplace_back([=] { bh_apply_asm(swp, pi, pq, po, n); });
                if (ts.size() == NTHREADS) { for (auto& th : ts) th.join(); ts.clear(); }
            }
        }
        for (auto& th : ts) th.join();
    }
}
```
Add `{"T4 asm-6core", bh_window_t4},` to `tiers[]`.

Note: the sample-range partition relies on `nfft % 4 == 0` and `chunk % 4 == 0` so every `bh_apply_asm` call gets a multiple-of-4 length — true for all workload sizes.

- [ ] **Step 2: Build and run — confirm T4 PASSES correctness**

Run: `bench/build.sh && bench/bh_window_bench`
Expected: `correctness: T4 asm-6core max-abs-err = ...e-13 -> PASS`.

- [ ] **Step 3: Commit**

```bash
git add bench/bh_window_bench.cpp
git commit -m "bench: add T4 multithreaded assembly tier"
```

---

## Task 6: Config sweep, timing, results table, README

**Files:**
- Modify: `bench/bh_window_bench.cpp` (add timing + sweep + table to `main`)
- Create: `bench/README.md`

- [ ] **Step 1: Add a timing helper and a sweep+report block to `main`**

Add `#include <cstdint>` and `<chrono>` (already present) usage. Add above `main`:
```cpp
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
```

After the correctness block in `main` (before `return 0;`), add:
```cpp
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
```

- [ ] **Step 2: Build, run, and capture the results**

Run: `bench/build.sh && bench/bh_window_bench | tee bench/results.txt`
Expected: anchor PASS, all correctness PASS, then a table with six config blocks; T0 speedup column reads `1.00x`, and T1–T4 show increasing Msamples/s.

- [ ] **Step 3: Write the README with build/run instructions and the captured table**

Create `bench/README.md`. Paste the actual table from `bench/results.txt` into the Results section and write a 1-paragraph attribution summary covering: how much came from killing transcendentals (T1 vs T0), from AVX intrinsics (T2 vs T1), from hand-tuned assembly (T3 vs T2), and from threading (T4 vs T3); plus the observation that at `navg=1` window *generation* dominates (so apply-side SIMD/asm shows less there) while at `navg=8` the apply-side tiers separate. Use this skeleton:
```markdown
# Blackman-Harris Window Optimization-Ladder Benchmark

Standalone spike measuring the speed ceiling of the genalyzer Blackman-Harris
complex-FFT window on this AVX-256 (no-FMA) Xeon E5-1650, 6 cores.

## Build & run
    ./build.sh
    ./bh_window_bench

## Tiers
- T0 baseline  — std::cos x3 per sample (copy of src/fourier_transforms.cpp:35)
- T1 no-trig   — precomputed window via phasor recurrence, scalar apply
- T2 avx-intr  — AVX-256 intrinsics apply
- T3 asm-1core — hand-tuned AVX-256 assembly apply
- T4 asm-6core — assembly apply split across 6 cores

## Results
<paste table from results.txt>

## Attribution
<1 paragraph: where the speedup came from at each rung; navg=1 vs navg=8 note>
```

- [ ] **Step 4: Commit**

```bash
git add bench/bh_window_bench.cpp bench/README.md bench/results.txt
git commit -m "bench: add timing sweep, results table, and README"
```

---

## Self-review notes (already applied)

- **Spec coverage:** T0–T4 tiers ✓ (Tasks 1–5); workload sweep nfft∈{4096,65536,1048576} × navg∈{1,8} ✓ (Task 6); correctness max-abs-err<1e-9 vs T0 ✓ (Tasks 2–6); Msamples/s + speedup + attribution ✓ (Task 6); standalone build, no src/ changes ✓ (Task 1); AVX-256 no-FMA target ✓ (`-mavx`, asm uses only AVX1 ops). Success-criteria headline numbers come from the Task 6 table.
- **Type consistency:** every tier matches `window_fn` (`const double*, const double*, double*, size_t, size_t, size_t`); `gen_scaled_window` and `bh_apply_asm` signatures are identical at definition and all call sites; `Tier`/`Cfg` structs defined before use in `main`.
- **No placeholders:** all code blocks complete; the only intentionally human-filled artifact is the captured results table + attribution paragraph in the README (Task 6 Step 3), which require the actual run output.
```
