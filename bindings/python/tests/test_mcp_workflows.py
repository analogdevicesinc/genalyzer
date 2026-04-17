# Copyright (C) 2024-2026 Analog Devices, Inc.
#
# SPDX short identifier: ADIBSD OR GPL-2.0-or-later
"""End-to-end tests mirroring workflow code blocks in docs/mcp/workflows.md."""

import sys
from pathlib import Path

import numpy as np
import pytest

pytestmark = [
    pytest.mark.genalyzer,
    pytest.mark.skipif(sys.version_info < (3, 10), reason="fastmcp requires Python 3.10+"),
]


def test_simulate_and_verify_12bit(tmp_path):
    """generate_real_tone -> quantize(12-bit) -> analyze_spectrum."""
    from genalyzer.mcp.fourier import analyze_spectrum
    from genalyzer.mcp.generators import generate_real_tone
    from genalyzer.mcp.quantize import quantize

    tone_path = str(tmp_path / "tone.npy")
    generate_real_tone(
        num_points=8192,
        sample_rate=250e6,
        tone_freq=30e6,
        amplitude=0.9,
        output_path=tone_path,
    )

    q_path = str(tmp_path / "tone.q.npy")
    # fullscale=2.0 matches the [-1, 1] analog range; gn.quantize uses step=fs/2^n
    q = quantize(npy_path=tone_path, bits=12, fullscale=2.0, output_path=q_path)
    assert "error" not in q, q

    result = analyze_spectrum(
        npy_path=q_path,
        sample_rate=250e6,
        window="blackman_harris",
        ssb=3,
    )
    assert "error" not in result, result
    assert 10.5 < result["enob"] < 13.5, f"ENOB out of envelope: {result['enob']}"


def test_hardware_capture_shape(tmp_path):
    """Synthetic complex tone + small DC offset -> analyze_spectrum."""
    from genalyzer.mcp.fourier import analyze_spectrum

    sample_rate = 250e6
    tone_freq = 30e6
    num_points = 8192
    t = np.arange(num_points) / sample_rate
    tone = 0.9 * np.exp(2j * np.pi * tone_freq * t) + 0.01
    capture_path = str(tmp_path / "capture.npy")
    np.save(capture_path, tone)

    result = analyze_spectrum(
        npy_path=capture_path,
        sample_rate=sample_rate,
        window="blackman_harris",
        ssb=12,
    )
    assert "error" not in result, result
    assert result["sfdr"] > 40.0
    # "A:freq" is the detected fundamental frequency; "fbin" is the bin width in Hz
    assert abs(result["results"].get("A:freq", 0) - tone_freq) < sample_rate / num_points


def test_automation_shape(tmp_path):
    """Pipeline pattern: analyze_spectrum(plot=False) then plot=True."""
    from genalyzer.mcp.fourier import analyze_spectrum
    from genalyzer.mcp.generators import generate_test_tone

    tone_path = str(tmp_path / "tone.npy")
    generate_test_tone(
        num_points=8192,
        sample_rate=250e6,
        tone_freq=30e6,
        amplitude=0.9,
        output_path=tone_path,
    )

    res_no_plot = analyze_spectrum(npy_path=tone_path, sample_rate=250e6)
    assert "plot_path" not in res_no_plot

    res_plot = analyze_spectrum(npy_path=tone_path, sample_rate=250e6, plot=True)
    assert "plot_path" in res_plot
    assert Path(res_plot["plot_path"]).exists()
    assert res_plot["plot_path"].endswith(".png")
