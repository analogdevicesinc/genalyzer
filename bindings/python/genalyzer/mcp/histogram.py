# Copyright (C) 2024-2026 Analog Devices, Inc.
#
# SPDX short identifier: ADIBSD OR GPL-2.0-or-later
"""Histogram and code-density MCP tools."""

from __future__ import annotations

from pathlib import Path

import numpy as np

from .io import load_array, save_array
from .server import _require_genalyzer, mcp


@mcp.tool()
def compute_histogram(
    npy_path: str,
    nbits: int,
    code_format: str = "twos_complement",
    output_path: str | None = None,
) -> dict:
    """Compute the code histogram of integer samples.

    Args:
        npy_path: Path to .npy/.csv with integer code samples.
        nbits: Bit depth of the quantizer that produced the data.
        code_format: "twos_complement" (default) or "offset_binary".
        output_path: Path to save histogram .npy. Auto-generated if omitted.
    """
    _require_genalyzer()
    import genalyzer as gn

    if not Path(npy_path).exists():
        return {"error": f"Input file not found: {npy_path}"}

    fmt = {
        "twos_complement": gn.CodeFormat.TWOS_COMPLEMENT,
        "offset_binary": gn.CodeFormat.OFFSET_BINARY,
    }.get(code_format.lower())
    if fmt is None:
        return {"error": f"Unknown code_format: {code_format}"}

    try:
        data = load_array(npy_path)
        if output_path is None:
            output_path = str(Path(npy_path).with_suffix(".hist.npy"))

        hist = gn.hist(data.astype(np.int64), nbits, fmt)
        save_array(output_path, np.asarray(hist))
        return {
            "output_path": output_path,
            "nbits": nbits,
            "code_format": code_format,
        }
    except Exception as e:
        return {"error": f"Histogram computation failed: {e}"}


@mcp.tool()
def analyze_histogram(
    npy_path: str,
    nbits: int,
    code_format: str = "twos_complement",
    plot: bool = False,
) -> dict:
    """Compute histogram analysis metrics from integer code samples.

    Returns the four native ``gn.hist_analysis`` summary fields:
    ``sum`` (total sample count), ``first_nz_index`` / ``last_nz_index``
    (first/last non-empty bin), and ``nz_range`` (span of populated codes).
    When plot=True, also render a histogram PNG next to the input file.
    """
    _require_genalyzer()
    import genalyzer as gn

    if not Path(npy_path).exists():
        return {"error": f"Input file not found: {npy_path}"}

    fmt = {
        "twos_complement": gn.CodeFormat.TWOS_COMPLEMENT,
        "offset_binary": gn.CodeFormat.OFFSET_BINARY,
    }.get(code_format.lower())
    if fmt is None:
        return {"error": f"Unknown code_format: {code_format}"}

    try:
        data = load_array(npy_path)
        hist = gn.hist(data.astype(np.int64), nbits, fmt)
        hist_arr = np.asarray(hist)

        results = gn.hist_analysis(hist_arr)

        metrics: dict[str, object] = {}
        for key in ("sum", "first_nz_index", "last_nz_index", "nz_range"):
            if key in results:
                metrics[key] = results[key]
        metrics["results"] = {k: v for k, v in results.items() if isinstance(k, str)}

        hist_path = str(Path(npy_path).with_suffix(".hist.npy"))
        save_array(hist_path, hist_arr)
        metrics["output_path"] = hist_path

        if plot:
            from .plots import plot_histogram

            plot_path = str(Path(npy_path).with_suffix(".histogram.png"))
            try:
                plot_histogram(hist_arr, plot_path)
                metrics["plot_path"] = plot_path
            except Exception as plot_err:
                metrics["plot_warning"] = f"Plot generation failed: {plot_err}"

        return metrics
    except Exception as e:
        return {"error": f"Histogram analysis failed: {e}"}
