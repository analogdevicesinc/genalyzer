# Copyright (C) 2025 Analog Devices, Inc.
#
# SPDX short identifier: ADIBSD OR GPL-2.0-or-later
from importlib.metadata import version

try:
    __version__ = version("pytest-genalyzer")
except Exception: 
    __version__ = "missing"

__pypi_url__ = "https://pypi.python.org/pypi/genalyzer"
