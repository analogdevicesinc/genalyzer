# Copyright (C) 2024-2026 Analog Devices, Inc.
#
# SPDX short identifier: ADIBSD OR GPL-2.0-or-later
"""Shared pytest fixtures for genalyzer test suite."""

import numpy as np
import pytest


@pytest.fixture
def synthetic_tone(tmp_path):
    """Write a clean complex tone to tmp_path/tone.npy and return the path."""
    sample_rate = 250e6
    num_points = 8192
    tone_freq = 30e6
    amplitude = 0.9
    t = np.arange(num_points) / sample_rate
    tone = amplitude * np.exp(2j * np.pi * tone_freq * t)
    path = str(tmp_path / "tone.npy")
    np.save(path, tone)
    return {"path": path, "sample_rate": sample_rate, "tone_freq": tone_freq}


@pytest.fixture
def synthetic_ramp(tmp_path):
    """Write an ideal 12-bit ramp to tmp_path/ramp.npy and return the path."""
    nbits = 12
    levels = 1 << nbits
    samples_per_code = 64
    codes = np.repeat(np.arange(-levels // 2, levels // 2), samples_per_code).astype(np.int32)
    path = str(tmp_path / "ramp.npy")
    np.save(path, codes)
    return {"path": path, "nbits": nbits}


@pytest.fixture
def synthetic_gaussian(tmp_path):
    """Write 65536 samples of AWGN to tmp_path/gauss.npy and return the path."""
    rng = np.random.default_rng(seed=0)
    samples = rng.normal(loc=0.0, scale=0.1, size=65536).astype(np.float64)
    path = str(tmp_path / "gauss.npy")
    np.save(path, samples)
    return {"path": path}
