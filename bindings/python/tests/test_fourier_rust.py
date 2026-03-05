# Copyright (C) 2024-2025 Analog Devices, Inc.
#
# SPDX short identifier: ADIBSD OR GPL-2.0-or-later
"""Integration tests for the Fourier analysis module.

Tests verify that:
1. The pure-Python fallback functions produce correct results.
2. When the Rust extension is available, its output matches the Python
   implementation within tolerance 1e-9.
3. The Rust path is faster than the pure-Python path for repeated calls
   (reported as an informational metric, not a hard assertion, because
   the ``list``-conversion overhead can dominate for small inputs).

Run with::

    pytest bindings/python/tests/test_fourier_rust.py -v
"""

from __future__ import annotations

import time

import numpy as np
import pytest

# Public API under test
from genalyzer.fourier import (
    FourierAnalyzer,
    _py_forward_fft,
    _py_frequencies,
    _py_inverse_fft,
    _py_power_spectrum,
    forward_fft,
    frequencies,
    inverse_fft,
    power_spectrum,
)

# Detect whether the Rust extension is available.
# Try production path first, then maturin-develop top-level install.
_rs = None
HAS_RUST = False
try:
    from genalyzer import _fourier_rs as _rs  # type: ignore[attr-defined]

    HAS_RUST = True
except (ImportError, OSError):
    try:
        import _fourier_rs as _rs  # type: ignore[no-redef]

        HAS_RUST = True
    except (ImportError, OSError):
        pass

TOLERANCE = 1e-9
_rng = np.random.default_rng(42)


# ── Helpers ───────────────────────────────────────────────────────────────────


def _sine(n: int, freq_bin: int) -> np.ndarray:
    return np.sin(2 * np.pi * freq_bin * np.arange(n) / n)


# ── Pure-Python tests (always run) ───────────────────────────────────────────


class TestPythonImplementation:
    """Verify correctness of the pure-Python fallback functions."""

    def test_forward_fft_length(self):
        sig = _sine(64, 5)
        assert _py_forward_fft(sig).shape == (64,)

    def test_round_trip(self):
        sig = _rng.standard_normal(128)
        spectrum = _py_forward_fft(sig)
        recovered = _py_inverse_fft(spectrum)
        np.testing.assert_allclose(recovered, sig, atol=1e-10)

    def test_power_spectrum_single_tone(self):
        n, k = 64, 7
        sig = _sine(n, k)
        ps = _py_power_spectrum(sig)
        max_bin = int(np.argmax(ps))
        assert max_bin == k or max_bin == n - k, f"expected {k} or {n - k}, got {max_bin}"

    def test_power_spectrum_dc(self):
        n = 32
        sig = np.ones(n)
        ps = _py_power_spectrum(sig)
        assert int(np.argmax(ps)) == 0

    def test_frequencies_dc_left(self):
        fs = 1000.0
        freqs = _py_frequencies(8, fs)
        np.testing.assert_allclose(freqs, [0, 125, 250, 375, 500, 625, 750, 875])

    def test_ifft_normalization(self):
        """IFFT([N, 0, …, 0]) == all-ones (verifies 1/N normalization)."""
        n = 8
        spectrum = np.zeros(n, dtype=complex)
        spectrum[0] = n
        result = _py_inverse_fft(spectrum)
        np.testing.assert_allclose(result, np.ones(n), atol=1e-12)


class TestFourierAnalyzerPython:
    """Verify the FourierAnalyzer class using the Python backend."""

    def test_repr(self):
        a = FourierAnalyzer(32)
        assert "FourierAnalyzer" in repr(a)

    def test_n_property(self):
        assert FourierAnalyzer(128).n == 128

    def test_round_trip(self):
        sig = _rng.standard_normal(64)
        a = FourierAnalyzer(64)
        recovered = a.inverse_fft(a.forward_fft(sig))
        np.testing.assert_allclose(recovered, sig, atol=1e-10)

    def test_power_spectrum(self):
        n, k = 64, 3
        sig = _sine(n, k)
        a = FourierAnalyzer(n)
        ps = a.power_spectrum(sig)
        max_bin = int(np.argmax(ps))
        assert max_bin == k or max_bin == n - k


# ── Rust-parity tests (skipped when Rust extension is unavailable) ────────────


@pytest.mark.skipif(not HAS_RUST, reason="genalyzer._fourier_rs not built")
class TestRustParity:
    """Assert that Rust outputs are numerically equivalent to numpy."""

    def test_forward_fft_parity(self):
        sig = _sine(64, 5)
        py_out = _py_forward_fft(sig)
        rs_pairs = _rs.forward_fft(sig.tolist())
        rs_out = np.array([re + 1j * im for re, im in rs_pairs])
        np.testing.assert_allclose(rs_out.real, py_out.real, atol=TOLERANCE)
        np.testing.assert_allclose(rs_out.imag, py_out.imag, atol=TOLERANCE)

    def test_inverse_fft_parity(self):
        sig = _rng.standard_normal(128)
        spectrum = _py_forward_fft(sig)
        py_out = _py_inverse_fft(spectrum)
        rs_pairs = [(c.real, c.imag) for c in spectrum]
        rs_out = np.array(_rs.inverse_fft(rs_pairs))
        np.testing.assert_allclose(rs_out, py_out, atol=TOLERANCE)

    def test_power_spectrum_parity(self):
        sig = _rng.standard_normal(256)
        py_out = _py_power_spectrum(sig)
        rs_out = np.array(_rs.power_spectrum(sig.tolist()))
        np.testing.assert_allclose(rs_out, py_out, atol=TOLERANCE)

    def test_frequencies_parity(self):
        fs = 44100.0
        n = 1024
        py_out = _py_frequencies(n, fs)
        rs_out = np.array(_rs.frequencies(n, fs))
        np.testing.assert_allclose(rs_out, py_out, atol=TOLERANCE)

    def test_round_trip_via_analyzer(self):
        sig = _rng.standard_normal(256)
        a = _rs.FourierAnalyzer(256)
        spectrum = a.forward_fft(sig.tolist())
        recovered = np.array(a.inverse_fft(spectrum))
        np.testing.assert_allclose(recovered, sig, atol=TOLERANCE)

    def test_power_spectrum_single_tone(self):
        n, k = 128, 11
        sig = _sine(n, k)
        a = _rs.FourierAnalyzer(n)
        ps = np.array(a.power_spectrum(sig.tolist()))
        max_bin = int(np.argmax(ps))
        assert max_bin == k or max_bin == n - k, (
            f"expected bin {k} or {n - k}, got {max_bin}"
        )

    def test_zero_length_guard(self):
        assert _rs.forward_fft([]) == []
        assert _rs.inverse_fft([]) == []
        assert _rs.power_spectrum([]) == []
        assert _rs.frequencies(0, 1000.0) == []

    # ------------------------------------------------------------------
    # Public module-level functions (routing through fourier.py) match.
    # ------------------------------------------------------------------

    def test_module_forward_fft_parity(self):
        sig = _rng.standard_normal(64)
        py_ref = _py_forward_fft(sig)
        result = forward_fft(sig)  # uses Rust path when available
        np.testing.assert_allclose(result.real, py_ref.real, atol=TOLERANCE)
        np.testing.assert_allclose(result.imag, py_ref.imag, atol=TOLERANCE)

    def test_module_inverse_fft_parity(self):
        sig = _rng.standard_normal(64)
        spectrum = np.fft.fft(sig)
        py_ref = _py_inverse_fft(spectrum)
        result = inverse_fft(spectrum)
        np.testing.assert_allclose(result, py_ref, atol=TOLERANCE)

    def test_module_power_spectrum_parity(self):
        sig = _rng.standard_normal(64)
        py_ref = _py_power_spectrum(sig)
        result = power_spectrum(sig)
        np.testing.assert_allclose(result, py_ref, atol=TOLERANCE)

    def test_module_frequencies_parity(self):
        py_ref = _py_frequencies(64, 1000.0)
        result = frequencies(64, 1000.0)
        np.testing.assert_allclose(result, py_ref, atol=TOLERANCE)


# ── Benchmark: repeated calls ─────────────────────────────────────────────────


@pytest.mark.skipif(not HAS_RUST, reason="genalyzer._fourier_rs not built")
class TestBenchmark:
    """Timing comparison — informational, not a hard assertion."""

    N_CALLS = 1000
    SIG_LEN = 1024

    def _time_python(self, sig: np.ndarray) -> float:
        t0 = time.perf_counter()
        for _ in range(self.N_CALLS):
            _py_power_spectrum(sig)
        return time.perf_counter() - t0

    def _time_rust(self, sig: np.ndarray) -> float:
        a = _rs.FourierAnalyzer(len(sig))
        sig_list = sig.tolist()  # conversion outside the timed loop
        t0 = time.perf_counter()
        for _ in range(self.N_CALLS):
            a.power_spectrum(sig_list)
        return time.perf_counter() - t0

    def test_rust_repeated_calls(self, capsys):
        sig = np.sin(2 * np.pi * 5 * np.arange(self.SIG_LEN) / self.SIG_LEN)
        py_time = self._time_python(sig)
        rs_time = self._time_rust(sig)
        speedup = py_time / rs_time if rs_time > 0 else float("inf")
        with capsys.disabled():
            print(
                f"\n[benchmark] {self.N_CALLS}×power_spectrum(n={self.SIG_LEN}): "
                f"Python={py_time:.3f}s  Rust={rs_time:.3f}s  "
                f"speedup={speedup:.1f}x"
            )
        # Soft assertion: Rust should not be >5× slower than numpy.
        assert rs_time < py_time * 5, (
            f"Rust path unexpectedly slow: {rs_time:.3f}s vs Python {py_time:.3f}s"
        )
