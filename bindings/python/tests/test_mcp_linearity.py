# Copyright (C) 2024-2026 Analog Devices, Inc.
#
# SPDX short identifier: ADIBSD OR GPL-2.0-or-later
"""Tests for MCP DNL/INL tools."""

import sys

import numpy as np
import pytest

pytestmark = [
    pytest.mark.genalyzer,
    pytest.mark.skipif(
        sys.version_info < (3, 10), reason="fastmcp requires Python 3.10+"
    ),
]


class TestComputeDNL:
    def test_dnl_from_histogram(self, synthetic_ramp, tmp_path):
        from genalyzer.mcp.histogram import compute_histogram
        from genalyzer.mcp.linearity import compute_dnl

        hist_out = str(tmp_path / "h.npy")
        compute_histogram(
            npy_path=synthetic_ramp["path"],
            nbits=synthetic_ramp["nbits"],
            output_path=hist_out,
        )

        dnl_out = str(tmp_path / "dnl.npy")
        result = compute_dnl(
            hist_npy_path=hist_out,
            signal_type="ramp",
            output_path=dnl_out,
        )
        assert "error" not in result, result
        dnl = np.load(result["output_path"])
        # Endpoint codes are marked -1.0 by genalyzer for ramp signal; inner codes are 0.0
        assert np.max(np.abs(dnl[1:-1])) < 0.5


class TestComputeINL:
    def test_inl_from_dnl(self, synthetic_ramp, tmp_path):
        from genalyzer.mcp.histogram import compute_histogram
        from genalyzer.mcp.linearity import compute_dnl, compute_inl

        hist_out = str(tmp_path / "h.npy")
        compute_histogram(
            npy_path=synthetic_ramp["path"],
            nbits=synthetic_ramp["nbits"],
            output_path=hist_out,
        )

        dnl_out = str(tmp_path / "dnl.npy")
        compute_dnl(hist_npy_path=hist_out, signal_type="ramp", output_path=dnl_out)

        inl_out = str(tmp_path / "inl.npy")
        result = compute_inl(dnl_npy_path=dnl_out, output_path=inl_out)
        assert "error" not in result, result
        inl = np.load(result["output_path"])
        assert np.max(np.abs(inl)) < 1.0


class TestAnalyzeDNL:
    def test_analyze_dnl_no_plot(self, synthetic_ramp):
        from genalyzer.mcp.linearity import analyze_dnl

        result = analyze_dnl(
            npy_path=synthetic_ramp["path"],
            nbits=synthetic_ramp["nbits"],
        )
        assert "error" not in result, result
        assert "dnl_max" in result
        assert "plot_path" not in result

    def test_analyze_dnl_with_plot(self, synthetic_ramp):
        from pathlib import Path

        from genalyzer.mcp.linearity import analyze_dnl

        result = analyze_dnl(
            npy_path=synthetic_ramp["path"],
            nbits=synthetic_ramp["nbits"],
            plot=True,
        )
        assert "plot_path" in result
        assert Path(result["plot_path"]).exists()


class TestAnalyzeINL:
    def test_analyze_inl_no_plot(self, synthetic_ramp):
        from genalyzer.mcp.linearity import analyze_inl

        result = analyze_inl(
            npy_path=synthetic_ramp["path"],
            nbits=synthetic_ramp["nbits"],
        )
        assert "error" not in result, result
        assert "inl_max" in result
        assert "plot_path" not in result

    def test_analyze_inl_with_plot(self, synthetic_ramp):
        from pathlib import Path

        from genalyzer.mcp.linearity import analyze_inl

        result = analyze_inl(
            npy_path=synthetic_ramp["path"],
            nbits=synthetic_ramp["nbits"],
            plot=True,
        )
        assert "plot_path" in result
        assert Path(result["plot_path"]).exists()
