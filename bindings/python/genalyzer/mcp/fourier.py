# Copyright (C) 2024-2026 Analog Devices, Inc.
#
# SPDX short identifier: ADIBSD OR GPL-2.0-or-later
"""Fourier-domain analysis MCP tools."""

from __future__ import annotations

import contextlib
from pathlib import Path

import numpy as np

from .io import load_array, save_array
from .plots import plot_spectrum
from .server import _require_genalyzer, mcp


@mcp.tool()
def compute_fft(
    npy_path: str,
    sample_rate: float,
    nfft: int | None = None,
    output_path: str | None = None,
) -> dict:
    """Compute FFT of time-domain data stored in a .npy or .csv file.

    Args:
        npy_path: Path to input .npy or .csv file containing time-domain samples.
        sample_rate: Sample rate in Hz.
        nfft: FFT size. Defaults to the length of the input data.
        output_path: Path to save the FFT result .npy file. Auto-generated if not provided.

    Returns:
        Dictionary with output_path, nfft, and sample_rate.
    """
    _require_genalyzer()
    import genalyzer as gn

    if not Path(npy_path).exists():
        return {"error": f"Input file not found: {npy_path}"}

    try:
        data = load_array(npy_path)
        if nfft is None:
            nfft = len(data)

        if output_path is None:
            output_path = str(Path(npy_path).with_suffix(".fft.npy"))

        if np.iscomplexobj(data):
            fft_out = gn.fft(data, 1, nfft, gn.Window.NO_WINDOW)
        else:
            fft_out = gn.rfft(data, 1, nfft, gn.Window.NO_WINDOW)

        save_array(output_path, fft_out)

        return {"output_path": output_path, "nfft": nfft, "sample_rate": sample_rate}
    except Exception as e:
        return {"error": f"FFT computation failed: {e}"}


@mcp.tool()
def get_fa_metrics(
    fft_npy_path: str,
    sample_rate: float,
    tone_freq: float,
    max_harmonics: int = 6,
    wo: int = 0,
) -> dict:
    """Compute frequency analysis metrics from a pre-computed FFT .npy file.

    Use this when you already have an FFT artifact and want metrics from it,
    for example in a sweep pipeline that reuses one FFT across many metric
    extractions.
    """
    _require_genalyzer()
    import genalyzer as gn

    if not Path(fft_npy_path).exists():
        return {"error": f"FFT file not found: {fft_npy_path}"}

    try:
        fft_data = np.load(fft_npy_path)
        nfft = len(fft_data) // 2 if not np.iscomplexobj(fft_data) else len(fft_data)

        test_key = "mcp_fa"
        gn.fa_create(test_key)
        gn.fa_fsample(test_key, sample_rate)
        gn.fa_fdata(test_key, tone_freq)
        gn.fa_hd(test_key, max_harmonics)
        gn.fa_ssb(test_key, gn.FaSsb.DEFAULT, 12)
        gn.fa_wo(test_key, wo)
        gn.fa_fund_images(test_key, True)
        gn.fa_max_tone(test_key, "A", gn.FaCompTag.SIGNAL, -1)

        results = gn.fft_analysis(test_key, fft_data, nfft)

        metrics: dict[str, object] = {}
        for key in ("sfdr", "snr", "sinad", "fbin", "nsd"):
            if key in results:
                metrics[key] = results[key]
        if "thd_rss" in results:
            metrics["thd"] = results["thd_rss"]
        if "sinad" in results:
            metrics["enob"] = (results["sinad"] - 1.76) / 6.02

        metrics["results"] = {k: v for k, v in results.items() if isinstance(k, str)}

        with contextlib.suppress(Exception):
            gn.mgr_remove(test_key)

        return metrics
    except Exception as e:
        return {"error": f"Frequency analysis failed: {e}"}


@mcp.tool()
def analyze_spectrum(
    npy_path: str,
    sample_rate: float,
    nfft: int | None = None,
    num_tones: int = 1,
    max_harmonics: int = 6,
    window: str = "no_window",
    ssb: int = 12,
    wo: int = 0,
    plot: bool = False,
) -> dict:
    """All-in-one spectral analysis: load data, compute FFT, and return metrics.

    When ``plot=True``, an annotated spectrum PNG is also written next to
    the input file.
    """
    _require_genalyzer()
    import genalyzer as gn

    if not Path(npy_path).exists():
        return {"error": f"Input file not found: {npy_path}"}

    try:
        data = load_array(npy_path)
        if nfft is None:
            nfft = len(data)

        _window_map = {
            "no_window": gn.Window.NO_WINDOW,
            "blackman_harris": gn.Window.BLACKMAN_HARRIS,
            "hann": gn.Window.HANN,
        }
        win = _window_map.get(window.lower(), gn.Window.NO_WINDOW)

        if data.dtype in (np.complex128, np.complex64, np.float64, np.float32):
            max_abs = np.max(np.abs(data))
            if max_abs > 0:
                data = data / max_abs

        if np.iscomplexobj(data):
            fft_data = gn.fft(data, 1, nfft, win)
        else:
            fft_data = gn.rfft(data, 1, nfft, win)

        fft_path = str(Path(npy_path).with_suffix(".fft.npy"))
        save_array(fft_path, fft_data)

        test_key = "mcp_analyze"
        gn.fa_create(test_key)
        gn.fa_fsample(test_key, sample_rate)
        gn.fa_hd(test_key, max_harmonics)
        if ssb > 0:
            gn.fa_ssb(test_key, gn.FaSsb.DEFAULT, ssb)
        gn.fa_wo(test_key, wo)
        gn.fa_fund_images(test_key, True)
        gn.fa_max_tone(test_key, "A", gn.FaCompTag.SIGNAL, -1)

        results = gn.fft_analysis(test_key, fft_data, nfft)

        metrics: dict[str, object] = {"fft_output_path": fft_path}
        for key in ("sfdr", "snr", "sinad", "fbin", "nsd"):
            if key in results:
                metrics[key] = results[key]
        if "thd_rss" in results:
            metrics["thd"] = results["thd_rss"]
        if "sinad" in results:
            metrics["enob"] = (results["sinad"] - 1.76) / 6.02

        metrics["results"] = {k: v for k, v in results.items() if isinstance(k, str)}

        if plot:
            plot_path = str(Path(npy_path).with_suffix(".spectrum.png"))
            try:
                plot_spectrum(fft_data, nfft, sample_rate, results, metrics, plot_path)
                metrics["plot_path"] = plot_path
            except Exception as plot_err:
                metrics["plot_warning"] = f"Plot generation failed: {plot_err}"

        with contextlib.suppress(Exception):
            gn.mgr_remove(test_key)

        return metrics
    except Exception as e:
        return {"error": f"Spectrum analysis failed: {e}"}
