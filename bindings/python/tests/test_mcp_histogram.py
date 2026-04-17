# Copyright (C) 2024-2026 Analog Devices, Inc.
#
# SPDX short identifier: ADIBSD OR GPL-2.0-or-later
"""Tests for MCP histogram tools."""

import sys
import pytest
import numpy as np

pytestmark = [
    pytest.mark.genalyzer,
    pytest.mark.skipif(sys.version_info < (3, 10), reason="fastmcp requires Python 3.10+"),
]


class TestComputeHistogram:
    def test_histogram_ramp_is_approximately_uniform(self, synthetic_ramp, tmp_path):
        from genalyzer.mcp.histogram import compute_histogram

        out = str(tmp_path / "hist.npy")
        result = compute_histogram(
            npy_path=synthetic_ramp["path"],
            nbits=synthetic_ramp["nbits"],
            output_path=out,
        )
        assert "error" not in result, result
        assert "output_path" in result
        hist = np.load(result["output_path"])
        assert np.all(hist > 0)


class TestAnalyzeHistogram:
    def test_analyze_histogram_no_plot_by_default(self, synthetic_ramp):
        from genalyzer.mcp.histogram import analyze_histogram

        result = analyze_histogram(
            npy_path=synthetic_ramp["path"],
            nbits=synthetic_ramp["nbits"],
        )
        assert "error" not in result, result
        # gn.hist_analysis returns: sum, first_nz_index, last_nz_index, nz_range
        assert "sum" in result and "nz_range" in result
        assert "plot_path" not in result

    def test_analyze_histogram_with_plot(self, synthetic_ramp):
        from pathlib import Path
        from genalyzer.mcp.histogram import analyze_histogram

        result = analyze_histogram(
            npy_path=synthetic_ramp["path"],
            nbits=synthetic_ramp["nbits"],
            plot=True,
        )
        assert "error" not in result
        assert "plot_path" in result
        assert Path(result["plot_path"]).exists()
        assert Path(result["plot_path"]).stat().st_size > 100
