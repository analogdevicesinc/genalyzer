# Copyright (C) 2024-2026 Analog Devices, Inc.
#
# SPDX short identifier: ADIBSD OR GPL-2.0-or-later
"""Smoke tests for self-contained binary wheels."""

from importlib import metadata, resources
from pathlib import Path

import numpy as np

import genalyzer as gn
import genalyzer.pygenalyzer as pygn


def test_wheel_metadata_selects_gpl_license():
    distribution = metadata.distribution("genalyzer")

    assert distribution.metadata["License-Expression"] == "GPL-2.0-or-later"
    license_files = {
        str(path).replace("\\", "/")
        for path in (distribution.files or ())
        if ".dist-info/licenses/" in str(path).replace("\\", "/")
    }
    assert any(path.endswith("/LICENSE") for path in license_files)
    assert any(path.endswith("/THIRD_PARTY_LICENSES.md") for path in license_files)


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


def test_wheel_bundles_fftw_and_executes_fft():
    distribution = metadata.distribution("genalyzer")
    installed_files = [str(path).lower() for path in (distribution.files or ())]

    assert any(
        "fftw3" in path and (".so" in path or path.endswith((".dll", ".dylib")))
        for path in installed_files
    ), "repaired wheel does not contain an FFTW runtime library"

    samples = np.ones(8, dtype=np.complex128)
    result = gn.fft(samples, 1, 8, gn.Window.NO_WINDOW)
    assert result.size == samples.size
