# Copyright (C) 2024-2026 Analog Devices, Inc.
#
# SPDX short identifier: ADIBSD OR GPL-2.0-or-later
"""Setuptools entry point for platform-specific binary wheels."""

from setuptools import setup
from setuptools.command.bdist_wheel import bdist_wheel


class BinaryWheel(bdist_wheel):
    """Mark wheels as platform-specific because they contain libgenalyzer."""

    def finalize_options(self):
        super().finalize_options()
        self.root_is_pure = False


setup(cmdclass={"bdist_wheel": BinaryWheel})
