# Copyright (C) 2024-2026 Analog Devices, Inc.
#
# SPDX short identifier: ADIBSD OR GPL-2.0-or-later
"""Top-level genalyzer CLI. Parallel surface to the MCP server for AI clients."""

from __future__ import annotations

import click

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
