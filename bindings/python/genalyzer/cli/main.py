# Copyright (C) 2024-2026 Analog Devices, Inc.
#
# SPDX short identifier: ADIBSD OR GPL-2.0-or-later
"""Top-level genalyzer CLI. Parallel surface to the MCP server for AI clients."""

from __future__ import annotations

import click

from ..mcp.fourier import analyze_spectrum, compute_fft, get_fa_metrics
from ..mcp.generators import (
    generate_gaussian_noise,
    generate_ramp,
    generate_real_tone,
    generate_test_tone,
)
from ..mcp.histogram import analyze_histogram, compute_histogram
from ..mcp.linearity import analyze_dnl, analyze_inl, compute_dnl, compute_inl
from ..mcp.quantize import quantize
from ..mcp.waveform import analyze_waveform, compute_waveform_stats
from ._builder import click_from_tool

_TOOL_NAMES = [
    "generate_test_tone",
    "generate_real_tone",
    "generate_ramp",
    "generate_gaussian_noise",
    "quantize",
    "compute_fft",
    "get_fa_metrics",
    "analyze_spectrum",
    "compute_histogram",
    "analyze_histogram",
    "compute_dnl",
    "compute_inl",
    "analyze_dnl",
    "analyze_inl",
    "compute_waveform_stats",
    "analyze_waveform",
]


@click.group()
@click.version_option(package_name="genalyzer")
def cli():
    """genalyzer: parallel surface to the MCP server for AI clients.

    Every subcommand wraps a genalyzer.mcp tool, accepts the same parameters
    as kebab-case CLI flags, and emits the tool's return dict as JSON on
    stdout. Use `--compact` for single-line JSON.
    """


@cli.command("tools")
def _tools_cmd():
    """Print the 16 MCP tool names (one per line)."""
    click.echo("\n".join(_TOOL_NAMES))


# ---- generators ----
@cli.group()
def generators():
    """Synthetic waveform generators."""


generators.add_command(click_from_tool(generate_test_tone, "test-tone"))
generators.add_command(click_from_tool(generate_real_tone, "real-tone"))
generators.add_command(click_from_tool(generate_ramp, "ramp"))
generators.add_command(click_from_tool(generate_gaussian_noise, "gaussian-noise"))


# ---- quantize (standalone) ----
cli.add_command(click_from_tool(quantize, "quantize"))


# ---- fourier ----
@cli.group()
def fourier():
    """Fourier-domain analysis."""


fourier.add_command(click_from_tool(compute_fft, "fft"))
fourier.add_command(click_from_tool(get_fa_metrics, "fa-metrics"))
fourier.add_command(click_from_tool(analyze_spectrum, "analyze"))


# ---- histogram ----
@cli.group()
def histogram():
    """Code-density histogram tools."""


histogram.add_command(click_from_tool(compute_histogram, "compute"))
histogram.add_command(click_from_tool(analyze_histogram, "analyze"))


# ---- linearity ----
@cli.group()
def linearity():
    """DNL / INL tools."""


linearity.add_command(click_from_tool(compute_dnl, "compute-dnl"))
linearity.add_command(click_from_tool(compute_inl, "compute-inl"))
linearity.add_command(click_from_tool(analyze_dnl, "analyze-dnl"))
linearity.add_command(click_from_tool(analyze_inl, "analyze-inl"))


# ---- waveform ----
@cli.group()
def waveform():
    """Time-domain waveform tools."""


waveform.add_command(click_from_tool(compute_waveform_stats, "stats"))
waveform.add_command(click_from_tool(analyze_waveform, "analyze"))
