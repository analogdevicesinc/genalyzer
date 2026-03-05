# Copyright (C) 2024-2025 Analog Devices, Inc.
#
# SPDX short identifier: ADIBSD OR GPL-2.0-or-later
"""pytest configuration for the genalyzer Python binding tests.

This conftest adds ``bindings/python/`` to ``sys.path`` and pre-registers a
lightweight ``genalyzer`` stub in ``sys.modules``.  That prevents
``genalyzer/__init__.py`` from being executed (and failing because the compiled
C library is not present) whenever a test imports only a sub-module such as
``genalyzer.fourier``.

Tests that actually exercise the C-level bindings (e.g. ``test_genalyzer.py``)
should call ``genalyzer_impl_init()`` to swap the stub for the real package.
"""
from __future__ import annotations

import importlib
import importlib.util
import pathlib
import sys
import types

# ── path setup ────────────────────────────────────────────────────────────────
_HERE = pathlib.Path(__file__).parent        # tests/
_PKG_ROOT = _HERE.parent                     # bindings/python/
_GENALYZER_DIR = _PKG_ROOT / "genalyzer"

if str(_PKG_ROOT) not in sys.path:
    sys.path.insert(0, str(_PKG_ROOT))

# ── genalyzer package stub ────────────────────────────────────────────────────
# If the real genalyzer package is already importable (C library present) leave
# it alone.  Otherwise inject a minimal namespace package so that sub-modules
# (like genalyzer.fourier) can be imported without triggering the C-library
# loader in __init__.py.
if "genalyzer" not in sys.modules:
    try:
        # Try the real thing first — succeeds when libgenalyzer.so is installed.
        import genalyzer  # noqa: F401
    except (ImportError, OSError):
        stub = types.ModuleType("genalyzer")
        stub.__path__ = [str(_GENALYZER_DIR)]   # type: ignore[assignment]
        stub.__package__ = "genalyzer"
        stub.__spec__ = importlib.util.spec_from_file_location(
            "genalyzer",
            _GENALYZER_DIR / "__init__.py",
            submodule_search_locations=[str(_GENALYZER_DIR)],
        )
        sys.modules["genalyzer"] = stub
