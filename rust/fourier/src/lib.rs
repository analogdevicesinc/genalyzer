// Copyright (C) 2024-2025 Analog Devices, Inc.
//
// SPDX short identifier: ADIBSD OR GPL-2.0-or-later

//! Rust FFT module for genalyzer — optimised for speed and reentrant operation.
//!
//! This module exposes:
//! - A stateful [`FourierAnalyzer`] that pre-plans FFT algorithms once and
//!   reuses them across calls.  It is `Send + Sync` so a single instance can
//!   be shared across threads without any locks.
//! - Stateless free functions ([`forward_fft`], [`inverse_fft`],
//!   [`power_spectrum`], [`frequencies`]) for one-shot use.
//!
//! # Normalization convention
//! Forward FFT is unnormalized.  Inverse FFT divides by `N`, matching the
//! convention used by `numpy.fft` and the existing Python implementation.
//!
//! # Building the Python extension
//! Enable the `python` feature and use `maturin`:
//! ```text
//! maturin develop --features python
//! ```
//! The resulting module is importable as `genalyzer._fourier_rs`.

use num_complex::Complex;
use rustfft::{Fft, FftPlanner};
use std::sync::Arc;

// ── FourierAnalyzer ──────────────────────────────────────────────────────────

/// A pre-planned FFT analyzer for efficient, concurrent repeated use.
///
/// Construct once with the FFT size `n`, then call any of its methods freely
/// from multiple threads simultaneously — the plans are immutable after
/// construction and scratch buffers are allocated per call.
///
/// ```no_run
/// use std::sync::Arc;
/// use std::thread;
/// // (crate is linked as _fourier_rs at the binary level)
/// # use _fourier_rs::FourierAnalyzer;
///
/// let analyzer = Arc::new(FourierAnalyzer::new(64));
/// let a2 = Arc::clone(&analyzer);
/// thread::spawn(move || {
///     let ps = a2.power_spectrum(&vec![1.0_f64; 64]);
///     assert_eq!(ps.len(), 64);
/// }).join().unwrap();
/// ```
pub struct FourierAnalyzer {
    n: usize,
    forward_plan: Arc<dyn Fft<f64>>,
    inverse_plan: Arc<dyn Fft<f64>>,
    fwd_scratch_len: usize,
    inv_scratch_len: usize,
}

// SAFETY: `Arc<dyn Fft<f64>>` is Send + Sync when the underlying algorithm
// type is Send + Sync.  All rustfft algorithm structs implement both traits.
unsafe impl Send for FourierAnalyzer {}
unsafe impl Sync for FourierAnalyzer {}

impl FourierAnalyzer {
    /// Create a new analyzer pre-planned for inputs of length `n`.
    ///
    /// Planning runs once here; subsequent calls reuse the cached plans.
    pub fn new(n: usize) -> Self {
        let mut planner = FftPlanner::<f64>::new();
        let forward_plan = planner.plan_fft_forward(n);
        let inverse_plan = planner.plan_fft_inverse(n);
        let fwd_scratch_len = forward_plan.get_inplace_scratch_len();
        let inv_scratch_len = inverse_plan.get_inplace_scratch_len();
        Self {
            n,
            forward_plan,
            inverse_plan,
            fwd_scratch_len,
            inv_scratch_len,
        }
    }

    /// Forward FFT of a real-valued signal.
    ///
    /// The input is zero-padded (or silently truncated) to `self.n()`.
    /// Returns an empty `Vec` when `input` is empty.
    pub fn forward_fft(&self, input: &[f64]) -> Vec<Complex<f64>> {
        if input.is_empty() {
            return Vec::new();
        }
        let mut buf: Vec<Complex<f64>> = input
            .iter()
            .take(self.n)
            .map(|&x| Complex::new(x, 0.0))
            .collect();
        buf.resize(self.n, Complex::new(0.0, 0.0));
        let mut scratch = vec![Complex::new(0.0, 0.0); self.fwd_scratch_len];
        self.forward_plan.process_with_scratch(&mut buf, &mut scratch);
        buf
    }

    /// Inverse FFT of a complex spectrum, returning the real part.
    ///
    /// Normalized by `1/N`, matching the Python/numpy convention.
    /// Input is zero-padded or truncated to `self.n()`.
    /// Returns an empty `Vec` when `input` is empty.
    pub fn inverse_fft(&self, input: &[Complex<f64>]) -> Vec<f64> {
        if input.is_empty() {
            return Vec::new();
        }
        let n = self.n;
        let mut buf: Vec<Complex<f64>> = input.iter().take(n).cloned().collect();
        buf.resize(n, Complex::new(0.0, 0.0));
        let mut scratch = vec![Complex::new(0.0, 0.0); self.inv_scratch_len];
        self.inverse_plan.process_with_scratch(&mut buf, &mut scratch);
        let scale = 1.0 / n as f64;
        buf.iter().map(|c| c.re * scale).collect()
    }

    /// Power spectrum: magnitude squared of each FFT bin.
    pub fn power_spectrum(&self, input: &[f64]) -> Vec<f64> {
        self.forward_fft(input)
            .iter()
            .map(|c| c.norm_sqr())
            .collect()
    }

    /// FFT size this analyzer was planned for.
    pub fn n(&self) -> usize {
        self.n
    }
}

// ── Stateless free functions ──────────────────────────────────────────────────

/// Single-shot forward FFT of a real-valued signal.
///
/// Creates a fresh planner and plan on every call.  Prefer [`FourierAnalyzer`]
/// for repeated calls to the same size.
pub fn forward_fft(input: &[f64]) -> Vec<Complex<f64>> {
    if input.is_empty() {
        return Vec::new();
    }
    let n = input.len();
    let mut planner = FftPlanner::<f64>::new();
    let plan = planner.plan_fft_forward(n);
    let mut buf: Vec<Complex<f64>> = input.iter().map(|&x| Complex::new(x, 0.0)).collect();
    let scratch_len = plan.get_inplace_scratch_len();
    let mut scratch = vec![Complex::new(0.0, 0.0); scratch_len];
    plan.process_with_scratch(&mut buf, &mut scratch);
    buf
}

/// Single-shot inverse FFT.  Normalized by `1/N`.
///
/// Creates a fresh planner and plan on every call.  Prefer [`FourierAnalyzer`]
/// for repeated calls to the same size.
pub fn inverse_fft(input: &[Complex<f64>]) -> Vec<f64> {
    if input.is_empty() {
        return Vec::new();
    }
    let n = input.len();
    let mut planner = FftPlanner::<f64>::new();
    let plan = planner.plan_fft_inverse(n);
    let mut buf = input.to_vec();
    let scratch_len = plan.get_inplace_scratch_len();
    let mut scratch = vec![Complex::new(0.0, 0.0); scratch_len];
    plan.process_with_scratch(&mut buf, &mut scratch);
    let scale = 1.0 / n as f64;
    buf.iter().map(|c| c.re * scale).collect()
}

/// Single-shot power spectrum (magnitude squared per bin).
pub fn power_spectrum(input: &[f64]) -> Vec<f64> {
    forward_fft(input).iter().map(|c| c.norm_sqr()).collect()
}

/// Frequency bin centres for an FFT of size `n` at `sample_rate` Hz.
///
/// Uses the DC-left convention: bin `k` corresponds to `k * sample_rate / n`.
/// Returns `n` values in `[0, sample_rate)`.
pub fn frequencies(n: usize, sample_rate: f64) -> Vec<f64> {
    if n == 0 {
        return Vec::new();
    }
    let bin_width = sample_rate / n as f64;
    (0..n).map(|i| i as f64 * bin_width).collect()
}

// ── PyO3 Python bindings ──────────────────────────────────────────────────────

#[cfg(feature = "python")]
mod python {
    // Import only what we need — a glob import would conflict with the local
    // #[pyfunction] definitions that share names with the outer free functions.
    use super::FourierAnalyzer;
    use num_complex::Complex;
    use pyo3::prelude::*;

    /// Python-visible wrapper around [`FourierAnalyzer`].
    ///
    /// Complex numbers are exchanged as `(real: float, imag: float)` tuples so
    /// no numpy dependency is required on the Rust side.
    #[pyclass(name = "FourierAnalyzer")]
    pub struct PyFourierAnalyzer {
        inner: FourierAnalyzer,
    }

    #[pymethods]
    impl PyFourierAnalyzer {
        /// Create a new analyzer pre-planned for `n`-point FFTs.
        #[new]
        fn new(n: usize) -> Self {
            Self {
                inner: FourierAnalyzer::new(n),
            }
        }

        /// Forward FFT of a real-valued list.
        ///
        /// Returns a list of `(real, imag)` tuples.
        fn forward_fft(&self, input: Vec<f64>) -> Vec<(f64, f64)> {
            self.inner
                .forward_fft(&input)
                .into_iter()
                .map(|c| (c.re, c.im))
                .collect()
        }

        /// Inverse FFT.  `input` is a list of `(real, imag)` tuples.
        ///
        /// Normalized by `1/N`.  Returns a list of real floats.
        fn inverse_fft(&self, input: Vec<(f64, f64)>) -> Vec<f64> {
            let complex_in: Vec<Complex<f64>> = input
                .into_iter()
                .map(|(re, im)| Complex::new(re, im))
                .collect();
            self.inner.inverse_fft(&complex_in)
        }

        /// Power spectrum (magnitude squared) of a real-valued list.
        fn power_spectrum(&self, input: Vec<f64>) -> Vec<f64> {
            self.inner.power_spectrum(&input)
        }

        /// FFT size this analyzer was planned for.
        fn n(&self) -> usize {
            self.inner.n()
        }
    }

    /// Single-shot forward FFT.  Returns a list of `(real, imag)` tuples.
    #[pyfunction]
    fn forward_fft(input: Vec<f64>) -> Vec<(f64, f64)> {
        super::forward_fft(&input)
            .into_iter()
            .map(|c| (c.re, c.im))
            .collect()
    }

    /// Single-shot inverse FFT.  `input` is a list of `(real, imag)` tuples.
    #[pyfunction]
    fn inverse_fft(input: Vec<(f64, f64)>) -> Vec<f64> {
        let complex_in: Vec<Complex<f64>> = input
            .into_iter()
            .map(|(re, im)| Complex::new(re, im))
            .collect();
        super::inverse_fft(&complex_in)
    }

    /// Single-shot power spectrum.
    #[pyfunction]
    fn power_spectrum(input: Vec<f64>) -> Vec<f64> {
        super::power_spectrum(&input)
    }

    /// Frequency bin centres for an FFT of size `n` at `sample_rate` Hz.
    #[pyfunction]
    fn frequencies(n: usize, sample_rate: f64) -> Vec<f64> {
        super::frequencies(n, sample_rate)
    }

    /// Python module `genalyzer._fourier_rs`.
    ///
    /// Build with:  `maturin develop --features python`
    #[pymodule]
    #[pyo3(name = "_fourier_rs")]
    pub fn fourier_rs_module(m: &Bound<'_, PyModule>) -> PyResult<()> {
        m.add_class::<PyFourierAnalyzer>()?;
        m.add_function(wrap_pyfunction!(forward_fft, m)?)?;
        m.add_function(wrap_pyfunction!(inverse_fft, m)?)?;
        m.add_function(wrap_pyfunction!(power_spectrum, m)?)?;
        m.add_function(wrap_pyfunction!(frequencies, m)?)?;
        Ok(())
    }
}

// ── Tests ─────────────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;
    use num_complex::Complex;
    use std::f64::consts::PI;

    const N: usize = 64;

    fn sine(freq_bin: usize, n: usize) -> Vec<f64> {
        (0..n)
            .map(|i| (2.0 * PI * freq_bin as f64 * i as f64 / n as f64).sin())
            .collect()
    }

    // ── Round-trip FFT / IFFT ────────────────────────────────────────────────

    #[test]
    fn test_round_trip_stateful() {
        let signal = sine(5, N);
        let analyzer = FourierAnalyzer::new(N);
        let spectrum = analyzer.forward_fft(&signal);
        let reconstructed = analyzer.inverse_fft(&spectrum);
        for (original, recovered) in signal.iter().zip(reconstructed.iter()) {
            assert!(
                (original - recovered).abs() < 1e-10,
                "round-trip mismatch: {original} != {recovered}"
            );
        }
    }

    #[test]
    fn test_round_trip_stateless() {
        let signal = sine(3, N);
        let spectrum = forward_fft(&signal);
        let reconstructed = inverse_fft(&spectrum);
        for (original, recovered) in signal.iter().zip(reconstructed.iter()) {
            assert!(
                (original - recovered).abs() < 1e-10,
                "round-trip mismatch: {original} != {recovered}"
            );
        }
    }

    // ── Power spectrum on a known single-frequency sine ──────────────────────

    #[test]
    fn test_power_spectrum_single_tone() {
        let k = 5_usize;
        let signal = sine(k, N);
        let analyzer = FourierAnalyzer::new(N);
        let ps = analyzer.power_spectrum(&signal);

        assert_eq!(ps.len(), N);

        let max_bin = ps
            .iter()
            .enumerate()
            .max_by(|a, b| a.1.partial_cmp(b.1).unwrap())
            .unwrap()
            .0;

        // For a real sine the energy sits at bins k and N-k.
        assert!(
            max_bin == k || max_bin == N - k,
            "max power at bin {max_bin}, expected {k} or {}",
            N - k
        );
    }

    #[test]
    fn test_power_spectrum_dc() {
        // A DC signal should have all power in bin 0.
        let signal = vec![1.0_f64; N];
        let ps = power_spectrum(&signal);
        let max_bin = ps
            .iter()
            .enumerate()
            .max_by(|a, b| a.1.partial_cmp(b.1).unwrap())
            .unwrap()
            .0;
        assert_eq!(max_bin, 0, "DC signal: max power should be at bin 0");
    }

    // ── Zero-length input guards ─────────────────────────────────────────────

    #[test]
    fn test_forward_fft_zero_length() {
        let result = forward_fft(&[]);
        assert!(result.is_empty());
    }

    #[test]
    fn test_inverse_fft_zero_length() {
        let result = inverse_fft(&[]);
        assert!(result.is_empty());
    }

    #[test]
    fn test_power_spectrum_zero_length() {
        let result = power_spectrum(&[]);
        assert!(result.is_empty());
    }

    #[test]
    fn test_frequencies_zero_length() {
        let result = frequencies(0, 1000.0);
        assert!(result.is_empty());
    }

    // ── frequencies() correctness ────────────────────────────────────────────

    #[test]
    fn test_frequencies_bins() {
        let fs = 1000.0_f64;
        let freqs = frequencies(8, fs);
        assert_eq!(freqs.len(), 8);
        assert!((freqs[0]).abs() < 1e-12, "bin 0 should be 0 Hz");
        assert!((freqs[1] - 125.0).abs() < 1e-10, "bin 1 = fs/8");
        assert!((freqs[4] - 500.0).abs() < 1e-10, "bin 4 = fs/2");
        assert!((freqs[7] - 875.0).abs() < 1e-10, "bin 7 = 7*fs/8");
    }

    // ── Non-power-of-two sizes ───────────────────────────────────────────────

    #[test]
    fn test_round_trip_non_power_of_two() {
        let n = 100_usize; // not a power of two
        let signal: Vec<f64> = (0..n).map(|i| i as f64).collect();
        let analyzer = FourierAnalyzer::new(n);
        let spectrum = analyzer.forward_fft(&signal);
        let reconstructed = analyzer.inverse_fft(&spectrum);
        for (orig, rec) in signal.iter().zip(reconstructed.iter()) {
            assert!(
                (orig - rec).abs() < 1e-8,
                "non-pow2 round-trip failed: {orig} != {rec}"
            );
        }
    }

    // ── Thread-safety smoke test ─────────────────────────────────────────────

    #[test]
    fn test_thread_safety() {
        use std::sync::Arc;
        use std::thread;

        let analyzer = Arc::new(FourierAnalyzer::new(N));
        let handles: Vec<_> = (0..8_usize)
            .map(|t| {
                let a = Arc::clone(&analyzer);
                thread::spawn(move || {
                    let signal = sine(t + 1, N);
                    let spectrum = a.forward_fft(&signal);
                    let reconstructed = a.inverse_fft(&spectrum);
                    for (orig, rec) in signal.iter().zip(reconstructed.iter()) {
                        assert!(
                            (orig - rec).abs() < 1e-9,
                            "thread {t}: round-trip failed: {orig} != {rec}"
                        );
                    }
                })
            })
            .collect();

        for h in handles {
            h.join().expect("worker thread panicked");
        }
    }

    // ── Normalization convention matches numpy ───────────────────────────────

    #[test]
    fn test_ifft_normalization() {
        // IFFT of a spectrum that has 1.0 at bin 0 should produce a constant
        // signal with value 1/N.  After multiplying by N (un-normalizing) we
        // get 1.0.  Our convention divides by N on the inverse, so the result
        // of IFFT([N, 0, ..., 0]) should be all-ones.
        let n = 8_usize;
        let mut spectrum = vec![Complex::new(0.0, 0.0); n];
        spectrum[0] = Complex::new(n as f64, 0.0);
        let result = inverse_fft(&spectrum);
        for v in &result {
            assert!(
                (v - 1.0).abs() < 1e-12,
                "normalization failed: expected 1.0, got {v}"
            );
        }
    }
}
