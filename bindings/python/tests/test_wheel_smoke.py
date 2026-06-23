# Copyright (C) 2024-2026 Analog Devices, Inc.
#
# SPDX short identifier: ADIBSD OR GPL-2.0-or-later
"""Smoke tests for self-contained binary wheels."""

from importlib import resources
from pathlib import Path

import genalyzer as gn
import genalyzer.pygenalyzer as pygn


def test_wheel_loads_bundled_native_library():
    package_dir = Path(str(resources.files("genalyzer"))).resolve()
    library_path = Path(pygn._libpath).resolve()

    assert library_path.is_file()
    assert library_path.is_relative_to(package_dir)
    assert library_path.name.startswith("libgenalyzer.so") or library_path.name in {
        "libgenalyzer.dylib",
        "libgenalyzer.dll",
        "genalyzer.dll",
    }

    assert isinstance(gn.mgr_size(), int)
