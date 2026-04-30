# Copyright (C) 2024-2026 Analog Devices, Inc.
#
# SPDX short identifier: ADIBSD OR GPL-2.0-or-later
"""Quantize MCP tool — applies N-bit quantization to a waveform."""

from __future__ import annotations

from pathlib import Path

from .io import load_array, save_array
from .server import _require_genalyzer, mcp


@mcp.tool()
def quantize(
    npy_path: str,
    bits: int,
    fullscale: float = 1.0,
    noise: float = 0.0,
    output_path: str | None = None,
) -> dict:
    """Quantize a time-domain waveform to N-bit codes, saving as .npy.

    Args:
        npy_path: Path to input .npy or .csv time-domain samples.
        bits: Number of quantizer bits (e.g. 12, 14, 16).
        fullscale: Full-scale range of the quantizer (default 1.0).
        noise: Optional additive-noise standard deviation passed to the native
            gn.quantize() call (default 0.0 — ideal quantizer).
        output_path: Path to save the quantized .npy. Auto-generated if omitted.

    Returns:
        Dictionary with output_path, bits, and fullscale.
    """
    _require_genalyzer()
    import genalyzer as gn

    if not Path(npy_path).exists():
        return {"error": f"Input file not found: {npy_path}"}

    try:
        data = load_array(npy_path)
        if output_path is None:
            output_path = str(Path(npy_path).with_suffix(".q.npy"))

        quantized = gn.quantize(data, fullscale, bits, noise)
        save_array(output_path, quantized)

        return {
            "output_path": output_path,
            "bits": bits,
            "fullscale": fullscale,
        }
    except Exception as e:
        return {"error": f"Quantize failed: {e}"}
