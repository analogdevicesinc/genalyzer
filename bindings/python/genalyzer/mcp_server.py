# Copyright (C) 2024-2026 Analog Devices, Inc.
#
# SPDX short identifier: ADIBSD OR GPL-2.0-or-later
"""Backward-compatibility shim — real implementation lives in genalyzer.mcp."""

from genalyzer.mcp.fourier import analyze_spectrum, compute_fft, get_fa_metrics
from genalyzer.mcp.generators import generate_test_tone
from genalyzer.mcp.server import main, mcp

__all__ = [
    "analyze_spectrum",
    "compute_fft",
    "generate_test_tone",
    "get_fa_metrics",
    "main",
    "mcp",
]
