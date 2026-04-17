# Copyright (C) 2024-2026 Analog Devices, Inc.
#
# SPDX short identifier: ADIBSD OR GPL-2.0-or-later
"""Tests for the genalyzer CLI."""

import json
import sys

import pytest
from click.testing import CliRunner

pytestmark = [
    pytest.mark.genalyzer,
    pytest.mark.skipif(sys.version_info < (3, 10), reason="CLI wraps MCP tools which require Python 3.10+"),
]


@pytest.fixture
def runner():
    return CliRunner()


class TestStructure:
    def test_cli_importable(self):
        from genalyzer.cli.main import cli
        assert cli.name == "cli" or cli.name == "genalyzer"

    def test_version(self, runner):
        from genalyzer.cli.main import cli
        result = runner.invoke(cli, ["--version"])
        assert result.exit_code == 0
        assert "genalyzer" in result.output.lower() or any(c.isdigit() for c in result.output)

    def test_tools_lists_sixteen_names(self, runner):
        from genalyzer.cli.main import cli
        result = runner.invoke(cli, ["tools"])
        assert result.exit_code == 0
        names = [line.strip() for line in result.output.strip().splitlines() if line.strip()]
        assert len(names) == 16
        for expected in ("generate_test_tone", "analyze_spectrum", "analyze_waveform"):
            assert expected in names


class TestBuilder:
    def test_translate_int_required(self):
        import inspect
        from genalyzer.cli._builder import _translate

        def _fn(x: int):
            pass
        param = inspect.signature(_fn).parameters["x"]
        click_type, required, default, is_flag = _translate(param)
        import click as _click
        assert click_type is _click.INT
        assert required is True
        assert default is None
        assert is_flag is False

    def test_translate_float_with_default(self):
        import inspect
        from genalyzer.cli._builder import _translate

        def _fn(x: float = 1.5):
            pass
        param = inspect.signature(_fn).parameters["x"]
        click_type, required, default, is_flag = _translate(param)
        import click as _click
        assert click_type is _click.FLOAT
        assert required is False
        assert default == 1.5
        assert is_flag is False

    def test_translate_str(self):
        import inspect
        from genalyzer.cli._builder import _translate

        def _fn(x: str = "hello"):
            pass
        param = inspect.signature(_fn).parameters["x"]
        click_type, required, default, is_flag = _translate(param)
        import click as _click
        assert click_type is _click.STRING
        assert default == "hello"
        assert is_flag is False

    def test_translate_bool_becomes_flag(self):
        import inspect
        from genalyzer.cli._builder import _translate

        def _fn(plot: bool = False):
            pass
        param = inspect.signature(_fn).parameters["plot"]
        _click_type, _required, default, is_flag = _translate(param)
        assert is_flag is True
        assert default is False

    def test_translate_optional_int(self):
        import inspect
        from genalyzer.cli._builder import _translate

        def _fn(nfft: int | None = None):
            pass
        param = inspect.signature(_fn).parameters["nfft"]
        click_type, required, default, is_flag = _translate(param)
        import click as _click
        assert click_type is _click.INT
        assert required is False
        assert default is None
        assert is_flag is False

    def test_translate_unsupported_raises(self):
        import inspect
        from genalyzer.cli._builder import _translate

        def _fn(x: list[int]):
            pass
        param = inspect.signature(_fn).parameters["x"]
        with pytest.raises(ValueError, match="unsupported annotation"):
            _translate(param)

    def test_json_default_numpy_scalar(self):
        import numpy as np
        from genalyzer.cli._builder import _json_default

        assert _json_default(np.float64(1.5)) == 1.5
        assert _json_default(np.int32(7)) == 7

    def test_json_default_numpy_array(self):
        import numpy as np
        from genalyzer.cli._builder import _json_default

        assert _json_default(np.array([1.0, 2.0])) == [1.0, 2.0]

    def test_json_default_falls_back_to_str(self):
        from pathlib import Path
        from genalyzer.cli._builder import _json_default

        assert _json_default(Path("/tmp/x")) == "/tmp/x"

    def test_click_from_tool_roundtrip(self, runner):
        from genalyzer.cli._builder import click_from_tool

        def sample(a: int, b: float = 1.0, c: str | None = None, plot: bool = False) -> dict:
            return {"a": a, "b": b, "c": c, "plot": plot}

        cmd = click_from_tool(sample, "sample")
        result = runner.invoke(cmd, ["--a", "3", "--b", "2.5", "--c", "hi", "--plot", "--compact"])
        assert result.exit_code == 0, result.output
        data = json.loads(result.output.strip())
        assert data == {"a": 3, "b": 2.5, "c": "hi", "plot": True}

    def test_click_from_tool_missing_required_exits_2(self, runner):
        from genalyzer.cli._builder import click_from_tool

        def sample(a: int) -> dict:
            return {"a": a}

        cmd = click_from_tool(sample, "sample")
        result = runner.invoke(cmd, [])
        assert result.exit_code == 2

    def test_click_from_tool_tool_error_exits_0(self, runner):
        from genalyzer.cli._builder import click_from_tool

        def sample(a: int) -> dict:
            return {"error": "bad input"}

        cmd = click_from_tool(sample, "sample")
        result = runner.invoke(cmd, ["--a", "1", "--compact"])
        assert result.exit_code == 0
        assert json.loads(result.output.strip()) == {"error": "bad input"}

    def test_click_from_tool_exception_exits_1(self, runner):
        from genalyzer.cli._builder import click_from_tool

        def sample(a: int) -> dict:
            raise RuntimeError("kaboom")

        cmd = click_from_tool(sample, "sample")
        result = runner.invoke(cmd, ["--a", "1", "--compact"])
        assert result.exit_code == 1
        data = json.loads(result.output.strip())
        assert "error" in data
        assert "RuntimeError" in data["error"]
        assert "kaboom" in data["error"]

    def test_click_from_tool_resolves_pep563_string_annotations(self, runner):
        """Builder must resolve string annotations produced by `from __future__ import annotations`."""
        from genalyzer.cli._builder import click_from_tool

        exec_globals: dict = {}
        src = (
            "from __future__ import annotations\n"
            "def sample(a: int, b: float = 2.5, c: str | None = None) -> dict:\n"
            "    return {'a': a, 'b': b, 'c': c}\n"
        )
        exec(src, exec_globals)
        sample = exec_globals["sample"]

        cmd = click_from_tool(sample, "sample")
        result = runner.invoke(cmd, ["--a", "7", "--compact"])
        assert result.exit_code == 0, result.output
        data = json.loads(result.output.strip())
        assert data == {"a": 7, "b": 2.5, "c": None}


class TestRegistration:
    def test_sixteen_subcommands_registered(self, runner):
        """Every one of the 16 MCP tool names resolves to a click subcommand."""
        from genalyzer.cli.main import cli

        # Expected (group, subcommand-kebab, MCP tool name)
        expected = [
            ("generators", "test-tone", "generate_test_tone"),
            ("generators", "real-tone", "generate_real_tone"),
            ("generators", "ramp", "generate_ramp"),
            ("generators", "gaussian-noise", "generate_gaussian_noise"),
            (None, "quantize", "quantize"),
            ("fourier", "fft", "compute_fft"),
            ("fourier", "fa-metrics", "get_fa_metrics"),
            ("fourier", "analyze", "analyze_spectrum"),
            ("histogram", "compute", "compute_histogram"),
            ("histogram", "analyze", "analyze_histogram"),
            ("linearity", "compute-dnl", "compute_dnl"),
            ("linearity", "compute-inl", "compute_inl"),
            ("linearity", "analyze-dnl", "analyze_dnl"),
            ("linearity", "analyze-inl", "analyze_inl"),
            ("waveform", "stats", "compute_waveform_stats"),
            ("waveform", "analyze", "analyze_waveform"),
        ]

        for group, sub, _mcp_name in expected:
            cmd_path = [group, sub] if group else [sub]
            result = runner.invoke(cli, cmd_path + ["--help"])
            assert result.exit_code == 0, f"Missing command: genalyzer {' '.join(cmd_path)}\n{result.output}"


class TestSmokePerGroup:
    def test_generators_test_tone(self, runner, tmp_path):
        from genalyzer.cli.main import cli
        out = str(tmp_path / "tone.npy")
        result = runner.invoke(
            cli,
            [
                "generators", "test-tone",
                "--num-points", "1024",
                "--sample-rate", "250e6",
                "--tone-freq", "30e6",
                "--amplitude", "0.9",
                "--output-path", out,
                "--compact",
            ],
        )
        assert result.exit_code == 0, result.output
        data = json.loads(result.output.strip())
        assert data["output_path"] == out

    def test_quantize(self, runner, synthetic_ramp, tmp_path):
        import numpy as np
        from genalyzer.cli.main import cli

        # Convert int32 ramp to float (gn.quantize requires float input)
        ramp = np.load(synthetic_ramp["path"]).astype(np.float64)
        ramp /= max(abs(ramp.min()), abs(ramp.max()))
        float_path = str(tmp_path / "ramp_f.npy")
        np.save(float_path, ramp)

        out = str(tmp_path / "q.npy")
        result = runner.invoke(
            cli,
            [
                "quantize",
                "--npy-path", float_path,
                "--bits", "12",
                "--fullscale", "2.0",
                "--output-path", out,
                "--compact",
            ],
        )
        assert result.exit_code == 0, result.output
        data = json.loads(result.output.strip())
        assert "error" not in data
        assert data["output_path"] == out

    def test_fourier_analyze(self, runner, synthetic_tone):
        from genalyzer.cli.main import cli
        result = runner.invoke(
            cli,
            [
                "fourier", "analyze",
                "--npy-path", synthetic_tone["path"],
                "--sample-rate", str(synthetic_tone["sample_rate"]),
                "--window", "blackman_harris",
                "--ssb", "3",
                "--compact",
            ],
        )
        assert result.exit_code == 0, result.output
        data = json.loads(result.output.strip())
        assert "sfdr" in data and "snr" in data
        assert "plot_path" not in data

    def test_histogram_analyze(self, runner, synthetic_ramp):
        from genalyzer.cli.main import cli
        result = runner.invoke(
            cli,
            [
                "histogram", "analyze",
                "--npy-path", synthetic_ramp["path"],
                "--nbits", str(synthetic_ramp["nbits"]),
                "--compact",
            ],
        )
        assert result.exit_code == 0, result.output
        data = json.loads(result.output.strip())
        assert "sum" in data and "nz_range" in data

    def test_linearity_analyze_dnl(self, runner, synthetic_ramp):
        from genalyzer.cli.main import cli
        result = runner.invoke(
            cli,
            [
                "linearity", "analyze-dnl",
                "--npy-path", synthetic_ramp["path"],
                "--nbits", str(synthetic_ramp["nbits"]),
                "--signal-type", "ramp",
                "--compact",
            ],
        )
        assert result.exit_code == 0, result.output
        data = json.loads(result.output.strip())
        assert "dnl_max" in data

    def test_waveform_stats(self, runner, synthetic_tone):
        from genalyzer.cli.main import cli
        result = runner.invoke(
            cli,
            ["waveform", "stats", "--npy-path", synthetic_tone["path"], "--compact"],
        )
        assert result.exit_code == 0, result.output
        data = json.loads(result.output.strip())
        for key in ("min", "max", "avg", "rms"):
            assert key in data


class TestOutputModes:
    def test_default_is_pretty_printed(self, runner, synthetic_tone):
        from genalyzer.cli.main import cli
        result = runner.invoke(
            cli,
            ["waveform", "stats", "--npy-path", synthetic_tone["path"]],
        )
        assert result.exit_code == 0
        # Pretty JSON has newlines and 2-space indent
        assert "\n  " in result.output

    def test_compact_is_single_line(self, runner, synthetic_tone):
        from genalyzer.cli.main import cli
        result = runner.invoke(
            cli,
            ["waveform", "stats", "--npy-path", synthetic_tone["path"], "--compact"],
        )
        assert result.exit_code == 0
        body = result.output.strip()
        assert "\n" not in body
        json.loads(body)

    def test_tool_error_exits_zero_with_error_key(self, runner, tmp_path):
        from genalyzer.cli.main import cli
        missing = str(tmp_path / "does_not_exist.npy")
        result = runner.invoke(
            cli,
            [
                "fourier", "analyze",
                "--npy-path", missing,
                "--sample-rate", "250e6",
                "--compact",
            ],
        )
        # Tool returns {"error": ...} dict → exit 0
        assert result.exit_code == 0, result.output
        data = json.loads(result.output.strip())
        assert "error" in data
        assert "not found" in data["error"].lower()

    def test_click_arg_error_exits_two(self, runner):
        from genalyzer.cli.main import cli
        # Missing required --npy-path
        result = runner.invoke(cli, ["fourier", "analyze", "--sample-rate", "250e6"])
        assert result.exit_code == 2


class TestEndToEnd:
    def test_simulate_and_verify_pipeline(self, runner, tmp_path):
        """Mirror test_mcp_workflows.py::test_simulate_and_verify_12bit via CLI."""
        from genalyzer.cli.main import cli

        tone_out = str(tmp_path / "tone.npy")
        r1 = runner.invoke(
            cli,
            [
                "generators", "real-tone",
                "--num-points", "8192",
                "--sample-rate", "250e6",
                "--tone-freq", "30e6",
                "--amplitude", "0.9",
                "--output-path", tone_out,
                "--compact",
            ],
        )
        assert r1.exit_code == 0, r1.output
        tone_path = json.loads(r1.output.strip())["output_path"]

        q_out = str(tmp_path / "tone.q.npy")
        r2 = runner.invoke(
            cli,
            [
                "quantize",
                "--npy-path", tone_path,
                "--bits", "12",
                "--fullscale", "2.0",
                "--output-path", q_out,
                "--compact",
            ],
        )
        assert r2.exit_code == 0, r2.output
        q_path = json.loads(r2.output.strip())["output_path"]

        r3 = runner.invoke(
            cli,
            [
                "fourier", "analyze",
                "--npy-path", q_path,
                "--sample-rate", "250e6",
                "--window", "blackman_harris",
                "--ssb", "3",
                "--compact",
            ],
        )
        assert r3.exit_code == 0, r3.output
        analysis = json.loads(r3.output.strip())
        assert "error" not in analysis, analysis
        assert 11.5 < analysis["enob"] < 12.5, f"ENOB out of envelope: {analysis['enob']}"
