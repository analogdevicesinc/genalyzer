# Copyright (C) 2024-2026 Analog Devices, Inc.
#
# SPDX short identifier: ADIBSD OR GPL-2.0-or-later
"""Waveform generator MCP tools."""

from __future__ import annotations

import os
import tempfile

import numpy as np

from .server import mcp


@mcp.tool()
def generate_test_tone(
    num_points: int,
    sample_rate: float,
    tone_freq: float,
    amplitude: float = 0.9,
    output_path: str | None = None,
) -> dict:
    """Generate a synthetic complex sinusoidal test tone and save to a .npy file.

    Args:
        num_points: Number of samples to generate.
        sample_rate: Sample rate in Hz.
        tone_freq: Tone frequency in Hz.
        amplitude: Amplitude (0.0 to 1.0, default 0.9).
        output_path: Path to save the .npy file. Auto-generated if not provided.

    Returns:
        Dictionary with output_path, num_points, sample_rate, and tone_freq.
    """
    if output_path is None:
        output_path = os.path.join(
            tempfile.gettempdir(), f"genalyzer_tone_{num_points}_{int(tone_freq)}.npy"
        )

    t = np.arange(num_points) / sample_rate
    tone = amplitude * np.exp(2j * np.pi * tone_freq * t)
    np.save(output_path, tone)

    return {
        "output_path": output_path,
        "num_points": num_points,
        "sample_rate": sample_rate,
        "tone_freq": tone_freq,
    }


@mcp.tool()
def generate_real_tone(
    num_points: int,
    sample_rate: float,
    tone_freq: float,
    amplitude: float = 0.9,
    phase: float = 0.0,
    output_path: str | None = None,
) -> dict:
    """Generate a real-valued sinusoidal test tone and save to .npy.

    Use this for real-only ADC characterization. For complex (IQ) ADCs,
    prefer generate_test_tone.
    """
    if output_path is None:
        output_path = os.path.join(
            tempfile.gettempdir(),
            f"genalyzer_real_tone_{num_points}_{int(tone_freq)}.npy",
        )

    t = np.arange(num_points) / sample_rate
    tone = amplitude * np.cos(2.0 * np.pi * tone_freq * t + phase)
    np.save(output_path, tone)

    return {
        "output_path": output_path,
        "num_points": num_points,
        "sample_rate": sample_rate,
        "tone_freq": tone_freq,
    }


@mcp.tool()
def generate_ramp(
    num_points: int,
    start: float = -1.0,
    stop: float = 1.0,
    output_path: str | None = None,
) -> dict:
    """Generate a linear ramp from `start` to `stop` and save to .npy.

    Useful for DNL and INL characterization. The ramp covers the closed
    interval [start, stop].
    """
    if output_path is None:
        output_path = os.path.join(
            tempfile.gettempdir(), f"genalyzer_ramp_{num_points}.npy"
        )
    data = np.linspace(start, stop, num_points)
    np.save(output_path, data)
    return {
        "output_path": output_path,
        "num_points": num_points,
        "start": start,
        "stop": stop,
    }


@mcp.tool()
def generate_gaussian_noise(
    num_points: int,
    mean: float = 0.0,
    std: float = 0.1,
    seed: int | None = None,
    output_path: str | None = None,
) -> dict:
    """Generate AWGN samples and save to .npy.

    Useful for NSD (noise spectral density) characterization and
    histogram Gaussian-fit tests.
    """
    if output_path is None:
        output_path = os.path.join(
            tempfile.gettempdir(), f"genalyzer_gauss_{num_points}.npy"
        )
    rng = np.random.default_rng(seed)
    data = rng.normal(loc=mean, scale=std, size=num_points)
    np.save(output_path, data)
    return {
        "output_path": output_path,
        "num_points": num_points,
        "mean": mean,
        "std": std,
    }
