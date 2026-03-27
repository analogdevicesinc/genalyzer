# Copyright (C) 2024-2025 Analog Devices, Inc.
#
# SPDX short identifier: ADIBSD OR GPL-2.0-or-later
"""Tests for genalyzer MCP server."""

import pytest
import numpy as np


pytestmark = pytest.mark.genalyzer


@pytest.fixture
def mcp_server():
    """Create and return the MCP server instance."""
    from genalyzer.mcp_server import mcp

    return mcp


def _get_tool_names(mcp_server):
    """Extract tool names from the MCP server."""
    import asyncio

    tools = asyncio.run(mcp_server.list_tools())
    return [t.name for t in tools]


class TestServerRegistration:
    """Test that the MCP server has the expected tools registered."""

    def test_server_has_tools(self, mcp_server):
        tool_names = _get_tool_names(mcp_server)
        expected = ["analyze_spectrum", "compute_fft", "get_fa_metrics", "generate_test_tone"]
        for name in expected:
            assert name in tool_names, f"Tool '{name}' not registered in MCP server"

    def test_server_name(self, mcp_server):
        assert mcp_server.name == "genalyzer"


class TestGenerateTestTone:
    """Test the generate_test_tone tool."""

    def test_generate_test_tone(self, tmp_path):
        from genalyzer.mcp_server import generate_test_tone

        output_path = str(tmp_path / "test_tone.npy")
        result = generate_test_tone(
            num_points=4096,
            sample_rate=250e6,
            tone_freq=97.3e6,
            amplitude=0.9,
            output_path=output_path,
        )
        assert "output_path" in result
        data = np.load(result["output_path"])
        assert data.shape == (4096,)
        assert data.dtype == np.complex128

    def test_generate_test_tone_default_path(self, tmp_path, monkeypatch):
        import tempfile

        monkeypatch.setattr(tempfile, "gettempdir", lambda: str(tmp_path))
        from genalyzer.mcp_server import generate_test_tone

        result = generate_test_tone(
            num_points=1024,
            sample_rate=100e6,
            tone_freq=10e6,
        )
        assert "output_path" in result
        data = np.load(result["output_path"])
        assert data.shape == (1024,)


class TestAnalyzeSpectrum:
    """Test the analyze_spectrum tool."""

    def test_analyze_spectrum_missing_file(self):
        from genalyzer.mcp_server import analyze_spectrum

        result = analyze_spectrum(
            npy_path="/nonexistent/path/data.npy",
            sample_rate=250e6,
        )
        assert "error" in result

    def test_analyze_spectrum_synthetic(self, tmp_path):
        from genalyzer.mcp_server import generate_test_tone, analyze_spectrum

        # Generate a synthetic tone
        tone_path = str(tmp_path / "tone.npy")
        generate_test_tone(
            num_points=8192,
            sample_rate=250e6,
            tone_freq=30e6,
            amplitude=0.9,
            output_path=tone_path,
        )

        # Analyze it
        result = analyze_spectrum(
            npy_path=tone_path,
            sample_rate=250e6,
        )
        assert "error" not in result, f"Unexpected error: {result.get('error')}"
        for key in ("sfdr", "snr", "thd", "enob"):
            assert key in result, f"Missing metric key '{key}' in result"
        # SFDR should be positive for a clean tone
        assert result["sfdr"] > 0

    def test_analyze_spectrum_returns_plot(self, tmp_path):
        from pathlib import Path
        from genalyzer.mcp_server import generate_test_tone, analyze_spectrum

        tone_path = str(tmp_path / "tone.npy")
        generate_test_tone(
            num_points=8192,
            sample_rate=250e6,
            tone_freq=30e6,
            amplitude=0.9,
            output_path=tone_path,
        )

        result = analyze_spectrum(npy_path=tone_path, sample_rate=250e6)
        assert "error" not in result
        assert "plot_path" in result, "analyze_spectrum must return 'plot_path'"
        assert Path(result["plot_path"]).exists(), f"Plot file not found: {result['plot_path']}"
        assert result["plot_path"].endswith(".png")


class TestComputeFFT:
    """Test the compute_fft tool."""

    def test_compute_fft_output(self, tmp_path):
        from genalyzer.mcp_server import generate_test_tone, compute_fft

        tone_path = str(tmp_path / "tone.npy")
        generate_test_tone(
            num_points=4096,
            sample_rate=250e6,
            tone_freq=30e6,
            amplitude=0.9,
            output_path=tone_path,
        )

        fft_out = str(tmp_path / "fft_out.npy")
        result = compute_fft(
            npy_path=tone_path,
            sample_rate=250e6,
            output_path=fft_out,
        )
        assert "error" not in result, f"Unexpected error: {result.get('error')}"
        assert "output_path" in result
        fft_data = np.load(result["output_path"])
        assert len(fft_data) > 0


class TestGetFAMetrics:
    """Test the get_fa_metrics tool."""

    def test_get_fa_metrics_keys(self, tmp_path):
        from genalyzer.mcp_server import generate_test_tone, compute_fft, get_fa_metrics

        tone_path = str(tmp_path / "tone.npy")
        generate_test_tone(
            num_points=8192,
            sample_rate=250e6,
            tone_freq=30e6,
            amplitude=0.9,
            output_path=tone_path,
        )

        fft_out = str(tmp_path / "fft_out.npy")
        compute_fft(
            npy_path=tone_path,
            sample_rate=250e6,
            output_path=fft_out,
        )

        result = get_fa_metrics(
            fft_npy_path=fft_out,
            sample_rate=250e6,
            tone_freq=30e6,
        )
        assert "error" not in result, f"Unexpected error: {result.get('error')}"
        for key in ("sfdr", "snr", "thd", "enob"):
            assert key in result, f"Missing metric key '{key}' in result"
