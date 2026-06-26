# Copyright (C) 2024-2026 Analog Devices, Inc.
#
# SPDX short identifier: ADIBSD OR GPL-2.0-or-later
"""Tests for MCP generator and quantize tools."""

import sys

import numpy as np
import pytest

pytestmark = [
    pytest.mark.genalyzer,
    pytest.mark.skipif(
        sys.version_info < (3, 10), reason="fastmcp requires Python 3.10+"
    ),
]


class TestGenerateRealTone:
    def test_real_tone_shape_and_dtype(self, tmp_path):
        from genalyzer.mcp.generators import generate_real_tone

        out = str(tmp_path / "real_tone.npy")
        result = generate_real_tone(
            num_points=4096,
            sample_rate=250e6,
            tone_freq=30e6,
            amplitude=0.9,
            output_path=out,
        )
        assert "output_path" in result
        data = np.load(result["output_path"])
        assert data.shape == (4096,)
        assert not np.iscomplexobj(data)


class TestGenerateRamp:
    def test_ramp_monotonic(self, tmp_path):
        from genalyzer.mcp.generators import generate_ramp

        out = str(tmp_path / "ramp.npy")
        result = generate_ramp(
            num_points=1024,
            start=-1.0,
            stop=1.0,
            output_path=out,
        )
        assert "output_path" in result
        data = np.load(result["output_path"])
        assert data.shape == (1024,)
        assert np.all(np.diff(data) >= 0)
        assert np.isclose(data[0], -1.0)
        assert np.isclose(data[-1], 1.0)


class TestGenerateGaussianNoise:
    def test_gaussian_shape_and_stats(self, tmp_path):
        from genalyzer.mcp.generators import generate_gaussian_noise

        out = str(tmp_path / "gauss.npy")
        result = generate_gaussian_noise(
            num_points=65536,
            mean=0.0,
            std=0.1,
            seed=123,
            output_path=out,
        )
        data = np.load(result["output_path"])
        assert data.shape == (65536,)
        assert abs(np.mean(data)) < 0.01
        assert 0.08 < np.std(data) < 0.12


class TestQuantize:
    def test_quantize_12bit_ramp(self, tmp_path):
        from genalyzer.mcp.generators import generate_ramp
        from genalyzer.mcp.quantize import quantize

        ramp_path = str(tmp_path / "ramp.npy")
        generate_ramp(num_points=4096, start=-1.0, stop=1.0, output_path=ramp_path)

        out = str(tmp_path / "q.npy")
        result = quantize(
            npy_path=ramp_path,
            bits=12,
            fullscale=1.0,
            output_path=out,
        )
        assert "error" not in result, result
        assert "output_path" in result
        data = np.load(result["output_path"])
        assert data.dtype in (np.int16, np.int32, np.int64)
        assert data.min() >= -(1 << 11)
        assert data.max() <= (1 << 11) - 1
