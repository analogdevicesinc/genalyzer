# Copyright (C) 2024-2026 Analog Devices, Inc.
#
# SPDX short identifier: ADIBSD OR GPL-2.0-or-later
"""MCP server for genalyzer spectral analysis library."""

from __future__ import annotations

import contextlib
import os
import tempfile
from pathlib import Path

import numpy as np
from fastmcp import FastMCP

# Try to import genalyzer native bindings; they require libgenalyzer to be installed.
try:
    import genalyzer as gn

    _HAS_GENALYZER = True
except (ImportError, OSError) as _import_err:
    _HAS_GENALYZER = False
    _gn_import_error = str(_import_err)

mcp = FastMCP("genalyzer")


def _require_genalyzer():
    """Raise a clear error if the native library is unavailable."""
    if not _HAS_GENALYZER:
        raise RuntimeError(
            f"genalyzer native library (libgenalyzer) is not available: {_gn_import_error}. "
            "Install libgenalyzer and reinstall the Python package."
        )


@mcp.tool()
def generate_test_tone(
    num_points: int,
    sample_rate: float,
    tone_freq: float,
    amplitude: float = 0.9,
    output_path: str | None = None,
) -> dict:
    """Generate a synthetic complex sinusoidal test tone and save to a .npy file.

    Args:
        num_points: Number of samples to generate.
        sample_rate: Sample rate in Hz.
        tone_freq: Tone frequency in Hz.
        amplitude: Amplitude (0.0 to 1.0, default 0.9).
        output_path: Path to save the .npy file. Auto-generated if not provided.

    Returns:
        Dictionary with output_path, num_points, sample_rate, and tone_freq.
    """
    if output_path is None:
        output_path = os.path.join(
            tempfile.gettempdir(), f"genalyzer_tone_{num_points}_{int(tone_freq)}.npy"
        )

    t = np.arange(num_points) / sample_rate
    tone = amplitude * np.exp(2j * np.pi * tone_freq * t)
    np.save(output_path, tone)

    return {
        "output_path": output_path,
        "num_points": num_points,
        "sample_rate": sample_rate,
        "tone_freq": tone_freq,
    }


@mcp.tool()
def compute_fft(
    npy_path: str,
    sample_rate: float,
    nfft: int | None = None,
    output_path: str | None = None,
) -> dict:
    """Compute FFT of time-domain data stored in a .npy file.

    Args:
        npy_path: Path to input .npy file containing time-domain samples.
        sample_rate: Sample rate in Hz.
        nfft: FFT size. Defaults to the length of the input data.
        output_path: Path to save the FFT result .npy file. Auto-generated if not provided.

    Returns:
        Dictionary with output_path, nfft, and sample_rate.
    """
    _require_genalyzer()

    if not Path(npy_path).exists():
        return {"error": f"Input file not found: {npy_path}"}

    try:
        data = np.load(npy_path)
        if nfft is None:
            nfft = len(data)

        if output_path is None:
            output_path = str(Path(npy_path).with_suffix(".fft.npy"))

        # Use genalyzer FFT — handle complex and real data
        if np.iscomplexobj(data):
            fft_out = gn.fft(data, 1, nfft, gn.Window.NO_WINDOW)
        else:
            fft_out = gn.rfft(data, 1, nfft, gn.Window.NO_WINDOW)

        np.save(output_path, fft_out)

        return {
            "output_path": output_path,
            "nfft": nfft,
            "sample_rate": sample_rate,
        }
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
    """Compute frequency analysis metrics from FFT data.

    Args:
        fft_npy_path: Path to .npy file containing FFT data.
        sample_rate: Sample rate in Hz.
        tone_freq: Fundamental tone frequency in Hz.
        max_harmonics: Maximum harmonic order to analyze (default 6).
        wo: Number of worst-other components. When 0 (default), worst-other
            tones are excluded from SFDR calculation. Set > 0 to include them.

    Returns:
        Dictionary with sfdr, snr, thd, enob, and additional metrics.
    """
    _require_genalyzer()

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

        # Extract key metrics
        metrics = {}

        if "sfdr" in results:
            metrics["sfdr"] = results["sfdr"]
        if "snr" in results:
            metrics["snr"] = results["snr"]
        if "sinad" in results:
            metrics["sinad"] = results["sinad"]
        if "thd_rss" in results:
            metrics["thd"] = results["thd_rss"]
        if "fbin" in results:
            metrics["fbin"] = results["fbin"]
        if "nsd" in results:
            metrics["nsd"] = results["nsd"]
        # ENOB is derived from SINAD: ENOB = (SINAD - 1.76) / 6.02
        if "sinad" in results:
            import math

            metrics["enob"] = (results["sinad"] - 1.76) / 6.02

        # Full results table for reference
        metrics["results"] = {k: v for k, v in results.items() if isinstance(k, str)}

        # Clean up
        with contextlib.suppress(Exception):
            gn.mgr_remove(test_key)

        return metrics
    except Exception as e:
        return {"error": f"Frequency analysis failed: {e}"}


def _plot_spectrum(
    fft_data: np.ndarray,
    nfft: int,
    sample_rate: float,
    results: dict,
    metrics: dict,
    plot_path: str,
) -> None:
    """Render an annotated spectrum plot and save it as a PNG file."""
    import matplotlib

    matplotlib.use("Agg")
    import matplotlib.pyplot as plt

    axis_type = gn.FreqAxisType.DC_CENTER
    axis_fmt = gn.FreqAxisFormat.FREQ

    freq_axis = gn.freq_axis(nfft, axis_type, sample_rate, axis_fmt)
    fft_db = gn.db(fft_data)
    if np.iscomplexobj(fft_data):
        fft_db = gn.fftshift(fft_db)

    annots = gn.fa_annotations(results, axis_type, axis_fmt)

    fig, ax = plt.subplots(figsize=(12, 5))
    freqs_mhz = np.asarray(freq_axis) / 1e6
    ax.plot(freqs_mhz, fft_db, linewidth=0.7, color="steelblue")
    ax.set_xlabel("Frequency (MHz)")
    ax.set_ylabel("Magnitude (dBFS)")
    ax.set_title("Spectrum Analysis")
    ax.grid(True, alpha=0.3)
    ax.set_xlim(freqs_mhz[0], freqs_mhz[-1])
    y_min = max(-160.0, float(np.min(fft_db)) - 10)
    y_max = float(np.max(fft_db)) + 10
    ax.set_ylim(y_min, y_max)

    for x, y, label in annots.get("labels", []):
        ax.annotate(
            label,
            xy=(x / 1e6, y),
            xytext=(0, 6),
            textcoords="offset points",
            ha="center",
            va="bottom",
            fontsize=7,
            arrowprops=dict(arrowstyle="-", color="gray", lw=0.5),
        )

    metric_lines = []
    for key, unit in (
        ("sfdr", " dB"),
        ("snr", " dB"),
        ("thd", " dB"),
        ("enob", " bits"),
    ):
        if key in metrics:
            metric_lines.append(f"{key.upper()} = {metrics[key]:.2f}{unit}")
    if metric_lines:
        ax.text(
            0.98,
            0.97,
            "\n".join(metric_lines),
            transform=ax.transAxes,
            fontsize=8,
            verticalalignment="top",
            horizontalalignment="right",
            bbox=dict(boxstyle="round,pad=0.4", facecolor="lightyellow", alpha=0.8),
            family="monospace",
        )

    plt.tight_layout()
    plt.savefig(plot_path, dpi=150)
    plt.close(fig)


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
) -> dict:
    """All-in-one spectral analysis: load data, compute FFT, and return metrics.

    Args:
        npy_path: Path to .npy file containing time-domain samples.
        sample_rate: Sample rate in Hz.
        nfft: FFT size. Defaults to the length of the input data.
        num_tones: Number of tones expected in the signal (default 1).
        max_harmonics: Maximum harmonic order to analyze (default 6).
        window: FFT window function. One of "no_window", "blackman_harris", "hann".
        wo: Number of worst-other components. When 0 (default), worst-other
            tones are excluded from SFDR calculation. Set > 0 to include them.
        ssb: Number of single-sideband bins for tone grouping. Defaults to
            12 to absorb energy spread from real hardware NCO imprecision.
            Set to 0 for ideal synthetic data where the tone is exactly
            coherent.

    Returns:
        Dictionary with sfdr, snr, thd, enob, and additional metrics.
    """
    _require_genalyzer()

    if not Path(npy_path).exists():
        return {"error": f"Input file not found: {npy_path}"}

    try:
        data = np.load(npy_path)
        if nfft is None:
            nfft = len(data)

        # Resolve window
        _window_map = {
            "no_window": gn.Window.NO_WINDOW,
            "blackman_harris": gn.Window.BLACKMAN_HARRIS,
            "hann": gn.Window.HANN,
        }
        win = _window_map.get(window.lower(), gn.Window.NO_WINDOW)

        # Normalize to [-1, 1] range — genalyzer expects normalized input
        # for complex128/float64 data (integer types are normalized internally).
        if data.dtype in (np.complex128, np.complex64, np.float64, np.float32):
            max_abs = np.max(np.abs(data))
            if max_abs > 0:
                data = data / max_abs

        # Compute FFT
        if np.iscomplexobj(data):
            fft_data = gn.fft(data, 1, nfft, win)
        else:
            fft_data = gn.rfft(data, 1, nfft, win)

        # Save FFT output alongside input
        fft_path = str(Path(npy_path).with_suffix(".fft.npy"))
        np.save(fft_path, fft_data)

        # Set up frequency analysis
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

        # Extract key metrics by exact key name from fft_analysis results
        metrics: dict[str, object] = {"fft_output_path": fft_path}

        if "sfdr" in results:
            metrics["sfdr"] = results["sfdr"]
        if "snr" in results:
            metrics["snr"] = results["snr"]
        if "sinad" in results:
            metrics["sinad"] = results["sinad"]
        if "thd_rss" in results:
            metrics["thd"] = results["thd_rss"]
        if "fbin" in results:
            metrics["fbin"] = results["fbin"]
        if "nsd" in results:
            metrics["nsd"] = results["nsd"]
        # ENOB is derived from SINAD: ENOB = (SINAD - 1.76) / 6.02
        if "sinad" in results:
            import math

            metrics["enob"] = (results["sinad"] - 1.76) / 6.02

        # Full results table for reference
        metrics["results"] = {k: v for k, v in results.items() if isinstance(k, str)}

        # Generate annotated spectrum plot
        plot_path = str(Path(npy_path).with_suffix(".spectrum.png"))
        try:
            _plot_spectrum(fft_data, nfft, sample_rate, results, metrics, plot_path)
            metrics["plot_path"] = plot_path
        except Exception as plot_err:
            metrics["plot_warning"] = f"Plot generation failed: {plot_err}"

        # Clean up
        with contextlib.suppress(Exception):
            gn.mgr_remove(test_key)

        return metrics
    except Exception as e:
        return {"error": f"Spectrum analysis failed: {e}"}


def main():
    """Run the genalyzer MCP server."""
    mcp.run()


if __name__ == "__main__":
    main()
