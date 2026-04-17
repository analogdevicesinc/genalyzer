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
