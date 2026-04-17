# Copyright (C) 2024-2026 Analog Devices, Inc.
#
# SPDX short identifier: ADIBSD OR GPL-2.0-or-later
"""Tests for MCP waveform (time-domain) tools."""

import sys
import pytest
import numpy as np

pytestmark = [
    pytest.mark.genalyzer,
    pytest.mark.skipif(sys.version_info < (3, 10), reason="fastmcp requires Python 3.10+"),
]


class TestComputeWaveformStats:
    def test_stats_on_tone(self, synthetic_tone):
        from genalyzer.mcp.waveform import compute_waveform_stats

        result = compute_waveform_stats(npy_path=synthetic_tone["path"])
        assert "error" not in result, result
        # gn.wf_analysis keys: min, max, mid, range, avg, rms, rmsac
        for key in ("min", "max", "avg", "rms"):
            assert key in result


class TestAnalyzeWaveform:
    def test_analyze_waveform_no_plot(self, synthetic_tone):
        from genalyzer.mcp.waveform import analyze_waveform

        result = analyze_waveform(npy_path=synthetic_tone["path"])
        assert "error" not in result, result
        assert "plot_path" not in result
