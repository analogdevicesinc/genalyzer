# Copyright (C) 2024-2025 Analog Devices, Inc.
#
# SPDX short identifier: ADIBSD OR GPL-2.0-or-later
"""Fourier analysis helpers with optional Rust acceleration.

This module exposes :class:`FourierAnalyzer` and four free functions:

- :func:`forward_fft`   — forward FFT (unnormalized)
- :func:`inverse_fft`   — inverse FFT (normalized by 1/N)
- :func:`power_spectrum` — magnitude-squared spectrum
- :func:`frequencies`   — frequency bin centres

When the ``genalyzer._fourier_rs`` extension module is available (built with
``maturin develop --features python`` from ``rust/fourier/``), the Rust
implementation is used automatically.  Otherwise the module falls back to a
pure-Python implementation backed by ``numpy.fft``.

The two implementations are numerically equivalent within floating-point
precision (typically < 1e-12 relative error).
"""

from __future__ import annotations

import numpy as np

# ── Try to import the Rust extension ─────────────────────────────────────────
# Two install paths are supported:
#   1. Production: genalyzer._fourier_rs  (maturin + module-name setting)
#   2. Development: _fourier_rs           (maturin develop, top-level package)
# Catch ImportError (extension not built) and OSError / FileNotFoundError
# (C library not found when the parent package's __init__ runs first).
_rs = None  # type: ignore[assignment]
_HAS_RUST = False

try:
    from genalyzer import _fourier_rs as _rs  # type: ignore[attr-defined]

    _HAS_RUST = True
except (ImportError, OSError):
    try:
        import _fourier_rs as _rs  # type: ignore[no-redef]

        _HAS_RUST = True
    except (ImportError, OSError):
        pass


# ── Pure-Python fallback implementations ─────────────────────────────────────


def _py_forward_fft(input_data: np.ndarray) -> np.ndarray:
    """Forward FFT backed by numpy (unnormalized)."""
    return np.fft.fft(np.asarray(input_data, dtype=np.float64))


def _py_inverse_fft(spectrum: np.ndarray) -> np.ndarray:
    """Inverse FFT backed by numpy, normalized by 1/N, real part only."""
    return np.fft.ifft(np.asarray(spectrum, dtype=complex)).real


def _py_power_spectrum(input_data: np.ndarray) -> np.ndarray:
    """Power spectrum (|FFT|²) backed by numpy."""
    spectrum = _py_forward_fft(input_data)
    return (spectrum.real**2 + spectrum.imag**2).astype(np.float64)


def _py_frequencies(n: int, sample_rate: float) -> np.ndarray:
    """Frequency bin centres using the DC-left convention."""
    bin_width = sample_rate / n
    return np.arange(n, dtype=np.float64) * bin_width


# ── Helpers for converting between Python complex and (re, im) tuples ────────


def _to_pairs(arr: np.ndarray) -> list[tuple[float, float]]:
    a = np.asarray(arr, dtype=complex)
    return list(zip(a.real.tolist(), a.imag.tolist()))


def _from_pairs(pairs: list[tuple[float, float]]) -> np.ndarray:
    return np.array([re + 1j * im for re, im in pairs], dtype=complex)


# ── Public free functions ─────────────────────────────────────────────────────


def forward_fft(input_data) -> np.ndarray:
    """Forward FFT of a real-valued signal (unnormalized).

    Parameters
    ----------
    input_data:
        1-D array-like of real floats, length ``N``.

    Returns
    -------
    numpy.ndarray of complex128, length ``N``
        Raw (unnormalized) DFT output.
    """
    arr = np.asarray(input_data, dtype=np.float64)
    if _HAS_RUST:
        pairs = _rs.forward_fft(arr.tolist())
        return _from_pairs(pairs)
    return _py_forward_fft(arr)


def inverse_fft(spectrum) -> np.ndarray:
    """Inverse FFT, normalized by ``1/N``, returning the real part.

    Parameters
    ----------
    spectrum:
        1-D array-like of complex values, length ``N``.

    Returns
    -------
    numpy.ndarray of float64, length ``N``
    """
    arr = np.asarray(spectrum, dtype=complex)
    if _HAS_RUST:
        result = _rs.inverse_fft(_to_pairs(arr))
        return np.array(result, dtype=np.float64)
    return _py_inverse_fft(arr)


def power_spectrum(input_data) -> np.ndarray:
    """Magnitude-squared FFT spectrum.

    Parameters
    ----------
    input_data:
        1-D array-like of real floats, length ``N``.

    Returns
    -------
    numpy.ndarray of float64, length ``N``
        ``|FFT[k]|²`` for each bin ``k``.
    """
    arr = np.asarray(input_data, dtype=np.float64)
    if _HAS_RUST:
        return np.array(_rs.power_spectrum(arr.tolist()), dtype=np.float64)
    return _py_power_spectrum(arr)


def frequencies(n: int, sample_rate: float) -> np.ndarray:
    """Frequency bin centres for an ``n``-point FFT at ``sample_rate`` Hz.

    Uses the DC-left convention: bin ``k`` corresponds to
    ``k * sample_rate / n`` Hz.

    Parameters
    ----------
    n:
        FFT size.
    sample_rate:
        Sample rate in Hz.

    Returns
    -------
    numpy.ndarray of float64, length ``n``
        Frequencies in Hz for bins ``0, 1, …, n-1``.
    """
    if _HAS_RUST:
        return np.array(_rs.frequencies(n, float(sample_rate)), dtype=np.float64)
    return _py_frequencies(n, float(sample_rate))


# ── Stateful class ────────────────────────────────────────────────────────────


class FourierAnalyzer:
    """Fourier analyzer pre-planned for a fixed FFT size.

    Uses the Rust extension when available (no per-call planning overhead).
    Falls back to numpy otherwise.

    Parameters
    ----------
    n:
        FFT size.  All method calls are planned for this length.

    Examples
    --------
    >>> import numpy as np
    >>> from genalyzer.fourier import FourierAnalyzer
    >>> analyzer = FourierAnalyzer(64)
    >>> signal = np.sin(2 * np.pi * 5 * np.arange(64) / 64)
    >>> ps = analyzer.power_spectrum(signal)
    >>> ps.shape
    (64,)
    """

    def __init__(self, n: int) -> None:
        self._n = n
        self._rs_obj = _rs.FourierAnalyzer(n) if _HAS_RUST else None

    # ------------------------------------------------------------------
    def forward_fft(self, input_data) -> np.ndarray:
        """Forward FFT of a real-valued signal (unnormalized).

        Returns
        -------
        numpy.ndarray of complex128, length ``n``
        """
        arr = np.asarray(input_data, dtype=np.float64)
        if self._rs_obj is not None:
            pairs = self._rs_obj.forward_fft(arr.tolist())
            return _from_pairs(pairs)
        return _py_forward_fft(arr)

    def inverse_fft(self, spectrum) -> np.ndarray:
        """Inverse FFT, normalized by ``1/N``, real part only.

        Parameters
        ----------
        spectrum:
            1-D array-like of complex values, length ``n``.

        Returns
        -------
        numpy.ndarray of float64, length ``n``
        """
        arr = np.asarray(spectrum, dtype=complex)
        if self._rs_obj is not None:
            result = self._rs_obj.inverse_fft(_to_pairs(arr))
            return np.array(result, dtype=np.float64)
        return _py_inverse_fft(arr)

    def power_spectrum(self, input_data) -> np.ndarray:
        """Magnitude-squared FFT spectrum.

        Returns
        -------
        numpy.ndarray of float64, length ``n``
        """
        arr = np.asarray(input_data, dtype=np.float64)
        if self._rs_obj is not None:
            return np.array(self._rs_obj.power_spectrum(arr.tolist()), dtype=np.float64)
        return _py_power_spectrum(arr)

    @property
    def n(self) -> int:
        """FFT size."""
        return self._n

    def __repr__(self) -> str:
        backend = "rust" if self._rs_obj is not None else "numpy"
        return f"FourierAnalyzer(n={self._n}, backend={backend!r})"
