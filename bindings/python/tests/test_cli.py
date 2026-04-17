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
