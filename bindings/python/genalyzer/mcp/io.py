# Copyright (C) 2024-2026 Analog Devices, Inc.
#
# SPDX short identifier: ADIBSD OR GPL-2.0-or-later
"""File IO layer for MCP tools. Handles .npy and .csv transparently."""

from __future__ import annotations

import csv
from pathlib import Path

import numpy as np


def _is_header(row: list[str]) -> bool:
    """Return True if the first cell of the row is not a float."""
    if not row:
        return False
    try:
        float(row[0])
        return False
    except ValueError:
        return True


def _load_csv(path: str) -> np.ndarray:
    with open(path, newline="") as f:
        reader = csv.reader(f)
        rows = [r for r in reader if r]  # drop blank lines

    if not rows:
        raise ValueError(f"CSV file is empty: {path}")

    if _is_header(rows[0]):
        rows = rows[1:]

    if not rows:
        raise ValueError(f"CSV has header but no data rows: {path}")

    ncols = len(rows[0])
    if ncols not in (1, 2):
        raise ValueError(
            f"CSV must have 1 or 2 columns (got {ncols}): {path}"
        )

    for i, r in enumerate(rows):
        if len(r) != ncols:
            raise ValueError(
                f"CSV row {i} has {len(r)} columns, expected {ncols}: {path}"
            )

    data = np.array([[float(x) for x in r] for r in rows])
    if ncols == 1:
        return data[:, 0]
    return data[:, 0] + 1j * data[:, 1]


def load_array(path: str) -> np.ndarray:
    """Load a 1-D numpy array from a .npy or .csv file.

    Returns a real-valued array for single-column CSV and .npy real arrays,
    or a complex-valued array for two-column CSV (I,Q) and .npy complex
    arrays. CSV header detection: if the first cell of row 0 is not a float,
    row 0 is treated as a header and skipped.
    """
    ext = Path(path).suffix.lower()
    if ext == ".npy":
        return np.load(path)
    if ext == ".csv":
        return _load_csv(path)
    raise ValueError(f"unsupported extension: {ext} (expected .npy or .csv)")


def save_array(path: str, arr: np.ndarray) -> None:
    """Save a 1-D numpy array to .npy or .csv, chosen by extension."""
    ext = Path(path).suffix.lower()
    if ext == ".npy":
        np.save(path, arr)
        return
    if ext == ".csv":
        arr = np.asarray(arr)
        if np.iscomplexobj(arr):
            data = np.column_stack([arr.real, arr.imag])
        else:
            data = arr.reshape(-1, 1)
        with open(path, "w", newline="") as f:
            writer = csv.writer(f)
            for row in data:
                writer.writerow([f"{v:.17g}" for v in row])
        return
    raise ValueError(f"unsupported extension: {ext} (expected .npy or .csv)")
