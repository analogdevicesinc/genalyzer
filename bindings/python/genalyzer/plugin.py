# Copyright (C) 2025 Analog Devices, Inc.
#
# SPDX short identifier: ADIBSD OR GPL-2.0-or-later
"""Pytest plugin for Genalyzer."""

import pytest
import logging

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger("genalyzer-pytest-plugin")


class GenalyzerPlotManager:
    def __init__(self):
        self.plots = []

    def add_plot(self, plot):
        self.plots.append(plot)

    def clear_plots(self):
        self.plots = []


def pytest_addoption(parser):
    """Add custom command line options to pytest."""
    parser.addoption(
        "--genalyzer-log-level",
        action="store",
        default="INFO",
        help="Set the logging level for Genalyzer (DEBUG, INFO, WARNING, ERROR, CRITICAL)",
    )

@pytest.fixture(autouse=True, scope="session")
def manager(request):
    """Fixture to manage Genalyzer plots."""
    log_level = request.config.getoption("--genalyzer-log-level").upper()
    logger.setLevel(getattr(logging, log_level, logging.INFO))
    plot_manager = GenalyzerPlotManager()
    yield plot_manager
    plot_manager.clear_plots()
    logger.info("Cleared all Genalyzer plots.")


@pytest.fixture()
def gn_plot_fft(manager):
    """Fixture to plot FFT from a signal."""

    yield manager