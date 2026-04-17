# Copyright (C) 2024-2026 Analog Devices, Inc.
#
# SPDX short identifier: ADIBSD OR GPL-2.0-or-later
"""FastMCP server instance and entry point for genalyzer MCP tools."""

from __future__ import annotations

from fastmcp import FastMCP

# Try to import genalyzer native bindings; they require libgenalyzer to be installed.
try:
    import genalyzer as gn

    _HAS_GENALYZER = True
    _gn_import_error = ""
except (ImportError, OSError) as _import_err:
    _HAS_GENALYZER = False
    _gn_import_error = str(_import_err)

mcp = FastMCP("genalyzer")


def _require_genalyzer():
    """Raise a clear error if the native library is unavailable."""
    if not _HAS_GENALYZER:
        raise RuntimeError(
            f"genalyzer native library (libgenalyzer) is not available: "
            f"{_gn_import_error}. Install libgenalyzer and reinstall the "
            "Python package."
        )


def main():
    """Run the genalyzer MCP server."""
    # Import domain modules so their @mcp.tool() decorators register.
    from . import (
        fourier,
        generators,
        histogram,
        linearity,
        quantize,
        waveform,
    )

    mcp.run()


if __name__ == "__main__":
    main()
