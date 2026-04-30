# Copyright (C) 2024-2026 Analog Devices, Inc.
#
# SPDX short identifier: ADIBSD OR GPL-2.0-or-later
"""Tests for the genalyzer.mcp.io file-loading layer."""

import sys
import pytest
import numpy as np

pytestmark = [
    pytest.mark.genalyzer,
    pytest.mark.skipif(sys.version_info < (3, 10), reason="fastmcp requires Python 3.10+"),
]


def test_roundtrip_npy_real(tmp_path):
    from genalyzer.mcp.io import load_array, save_array
    arr = np.linspace(-1.0, 1.0, 128)
    path = str(tmp_path / "x.npy")
    save_array(path, arr)
    out = load_array(path)
    np.testing.assert_allclose(out, arr)


def test_roundtrip_npy_complex(tmp_path):
    from genalyzer.mcp.io import load_array, save_array
    arr = np.exp(1j * np.linspace(0, 2 * np.pi, 64))
    path = str(tmp_path / "x.npy")
    save_array(path, arr)
    out = load_array(path)
    np.testing.assert_allclose(out, arr)


def test_roundtrip_csv_real_no_header(tmp_path):
    from genalyzer.mcp.io import load_array, save_array
    arr = np.linspace(-1.0, 1.0, 16)
    path = str(tmp_path / "x.csv")
    save_array(path, arr)
    out = load_array(path)
    np.testing.assert_allclose(out, arr)


def test_csv_real_with_header(tmp_path):
    from genalyzer.mcp.io import load_array
    path = tmp_path / "h.csv"
    path.write_text("sample\n1.0\n2.0\n3.0\n")
    out = load_array(str(path))
    np.testing.assert_allclose(out, np.array([1.0, 2.0, 3.0]))


def test_csv_iq_no_header(tmp_path):
    from genalyzer.mcp.io import load_array
    path = tmp_path / "iq.csv"
    path.write_text("1.0,0.5\n-0.5,1.0\n")
    out = load_array(str(path))
    assert np.iscomplexobj(out)
    np.testing.assert_allclose(out, np.array([1.0 + 0.5j, -0.5 + 1.0j]))


def test_csv_iq_with_header(tmp_path):
    from genalyzer.mcp.io import load_array
    path = tmp_path / "iq.csv"
    path.write_text("I,Q\n1.0,0.5\n-0.5,1.0\n")
    out = load_array(str(path))
    assert np.iscomplexobj(out)
    np.testing.assert_allclose(out, np.array([1.0 + 0.5j, -0.5 + 1.0j]))


def test_csv_mismatched_columns_raises(tmp_path):
    from genalyzer.mcp.io import load_array
    path = tmp_path / "bad.csv"
    path.write_text("1,2,3\n4,5,6\n")
    with pytest.raises(ValueError, match="1 or 2 columns"):
        load_array(str(path))


def test_unsupported_extension_raises(tmp_path):
    from genalyzer.mcp.io import load_array
    path = tmp_path / "x.txt"
    path.write_text("hello")
    with pytest.raises(ValueError, match="unsupported extension"):
        load_array(str(path))


def test_csv_header_only_raises(tmp_path):
    from genalyzer.mcp.io import load_array
    path = tmp_path / "headeronly.csv"
    path.write_text("sample\n")
    with pytest.raises(ValueError, match="header but no data"):
        load_array(str(path))
