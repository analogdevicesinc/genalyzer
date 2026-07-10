# Copyright (C) 2024-2026 Analog Devices, Inc.
#
# SPDX short identifier: ADIBSD OR GPL-2.0-or-later
"""Helpers for locating and loading the native Genalyzer library."""

from __future__ import annotations

import ctypes
import os
import sys
from ctypes.util import find_library
from importlib import resources
from pathlib import Path

_RUNTIME_PACKAGE_NAME = "genalyzer_fftw"
_DLL_DIRECTORY_HANDLES: list[object] = []


def _platform_library_names() -> tuple[str, ...]:
    if sys.platform.startswith("linux"):
        return ("libgenalyzer.so",)
    if sys.platform == "darwin":
        return ("libgenalyzer.dylib",)
    if sys.platform == "win32":
        return ("libgenalyzer.dll", "genalyzer.dll")
    raise OSError(f"Platform '{sys.platform}' is not supported.")


def bundled_library_path() -> str | None:
    """Return the bundled native library path, if present in the package."""

    try:
        package_dir = Path(str(resources.files(_RUNTIME_PACKAGE_NAME)))
    except ModuleNotFoundError:
        return None
    for library_name in _platform_library_names():
        library_path = package_dir / library_name
        if library_path.is_file():
            return str(library_path)
    return None


def _system_library_path() -> str | None:
    names = ["genalyzer"]
    if sys.platform == "win32":
        names.insert(0, "libgenalyzer.dll")

    for library_name in names:
        library_path = find_library(library_name)
        if library_path is not None:
            return library_path
    return None


def find_genalyzer_library() -> str:
    """Find the native Genalyzer library, preferring a bundled wheel copy."""

    library_path = bundled_library_path() or _system_library_path()
    if library_path is None:
        searched = ", ".join(_platform_library_names())
        raise OSError(
            2,
            "Could not find genalyzer C library. "
            f"Searched for bundled {searched} in the genalyzer-fftw runtime and "
            "then the system library path.",
        )
    return library_path


def load_genalyzer_library(**kwargs):
    """Load the native Genalyzer library and return ``(handle, path)``."""

    library_path = find_genalyzer_library()
    if sys.platform == "win32" and os.path.isabs(library_path):
        # Keep the handle alive so transitive DLL dependencies can be resolved
        # from the same directory as the bundled libgenalyzer.dll.
        add_dll_directory = getattr(os, "add_dll_directory", None)
        if add_dll_directory is not None:
            _DLL_DIRECTORY_HANDLES.append(
                add_dll_directory(str(Path(library_path).parent))
            )
    return ctypes.CDLL(library_path, **kwargs), library_path
