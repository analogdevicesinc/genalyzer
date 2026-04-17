# Copyright (C) 2024-2026 Analog Devices, Inc.
#
# SPDX short identifier: ADIBSD OR GPL-2.0-or-later
"""DNL / INL MCP tools."""

from __future__ import annotations

from pathlib import Path

import numpy as np

from .io import load_array, save_array
from .server import _require_genalyzer, mcp


@mcp.tool()
def compute_dnl(
    hist_npy_path: str,
    signal_type: str = "tone",
    output_path: str | None = None,
) -> dict:
    """Compute DNL from a histogram .npy produced by compute_histogram.

    Args:
        hist_npy_path: Path to a .npy histogram.
        signal_type: "tone" (default) or "ramp".
        output_path: Path to save DNL .npy. Auto-generated if omitted.
    """
    _require_genalyzer()
    import genalyzer as gn

    if not Path(hist_npy_path).exists():
        return {"error": f"Histogram file not found: {hist_npy_path}"}

    sig = {
        "tone": gn.DnlSignal.TONE,
        "ramp": gn.DnlSignal.RAMP,
    }.get(signal_type.lower())
    if sig is None:
        return {"error": f"Unknown signal_type: {signal_type}"}

    try:
        hist = load_array(hist_npy_path)
        dnl = gn.dnl(np.asarray(hist).astype(np.uint64), sig)

        if output_path is None:
            output_path = str(Path(hist_npy_path).with_suffix(".dnl.npy"))

        save_array(output_path, np.asarray(dnl))
        return {"output_path": output_path}
    except Exception as e:
        return {"error": f"DNL computation failed: {e}"}


@mcp.tool()
def compute_inl(
    dnl_npy_path: str,
    fit: str = "best_fit",
    output_path: str | None = None,
) -> dict:
    """Compute INL from a DNL .npy produced by compute_dnl.

    Args:
        dnl_npy_path: Path to a .npy DNL array.
        fit: "best_fit" (default) or "end_point" (maps to END_FIT).
        output_path: Path to save INL .npy. Auto-generated if omitted.
    """
    _require_genalyzer()
    import genalyzer as gn

    if not Path(dnl_npy_path).exists():
        return {"error": f"DNL file not found: {dnl_npy_path}"}

    fit_enum = {
        "best_fit": gn.InlLineFit.BEST_FIT,
        "end_point": gn.InlLineFit.END_FIT,
    }.get(fit.lower())
    if fit_enum is None:
        return {"error": f"Unknown fit type: {fit}"}

    try:
        dnl_arr = load_array(dnl_npy_path)
        inl = gn.inl(np.asarray(dnl_arr).astype(np.float64), fit_enum)

        if output_path is None:
            output_path = str(Path(dnl_npy_path).with_suffix(".inl.npy"))

        save_array(output_path, np.asarray(inl))
        return {"output_path": output_path}
    except Exception as e:
        return {"error": f"INL computation failed: {e}"}


@mcp.tool()
def analyze_dnl(
    npy_path: str,
    nbits: int,
    signal_type: str = "tone",
    code_format: str = "twos_complement",
    plot: bool = False,
) -> dict:
    """All-in-one DNL analysis from a raw code-samples .npy file.

    Loads samples, builds a histogram, computes DNL, returns summary metrics
    (dnl_max, dnl_min, dnl_abs_max). When plot=True, also renders a DNL PNG.
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

    sig = {
        "tone": gn.DnlSignal.TONE,
        "ramp": gn.DnlSignal.RAMP,
    }.get(signal_type.lower())
    if sig is None:
        return {"error": f"Unknown signal_type: {signal_type}"}

    try:
        data = load_array(npy_path)
        hist = gn.hist(data.astype(np.int64), nbits, fmt)
        dnl = np.asarray(gn.dnl(np.asarray(hist).astype(np.uint64), sig))

        results = gn.dnl_analysis(dnl)

        metrics: dict[str, object] = {
            "dnl_max": float(np.max(dnl)),
            "dnl_min": float(np.min(dnl)),
            "dnl_abs_max": float(np.max(np.abs(dnl))),
        }
        metrics["results"] = {k: v for k, v in results.items() if isinstance(k, str)}

        dnl_path = str(Path(npy_path).with_suffix(".dnl.npy"))
        save_array(dnl_path, dnl)
        metrics["output_path"] = dnl_path

        if plot:
            from .plots import plot_dnl

            plot_path = str(Path(npy_path).with_suffix(".dnl.png"))
            try:
                plot_dnl(dnl, plot_path)
                metrics["plot_path"] = plot_path
            except Exception as plot_err:
                metrics["plot_warning"] = f"Plot generation failed: {plot_err}"

        return metrics
    except Exception as e:
        return {"error": f"DNL analysis failed: {e}"}


@mcp.tool()
def analyze_inl(
    npy_path: str,
    nbits: int,
    signal_type: str = "tone",
    code_format: str = "twos_complement",
    fit: str = "best_fit",
    plot: bool = False,
) -> dict:
    """All-in-one INL analysis from a raw code-samples .npy file."""
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

    sig = {
        "tone": gn.DnlSignal.TONE,
        "ramp": gn.DnlSignal.RAMP,
    }.get(signal_type.lower())
    if sig is None:
        return {"error": f"Unknown signal_type: {signal_type}"}

    fit_enum = {
        "best_fit": gn.InlLineFit.BEST_FIT,
        "end_point": gn.InlLineFit.END_FIT,
    }.get(fit.lower())
    if fit_enum is None:
        return {"error": f"Unknown fit type: {fit}"}

    try:
        data = load_array(npy_path)
        hist = gn.hist(data.astype(np.int64), nbits, fmt)
        dnl = np.asarray(gn.dnl(np.asarray(hist).astype(np.uint64), sig))
        inl = np.asarray(gn.inl(dnl.astype(np.float64), fit_enum))

        results = gn.inl_analysis(inl)

        metrics: dict[str, object] = {
            "inl_max": float(np.max(inl)),
            "inl_min": float(np.min(inl)),
            "inl_abs_max": float(np.max(np.abs(inl))),
        }
        metrics["results"] = {k: v for k, v in results.items() if isinstance(k, str)}

        inl_path = str(Path(npy_path).with_suffix(".inl.npy"))
        save_array(inl_path, inl)
        metrics["output_path"] = inl_path

        if plot:
            from .plots import plot_inl

            plot_path = str(Path(npy_path).with_suffix(".inl.png"))
            try:
                plot_inl(inl, plot_path)
                metrics["plot_path"] = plot_path
            except Exception as plot_err:
                metrics["plot_warning"] = f"Plot generation failed: {plot_err}"

        return metrics
    except Exception as e:
        return {"error": f"INL analysis failed: {e}"}
