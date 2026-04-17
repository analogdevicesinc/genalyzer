# Copyright (C) 2024-2026 Analog Devices, Inc.
#
# SPDX short identifier: ADIBSD OR GPL-2.0-or-later
"""Time-domain waveform MCP tools."""

from __future__ import annotations

from pathlib import Path

import numpy as np

from .io import load_array
from .server import _require_genalyzer, mcp


def _stats_dict(data: np.ndarray) -> dict:
    _require_genalyzer()
    import genalyzer as gn

    # gn.wf_analysis returns a dict of time-domain stats keyed by string names.
    results = gn.wf_analysis(np.real(np.asarray(data, dtype=np.float64)))
    metrics: dict[str, object] = {}
    # gn.wf_analysis keys: signaltype, min, max, mid, range, avg, rms, rmsac,
    # min_index, max_index. Surface the sample-value stats at the top level.
    for key in ("min", "max", "mid", "range", "avg", "rms", "rmsac"):
        if key in results:
            metrics[key] = results[key]
    metrics["results"] = {k: v for k, v in results.items() if isinstance(k, str)}
    return metrics


@mcp.tool()
def compute_waveform_stats(npy_path: str) -> dict:
    """Compute time-domain statistics (min/max/mid/range/avg/rms/rmsac) on a real waveform.

    Complex inputs are reduced to their real part before analysis.
    """
    _require_genalyzer()

    if not Path(npy_path).exists():
        return {"error": f"Input file not found: {npy_path}"}

    try:
        data = load_array(npy_path)
        return _stats_dict(data)
    except Exception as e:
        return {"error": f"Waveform analysis failed: {e}"}


@mcp.tool()
def analyze_waveform(npy_path: str, plot: bool = False) -> dict:
    """All-in-one time-domain waveform analysis.

    Currently identical to compute_waveform_stats; plot=True renders a
    scope-style PNG of the first 10_000 samples next to the input file.
    """
    _require_genalyzer()

    if not Path(npy_path).exists():
        return {"error": f"Input file not found: {npy_path}"}

    try:
        data = load_array(npy_path)
        metrics = _stats_dict(data)

        if plot:
            import matplotlib

            matplotlib.use("Agg")
            import matplotlib.pyplot as plt

            plot_path = str(Path(npy_path).with_suffix(".waveform.png"))
            n = min(10_000, len(data))
            fig, ax = plt.subplots(figsize=(12, 4))
            ax.plot(np.arange(n), np.real(data[:n]), linewidth=0.7, color="steelblue")
            ax.set_xlabel("Sample")
            ax.set_ylabel("Amplitude")
            ax.set_title("Waveform")
            ax.grid(True, alpha=0.3)
            plt.tight_layout()
            plt.savefig(plot_path, dpi=150)
            plt.close(fig)
            metrics["plot_path"] = plot_path

        return metrics
    except Exception as e:
        return {"error": f"Waveform analysis failed: {e}"}
