# Copyright (C) 2024-2026 Analog Devices, Inc.
#
# SPDX short identifier: ADIBSD OR GPL-2.0-or-later
"""Plot helpers for MCP analyzer tools. Matplotlib is imported lazily."""

from __future__ import annotations

import numpy as np


def plot_spectrum(
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

    import genalyzer as gn

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
    for key, unit in (("sfdr", " dB"), ("snr", " dB"), ("thd", " dB"), ("enob", " bits")):
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


def plot_histogram(hist: np.ndarray, plot_path: str, title: str = "Code Histogram") -> None:
    """Render a code-histogram bar plot and save as PNG."""
    import matplotlib

    matplotlib.use("Agg")
    import matplotlib.pyplot as plt

    fig, ax = plt.subplots(figsize=(12, 5))
    ax.bar(np.arange(len(hist)), hist, width=1.0, color="steelblue")
    ax.set_xlabel("Code bin")
    ax.set_ylabel("Count")
    ax.set_title(title)
    ax.grid(True, alpha=0.3)
    plt.tight_layout()
    plt.savefig(plot_path, dpi=150)
    plt.close(fig)
