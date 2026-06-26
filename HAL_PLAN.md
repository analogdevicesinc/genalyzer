# Platform Wheels Bundling libgenalyzer (cibuildwheel) Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Ship `pip install genalyzer` wheels for Linux/macOS/Windows that bundle the compiled `libgenalyzer` shared library (and its FFTW3 dependency) so importing `genalyzer` works out of the box with no system-installed C library.

**Architecture:** `genalyzer` is a pure-Python package that loads `libgenalyzer.{so,dylib,dll}` via `ctypes` at import time. Because it uses `ctypes` (not a CPython C-extension), the shared library is **Python-version independent**, so a single `py3-none-<platform>` wheel serves all Python 3 versions on a given OS/arch. We use **cibuildwheel** invoked from the repo root pointing at the `bindings/python` package: its `before-all` step builds `libgenalyzer` with the existing CMake project (which lives at the repo root) and copies the resulting shared library into the package directory; setuptools then packages it; and cibuildwheel's repair step (auditwheel/delocate/delvewheel) vendors the external FFTW3 dependency into the wheel and patches the loader paths. The Python loader is rewritten to load the bundled library by absolute path, falling back to `find_library` for source/system installs.

**Tech Stack:** Python 3.9–3.13, setuptools (build backend, unchanged), CMake (existing), FFTW3 (external C dependency), cibuildwheel, auditwheel/delocate/delvewheel, GitHub Actions.

## Global Constraints

- **Package location:** `bindings/python/` (pyproject.toml lives there; the C/C++ CMake project lives at the repo root). Do **not** relocate `pyproject.toml` — the existing lint/type/test CI (`.github/workflows/test.yml`, `lint.yml`) `cd bindings/python` and depend on it.
- **Build backend stays setuptools.** Do not switch to scikit-build-core (evaluated and rejected below — it would require moving `pyproject.toml` to the repo root and rewriting the test/lint CI).
- **Shared library filename (plain, unversioned), placed directly inside the `genalyzer/` package directory:** Linux `libgenalyzer.so`, macOS `libgenalyzer.dylib`, Windows `libgenalyzer.dll`.
- **External runtime dependency:** FFTW3 (`libfftw3-3` / `libfftw3.so.3` / `libfftw3.dylib` / `libfftw3-3.dll`). It is a real link-time dependency of `libgenalyzer`, so the repair tools vendor it automatically once `libgenalyzer` is inside the wheel.
- **One wheel per platform:** emit `py3-none-<platform>` (not `cpXX-cpXX-<platform>`). The library does not touch the CPython ABI.
- **Supported interpreters for testing:** CPython 3.9, 3.10, 3.11, 3.12, 3.13.
- **manylinux base is AlmaLinux/CentOS** (default `manylinux2014`); FFTW dev headers come from EPEL via `yum`/`dnf`.
- **Version is currently `0.1.4`, declared in three places that must stay in sync:** `CMakeLists.txt:4` (`VERSION 0.1.4`), `bindings/python/pyproject.toml:8` (`version = "0.1.4"`), `bindings/python/genalyzer/__init__.py:6` (`__version__ = "0.1.4"`). Do not change the version in this plan.
- **Portable builds:** the wheel's `libgenalyzer` must NOT be compiled with `-march=native` (CMakeLists.txt:28) or it will crash with `SIGILL` on end-user CPUs that differ from the CI builder.
- **Loader must not regress source installs:** if no bundled library is present, fall back to `ctypes.util.find_library` so a system-wide `libgenalyzer` still works (current behavior).

---

## Architecture Decisions (read before starting)

**Why setuptools + cibuildwheel `before-all`, not scikit-build-core?**
scikit-build-core is the "purpose-built" tool for CMake-backed wheels, but it resolves all paths relative to `pyproject.toml`'s directory and does not officially support a `cmake.source-dir` *above* the project root. Using it here would force moving `pyproject.toml` to the repo root and rewriting every CI job that does `cd bindings/python`. That is a large, risky refactor for this repo. The setuptools + `before-all` approach keeps the existing layout and CI intact; its only real downside (a bare `pip install .` from an sdist outside CI will not compile the library) is acceptable because the loader keeps the `find_library` fallback for source/system installs, preserving today's behavior exactly.

**How cibuildwheel sees the root CMake sources.** cibuildwheel copies the **entire directory it is invoked from** (the "project root") into the manylinux container, and the positional argument selects which sub-package to build. We invoke `cibuildwheel bindings/python` **from the repo root**, so:
- `{project}` placeholder = repo root (has `CMakeLists.txt`, `src/`, `include/`, `bindings/c/`, `cmake/`).
- `{package}` placeholder = `bindings/python`.
- `before-all` runs **once** per platform (inside the container on Linux; on the host for macOS/Windows) with the full repo present, so it can run the root CMake build.

**Single wheel for all Python versions.** A `setup.py` `get_tag` override emits `py3-none-<platform>`. cibuildwheel detects the `py3-none` tag and reuses the one wheel across all selected interpreters, still running `test-command` on each.

**FFTW vendoring is automatic.** `libgenalyzer` records FFTW in its dynamic dependency table (`DT_NEEDED` / Mach-O load command / PE import table). Once `libgenalyzer` is inside the wheel, auditwheel/delocate/delvewheel follow that table, copy `libfftw3` into a private dir (`genalyzer.libs/`, `genalyzer/.dylibs/`, `genalyzer.libs\`), and rewrite RPATH/install-name/DLL-search so `libgenalyzer` finds it at runtime. The only repo-specific requirement is that **our** `libgenalyzer` be present in the wheel before repair runs (handled by `before-all` + setuptools package-data) and that on Windows `delvewheel` be told to treat the ctypes-loaded DLL as a scan root (`--analyze-existing`).

---

## File Structure

**Created:**
- `bindings/python/genalyzer/_libload.py` — single source of truth for locating + loading `libgenalyzer`. Imported by both existing loaders.
- `bindings/python/setup.py` — minimal shim forcing a platform-specific `py3-none-<plat>` wheel (metadata stays in `pyproject.toml`).
- `bindings/python/tests/test_libload.py` — unit tests for the loader resolution logic.
- `.github/scripts/cibw/before_all_linux.sh` — install FFTW + build libgenalyzer + stage `.so` (runs in manylinux container).
- `.github/scripts/cibw/before_all_macos.sh` — install FFTW + build libgenalyzer + stage `.dylib` (runs on macOS host).
- `.github/scripts/cibw/before_all_windows.sh` — build libgenalyzer + stage `.dll` (runs on Windows host; FFTW prepped by a prior CI step).
- `.github/scripts/cibw/setup_fftw_windows.ps1` — download FFTW DLLs and generate MSVC import libraries.
- `.github/workflows/wheels.yml` — cibuildwheel matrix + sdist + publish to TestPyPI (main) / PyPI (tags).

**Modified:**
- `CMakeLists.txt` — make `-march=native` opt-in so wheel builds are portable.
- `bindings/python/genalyzer/pygenalyzer.py:41-53` — use `_libload`.
- `bindings/python/genalyzer/simplified_beta/simplified_beta.py:28-38` — use `_libload`.
- `bindings/python/pyproject.toml` — bump `setuptools` floor, declare the shared-library package-data, add `[tool.cibuildwheel]` config.

**Deleted:**
- `.github/workflows/deploy-pypi.yml` and `.github/workflows/deploy-pypi-testing.yml` — superseded by `wheels.yml` (which builds + uploads platform wheels instead of the current broken pure-Python wheel).

---

## Task 1: Make the native library build portable (remove `-march=native`)

A wheel built with `-march=native` runs only on CPUs matching the CI builder; on other machines it dies with an illegal-instruction fault. Make the flag opt-in (default ON preserves today's local-build behavior; wheel CI passes `-DGENALYZER_NATIVE_ARCH=OFF`).

**Files:**
- Modify: `CMakeLists.txt:24-36`

**Interfaces:**
- Produces: a CMake cache option `GENALYZER_NATIVE_ARCH` (BOOL, default `ON`). When `OFF`, `-march=native` is omitted from `CMAKE_CXX_FLAGS_RELEASE`.

- [ ] **Step 1: Add the option and guard the flag**

In `CMakeLists.txt`, replace the Linux release-flags block. The current code (lines 24-28) reads:

```cmake
  if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(CMAKE_CXX_FLAGS
        "${CMAKE_CXX_FLAGS} -std=gnu++17 -fext-numeric-literals")
    set(CMAKE_CXX_FLAGS_RELEASE
        "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native -DNDEBUG")
```

Replace with:

```cmake
  if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(CMAKE_CXX_FLAGS
        "${CMAKE_CXX_FLAGS} -std=gnu++17 -fext-numeric-literals")
    option(GENALYZER_NATIVE_ARCH
           "Optimize for the build machine's CPU (-march=native). Disable for portable/distributable binaries such as Python wheels."
           ON)
    if(GENALYZER_NATIVE_ARCH)
      set(CMAKE_CXX_FLAGS_RELEASE
          "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native -DNDEBUG")
    else()
      set(CMAKE_CXX_FLAGS_RELEASE
          "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
    endif()
```

Leave the rest of the block (the GCC-version `else()`/`endif()` at lines 29-36) unchanged.

- [ ] **Step 2: Verify the option toggles the flag (portable configure)**

Run from the repo root:

```bash
rm -rf /tmp/gz-portable && cmake -S . -B /tmp/gz-portable -DCMAKE_BUILD_TYPE=Release -DGENALYZER_NATIVE_ARCH=OFF > /tmp/gz-portable-cfg.log 2>&1; echo "configure exit: $?"
grep -R "march=native" /tmp/gz-portable/CMakeCache.txt /tmp/gz-portable/CMakeFiles 2>/dev/null && echo "FOUND march=native (BAD)" || echo "no march=native (GOOD)"
```

Expected: `configure exit: 0` and `no march=native (GOOD)`.

- [ ] **Step 3: Verify default still uses native (no regression)**

```bash
rm -rf /tmp/gz-native && cmake -S . -B /tmp/gz-native -DCMAKE_BUILD_TYPE=Release > /tmp/gz-native-cfg.log 2>&1; echo "configure exit: $?"
grep -Rq "march=native" /tmp/gz-native/CMakeFiles && echo "native present (GOOD default)" || echo "native MISSING (BAD)"
```

Expected: `configure exit: 0` and `native present (GOOD default)`. (If FFTW is not installed locally the configure may fail at `find_package(FFTW)`; that is unrelated to this task — install `libfftw3-dev` or run only Step 2's grep on the generated flags file. The grep on `CMakeFiles` is the assertion that matters.)

- [ ] **Step 4: Commit**

```bash
git add CMakeLists.txt
git commit -m "build: make -march=native opt-in via GENALYZER_NATIVE_ARCH for portable wheels"
```

---

## Task 2: Centralize and fix the shared-library loader

Both existing loaders call `ctypes.util.find_library("genalyzer")`, which searches only **system** paths — it will never find a library bundled inside the package. Also, `pygenalyzer.py` raises on macOS (`darwin` is unhandled). Create one tested helper that looks inside the package directory first, then falls back to `find_library`, and supports Linux/macOS/Windows. Rewire both loaders to use it.

**Files:**
- Create: `bindings/python/genalyzer/_libload.py`
- Create: `bindings/python/tests/test_libload.py`
- Modify: `bindings/python/genalyzer/pygenalyzer.py:41-53`
- Modify: `bindings/python/genalyzer/simplified_beta/simplified_beta.py:28-38`

**Interfaces:**
- Produces:
  - `genalyzer._libload.find_genalyzer_library(package_dir=..., bundled_names=..., system_name=..., system_finder=...) -> str | None` — returns an absolute path to a bundled library if one exists in `package_dir`, else the result of `system_finder(system_name)` (default `ctypes.util.find_library`), else `None`. All arguments are keyword-overridable for testing.
  - `genalyzer._libload.load_genalyzer_library(**cdll_kwargs) -> ctypes.CDLL` — resolves the path via `find_genalyzer_library()` and returns `ctypes.CDLL(path, **cdll_kwargs)`; raises `OSError(2, msg)` if no library is found.
  - Module constants `BUNDLED_NAMES` (tuple) and `SYSTEM_NAME` (str), selected per `sys.platform`.

- [ ] **Step 1: Write the failing tests**

Create `bindings/python/tests/test_libload.py`:

```python
# Copyright (C) 2024-2026 Analog Devices, Inc.
#
# SPDX short identifier: ADIBSD OR GPL-2.0-or-later
"""Tests for the libgenalyzer loader resolution logic."""

import os

import pytest

from genalyzer import _libload


def test_finds_bundled_library_first(tmp_path):
    """A library file inside the package dir wins over the system finder."""
    bundled = tmp_path / "libgenalyzer.so"
    bundled.write_bytes(b"\x7fELF")

    def fake_system_finder(name):
        raise AssertionError("system finder must not be called when bundled exists")

    result = _libload.find_genalyzer_library(
        package_dir=str(tmp_path),
        bundled_names=("libgenalyzer.so",),
        system_name="genalyzer",
        system_finder=fake_system_finder,
    )
    assert result == str(bundled)


def test_falls_back_to_system_finder(tmp_path):
    """With no bundled file, the system finder result is returned."""
    result = _libload.find_genalyzer_library(
        package_dir=str(tmp_path),
        bundled_names=("libgenalyzer.so",),
        system_name="genalyzer",
        system_finder=lambda name: "/usr/lib/libgenalyzer.so",
    )
    assert result == "/usr/lib/libgenalyzer.so"


def test_returns_none_when_nothing_found(tmp_path):
    """No bundled file and a system finder that finds nothing returns None."""
    result = _libload.find_genalyzer_library(
        package_dir=str(tmp_path),
        bundled_names=("libgenalyzer.so",),
        system_name="genalyzer",
        system_finder=lambda name: None,
    )
    assert result is None


def test_load_raises_when_missing(tmp_path, monkeypatch):
    """load_genalyzer_library raises OSError(2) when no library resolves."""
    monkeypatch.setattr(
        _libload, "find_genalyzer_library", lambda **kwargs: None
    )
    with pytest.raises(OSError) as excinfo:
        _libload.load_genalyzer_library()
    assert excinfo.value.errno == 2


def test_platform_constants_are_defined():
    """The module selects a bundled-name tuple and a system name for this OS."""
    assert isinstance(_libload.BUNDLED_NAMES, tuple)
    assert len(_libload.BUNDLED_NAMES) >= 1
    assert isinstance(_libload.SYSTEM_NAME, str)
```

- [ ] **Step 2: Run the tests to verify they fail**

```bash
cd bindings/python
python -m pytest tests/test_libload.py -v
```

Expected: FAIL / collection error — `ModuleNotFoundError: No module named 'genalyzer._libload'` (the module does not exist yet).

- [ ] **Step 3: Create the loader module**

Create `bindings/python/genalyzer/_libload.py`:

```python
# Copyright (C) 2024-2026 Analog Devices, Inc.
#
# SPDX short identifier: ADIBSD OR GPL-2.0-or-later
"""Locate and load the libgenalyzer shared library.

In a binary wheel the shared library is bundled inside this package directory
(placed there by cibuildwheel's ``before-all`` step and patched by the wheel
repair tools). For source or system-wide installs no bundled library exists, so
we fall back to ``ctypes.util.find_library`` exactly as the package did before
wheels existed.
"""

import ctypes as _ctypes
import os as _os
import sys as _sys
from ctypes.util import find_library as _find_library

# Directory containing this file == the installed ``genalyzer`` package dir.
PACKAGE_DIR = _os.path.dirname(_os.path.abspath(__file__))

if _sys.platform.startswith("linux"):
    BUNDLED_NAMES = ("libgenalyzer.so",)
    SYSTEM_NAME = "genalyzer"
elif _sys.platform == "darwin":
    BUNDLED_NAMES = ("libgenalyzer.dylib",)
    SYSTEM_NAME = "genalyzer"
elif _sys.platform == "win32":
    BUNDLED_NAMES = ("libgenalyzer.dll",)
    SYSTEM_NAME = "libgenalyzer.dll"
else:
    raise OSError(f"Platform '{_sys.platform}' is not supported by genalyzer")


def find_genalyzer_library(
    package_dir=PACKAGE_DIR,
    bundled_names=BUNDLED_NAMES,
    system_name=SYSTEM_NAME,
    system_finder=_find_library,
):
    """Return an absolute path to the libgenalyzer shared library, or ``None``.

    A library bundled inside ``package_dir`` takes priority over a system-wide
    one. Arguments are keyword-overridable to make the logic unit-testable.
    """
    for name in bundled_names:
        candidate = _os.path.join(package_dir, name)
        if _os.path.exists(candidate):
            return candidate
    return system_finder(system_name)


def load_genalyzer_library(**cdll_kwargs):
    """Load and return the libgenalyzer ``ctypes.CDLL``.

    Raises ``OSError(2, ...)`` if the library cannot be located.
    """
    path = find_genalyzer_library()
    if path is None:
        raise OSError(
            2,
            "Could not find the genalyzer C library (libgenalyzer). "
            "Install a binary wheel with `pip install genalyzer`, or build and "
            "install libgenalyzer system-wide from source.",
        )
    return _ctypes.CDLL(path, **cdll_kwargs)
```

- [ ] **Step 4: Run the tests to verify they pass**

```bash
cd bindings/python
python -m pytest tests/test_libload.py -v
```

Expected: PASS (5 passed).

- [ ] **Step 5: Rewire `pygenalyzer.py` to use the loader**

In `bindings/python/genalyzer/pygenalyzer.py`, replace lines 41-53. The current code reads:

```python
_module_dir = _os.path.dirname(__file__)
if "linux" == _sys.platform:
    _libpath = _find_library("genalyzer")
elif "win32" == _sys.platform:
    _libpath = _find_library("libgenalyzer.dll")
else:
    raise Exception(f"Platform '{_sys.platform}' is not supported.")

if _libpath is None:
    raise OSError(2, "Could not find genalyzer C library")
_lib = _ctypes.cdll.LoadLibrary(_libpath)

del _find_library, _os, _sys
```

Replace with:

```python
from ._libload import load_genalyzer_library as _load_genalyzer_library

_lib = _load_genalyzer_library()

del _load_genalyzer_library, _os, _sys
```

Then remove the now-unused import on line 12. Change:

```python
from ctypes.util import find_library as _find_library
```

to (delete the line entirely — `find_library` is no longer referenced in this file).

- [ ] **Step 6: Rewire `simplified_beta.py` to use the loader**

In `bindings/python/genalyzer/simplified_beta/simplified_beta.py`, replace lines 28-38. The current code reads:

```python
from platform import system as _system
from ctypes.util import find_library
import os

if "Windows" in _system():
    _libgen = "libgenalyzer.dll"
else:
    # Non-windows, possibly Posix system
    _libgen = "genalyzer"

_libgen = CDLL(find_library(_libgen), use_errno=True, use_last_error=True)
```

Replace with:

```python
from .._libload import load_genalyzer_library as _load_genalyzer_library

_libgen = _load_genalyzer_library(use_errno=True, use_last_error=True)
```

Note: `CDLL` may now be unused in this file's import block (lines 10-27). Leave the `from ctypes import (...)` block as-is — removing one name risks breaking the long tuple and other names are still used. Running ruff (Step 8) will report `CDLL` as unused (F401) only if it truly is; if ruff flags it, delete just the `CDLL,` line from the tuple.

- [ ] **Step 7: Smoke-test both loaders against a system-installed library**

This confirms the rewiring still imports when a real `libgenalyzer` is present. Build + install the library system-wide first (portable flags), then import:

```bash
cd /home/tcollins/hal-fleet/genalyzer
bash ./.github/scripts/install_dependencies.sh
rm -rf build && mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DGENALYZER_NATIVE_ARCH=OFF
make -j4 && sudo make install && sudo ldconfig
cd ../bindings/python
pip install -e .
python -c "import genalyzer; print('pygenalyzer OK', genalyzer.__version__)"
python -c "import genalyzer.simplified_beta; print('simplified_beta OK')"
```

Expected: both lines print `OK`. (If the environment cannot `sudo make install`, this step may be deferred to CI; the unit tests in Step 4 already cover the resolution logic.)

- [ ] **Step 8: Lint the changed Python**

```bash
cd bindings/python
ruff check genalyzer/ tests/test_libload.py
ruff format --check genalyzer/ tests/test_libload.py
```

Expected: no errors. If ruff reports an unused `CDLL` import in `simplified_beta.py`, delete the `CDLL,` line from the `from ctypes import (...)` tuple and re-run.

- [ ] **Step 9: Commit**

```bash
cd /home/tcollins/hal-fleet/genalyzer
git add bindings/python/genalyzer/_libload.py bindings/python/tests/test_libload.py bindings/python/genalyzer/pygenalyzer.py bindings/python/genalyzer/simplified_beta/simplified_beta.py
git commit -m "feat: load bundled libgenalyzer from package dir with find_library fallback"
```

---

## Task 3: Force a platform-specific `py3-none-<plat>` wheel + declare the library as package data

A pure-Python package builds a `py3-none-any` wheel, which would advertise itself as universal while only working on the CI builder's platform. Add a `setup.py` shim that (a) marks the distribution non-pure so a platform tag is emitted, and (b) overrides the tag to `py3-none-<plat>` (version-independent, since the lib is ctypes-loaded). Declare the bundled library filenames as package data so setuptools includes whatever `before-all` stages into the package dir.

**Files:**
- Create: `bindings/python/setup.py`
- Modify: `bindings/python/pyproject.toml:1-3` (build-system), `:67-68` (package-data)

**Interfaces:**
- Consumes: nothing (metadata stays in `pyproject.toml`'s `[project]` table).
- Produces: wheels tagged `py3-none-<platform>` whenever `genalyzer/libgenalyzer.{so,dylib,dll}` is present at build time.

- [ ] **Step 1: Bump the setuptools floor**

In `bindings/python/pyproject.toml`, change the build-system block (lines 1-3) from:

```toml
[build-system]
requires = ["setuptools>=40.6.0"]
build-backend = "setuptools.build_meta"
```

to:

```toml
[build-system]
requires = ["setuptools>=70.1", "wheel"]
build-backend = "setuptools.build_meta"
```

(`setuptools>=70.1` is required for the `setuptools.command.bdist_wheel` import used below; `wheel` is the fallback import path.)

- [ ] **Step 2: Create `setup.py`**

Create `bindings/python/setup.py`:

```python
# Copyright (C) 2024-2026 Analog Devices, Inc.
#
# SPDX short identifier: ADIBSD OR GPL-2.0-or-later
"""Build shim that forces a platform-specific, Python-version-independent wheel.

All package metadata lives in pyproject.toml. This file only customizes the
wheel build so the bundled libgenalyzer shared library makes the wheel
platform-specific, tagged ``py3-none-<platform>`` (the library is loaded via
ctypes and does not depend on the CPython ABI, so one wheel serves all Py3
interpreters on a given OS/arch).
"""

from setuptools import setup
from setuptools.dist import Distribution

try:
    from setuptools.command.bdist_wheel import bdist_wheel as _bdist_wheel
except ImportError:  # setuptools < 70.1
    from wheel.bdist_wheel import bdist_wheel as _bdist_wheel


class BinaryDistribution(Distribution):
    """Marks the distribution as containing platform-specific binaries."""

    def has_ext_modules(self):  # noqa: D401 - setuptools hook
        return True


class bdist_wheel(_bdist_wheel):
    """Emit a platform-specific, version-independent (py3-none-<plat>) wheel."""

    def finalize_options(self):
        super().finalize_options()
        self.root_is_pure = False

    def get_tag(self):
        _python, _abi, plat = super().get_tag()
        return "py3", "none", plat


setup(distclass=BinaryDistribution, cmdclass={"bdist_wheel": bdist_wheel})
```

- [ ] **Step 3: Declare the shared library as package data**

In `bindings/python/pyproject.toml`, replace the package-data block (lines 67-68). The current code reads:

```toml
[tool.setuptools.package-data]
genalyzer = ["genalyzer/pytest/templates/genalyzer/*"]
```

Replace with:

```toml
[tool.setuptools.package-data]
genalyzer = [
    "genalyzer/pytest/templates/genalyzer/*",
    "genalyzer/pytest/templates/genalyzer/icons/*",
    "libgenalyzer.so",
    "libgenalyzer.dylib",
    "libgenalyzer.dll",
]
```

(Globs that do not match any file are silently ignored, so listing all three names on every platform is safe.)

- [ ] **Step 4: Verify a platform wheel tag is produced when a library is present**

Stage a dummy library into the package dir, build a wheel, and inspect its tag:

```bash
cd bindings/python
cp /dev/null genalyzer/libgenalyzer.so   # dummy stand-in for the real lib
python -m build --wheel --no-isolation 2>/dev/null || pip install build && python -m build --wheel
ls dist/
```

Expected: a wheel named like `genalyzer-0.1.4-py3-none-linux_x86_64.whl` (NOT `py3-none-any`). Confirm the dummy lib is inside it:

```bash
python -c "import zipfile,glob; w=sorted(glob.glob('dist/*.whl'))[-1]; print(w); print([n for n in zipfile.ZipFile(w).namelist() if 'libgenalyzer' in n])"
```

Expected: the printed wheel name contains `py3-none-linux` and the namelist shows `genalyzer/libgenalyzer.so`.

- [ ] **Step 5: Clean up the dummy artifacts**

```bash
cd bindings/python
rm -f genalyzer/libgenalyzer.so
rm -rf dist build genalyzer.egg-info
```

(The real library is never committed — it is built per-platform in CI. Ensure `genalyzer/libgenalyzer.*` is not accidentally staged.)

- [ ] **Step 6: Commit**

```bash
cd /home/tcollins/hal-fleet/genalyzer
git add bindings/python/setup.py bindings/python/pyproject.toml
git commit -m "build: emit platform-specific py3-none wheel and package the bundled library"
```

---

## Task 4: Add cibuildwheel configuration and per-platform `before-all` scripts

Configure cibuildwheel to build `libgenalyzer` with CMake, stage the shared library into the package, vendor FFTW, and smoke-test the import on every interpreter. The CMake project lives at the repo root (`{project}`); the package is `bindings/python` (`{package}`).

**Files:**
- Create: `.github/scripts/cibw/before_all_linux.sh`
- Create: `.github/scripts/cibw/before_all_macos.sh`
- Create: `.github/scripts/cibw/before_all_windows.sh`
- Create: `.github/scripts/cibw/setup_fftw_windows.ps1`
- Modify: `bindings/python/pyproject.toml` (append `[tool.cibuildwheel]` sections at end of file)

**Interfaces:**
- Consumes: `GENALYZER_NATIVE_ARCH=OFF` option (Task 1); `setup.py` platform-tag shim and `libgenalyzer.*` package-data (Task 3); the `_libload` loader that reads `genalyzer/libgenalyzer.*` (Task 2).
- Produces: a built `genalyzer/libgenalyzer.{so,dylib,dll}` inside `{package}` before each wheel build; repaired wheels with FFTW vendored.

- [ ] **Step 1: Create the Linux `before-all` script**

Create `.github/scripts/cibw/before_all_linux.sh`:

```bash
#!/usr/bin/env bash
# Runs ONCE inside the manylinux (AlmaLinux/CentOS) container.
# $1 = {project} (repo root). Installs FFTW, builds libgenalyzer with portable
# flags, and stages the .so into the Python package so setuptools packages it.
set -euo pipefail

PROJECT="${1:?project root path required}"

# FFTW dev headers live in EPEL on the manylinux base images.
if command -v dnf >/dev/null 2>&1; then
    dnf install -y epel-release || yum install -y epel-release || true
    dnf install -y fftw-devel
else
    yum install -y epel-release || true
    yum install -y fftw-devel
fi

cmake -S "${PROJECT}" -B "${PROJECT}/build" \
    -DCMAKE_BUILD_TYPE=Release \
    -DGENALYZER_NATIVE_ARCH=OFF
cmake --build "${PROJECT}/build" -j"$(nproc)"

# Dereference any versioned soname into a plain libgenalyzer.so next to the .py.
cp -L "${PROJECT}/build/bindings/c/src/libgenalyzer.so" \
    "${PROJECT}/bindings/python/genalyzer/libgenalyzer.so"
echo "Staged libgenalyzer.so:"
ls -l "${PROJECT}/bindings/python/genalyzer/libgenalyzer.so"
```

- [ ] **Step 2: Create the macOS `before-all` script**

Create `.github/scripts/cibw/before_all_macos.sh`:

```bash
#!/usr/bin/env bash
# Runs ONCE on the macOS host. $1 = {project} (repo root).
set -euo pipefail

PROJECT="${1:?project root path required}"

brew install fftw

cmake -S "${PROJECT}" -B "${PROJECT}/build" \
    -DCMAKE_BUILD_TYPE=Release
cmake --build "${PROJECT}/build" -j"$(sysctl -n hw.ncpu)"

# CMake produces libgenalyzer.<version>.dylib + a libgenalyzer.dylib symlink.
cp -L "${PROJECT}/build/bindings/c/src/libgenalyzer.dylib" \
    "${PROJECT}/bindings/python/genalyzer/libgenalyzer.dylib"
echo "Staged libgenalyzer.dylib:"
ls -l "${PROJECT}/bindings/python/genalyzer/libgenalyzer.dylib"
```

(macOS does not use `-march=native`, so `GENALYZER_NATIVE_ARCH` is irrelevant there; the flag block in CMakeLists is Linux-only.)

- [ ] **Step 3: Create the Windows FFTW setup script**

Create `.github/scripts/cibw/setup_fftw_windows.ps1`. This mirrors the existing `test-win.yml` FFTW handling: download the prebuilt FFTW DLLs and generate MSVC import libraries (`.lib`) so the CMake linker can resolve FFTW symbols. The DLL ships in the wheel; the `.lib` is link-time only.

```powershell
# Downloads FFTW Windows DLLs into deps\fftw3 and builds MSVC import libs.
# Requires the MSVC dev environment (lib.exe) to be active on PATH.
$ErrorActionPreference = "Stop"
$fftwDir = Join-Path $PSScriptRoot "..\..\..\deps\fftw3"
New-Item -ItemType Directory -Force -Path $fftwDir | Out-Null
Push-Location $fftwDir
Invoke-WebRequest -Uri "https://fftw.org/pub/fftw/fftw-3.3.5-dll64.zip" -OutFile "fftw.zip"
Expand-Archive -Path "fftw.zip" -DestinationPath "." -Force
Remove-Item "fftw.zip"
foreach ($name in @("libfftw3-3", "libfftw3f-3", "libfftw3l-3")) {
    lib /def:"$name.def" /machine:x64 /out:"$name.lib"
}
Get-ChildItem
Pop-Location
```

- [ ] **Step 4: Create the Windows `before-all` script**

Create `.github/scripts/cibw/before_all_windows.sh`. cibuildwheel runs `before-all` in Git-bash on Windows. FFTW is prepared by a prior CI step (Task 5) into `deps/fftw3`; here we configure CMake (the Visual Studio generator locates MSVC itself — no vcvars needed) and stage the DLL.

```bash
#!/usr/bin/env bash
# Runs ONCE on the Windows host (Git-bash). $1 = {project} (repo root).
# Assumes deps/fftw3 has been populated by setup_fftw_windows.ps1.
set -euo pipefail

PROJECT="${1:?project root path required}"
FFTW_DIR="${PROJECT}/deps/fftw3"

cmake -S "${PROJECT}" -B "${PROJECT}/build" \
    -G "Visual Studio 17 2022" -A x64 \
    -DFFTW_INCLUDE_DIRS="${FFTW_DIR}" \
    -DFFTW_LIBRARIES="${FFTW_DIR}/libfftw3-3.lib"
cmake --build "${PROJECT}/build" --config Release

cp "${PROJECT}/build/bindings/c/src/Release/libgenalyzer.dll" \
    "${PROJECT}/bindings/python/genalyzer/libgenalyzer.dll"
echo "Staged libgenalyzer.dll:"
ls -l "${PROJECT}/bindings/python/genalyzer/libgenalyzer.dll"
```

- [ ] **Step 5: Make the scripts executable**

```bash
chmod +x .github/scripts/cibw/before_all_linux.sh .github/scripts/cibw/before_all_macos.sh .github/scripts/cibw/before_all_windows.sh
```

- [ ] **Step 6: Append `[tool.cibuildwheel]` config to `pyproject.toml`**

Append to the end of `bindings/python/pyproject.toml`:

```toml
[tool.cibuildwheel]
# Build CPython 3.9-3.13. Because the library is ctypes-loaded the resulting
# wheel is tagged py3-none-<plat>; cibuildwheel reuses the single wheel across
# interpreters and runs the test-command on each.
build = "cp39-* cp310-* cp311-* cp312-* cp313-*"
# Skip 32-bit and musl targets (no FFTW wheel demand; reduces matrix).
skip = "*-win32 *_i686 *-musllinux*"
build-frontend = "build"
test-command = "python -c \"import genalyzer; import genalyzer.simplified_beta; print('genalyzer', genalyzer.__version__, 'import OK')\""

[tool.cibuildwheel.linux]
# {project} = repo root (copied into the manylinux container).
before-all = "bash {project}/.github/scripts/cibw/before_all_linux.sh {project}"

[tool.cibuildwheel.macos]
before-all = "bash {project}/.github/scripts/cibw/before_all_macos.sh {project}"
# Help delocate find Homebrew FFTW on both Intel and Apple Silicon runners.
[tool.cibuildwheel.macos.environment]
DYLD_LIBRARY_PATH = "/opt/homebrew/lib:/usr/local/lib"

[tool.cibuildwheel.windows]
before-all = "bash {project}/.github/scripts/cibw/before_all_windows.sh {project}"
# Treat the ctypes-loaded libgenalyzer.dll as a scan root and tell delvewheel
# where to find the FFTW runtime DLL to vendor.
repair-wheel-command = "delvewheel repair --analyze-existing --add-path deps\\fftw3 -w {dest_dir} {wheel}"
```

Notes for the implementer:
- Linux/macOS use the **default** repair commands (`auditwheel repair` / `delocate-wheel`); only Windows needs the override for `--analyze-existing`.
- Do **not** add a root-level `pyproject.toml`; cibuildwheel reads `[tool.cibuildwheel]` from the package's `pyproject.toml` (`bindings/python/pyproject.toml`) because we pass `bindings/python` as the package argument (Task 5).

- [ ] **Step 7: Local Linux smoke build (one interpreter, one arch)**

Run a single-target cibuildwheel build locally to validate the config end to end (requires Docker). From the repo root:

```bash
cd /home/tcollins/hal-fleet/genalyzer
pipx run cibuildwheel==2.* --platform linux --only cp312-manylinux_x86_64 bindings/python 2>&1 | tee /tmp/cibw-smoke.log; echo "exit: ${PIPESTATUS[0]}"
```

Expected: `exit: 0`; the log ends with the `test-command` printing `genalyzer 0.1.4 import OK`; a wheel appears in `./wheelhouse/`.

- [ ] **Step 8: Verify the repaired wheel bundles both libgenalyzer and FFTW**

```bash
cd /home/tcollins/hal-fleet/genalyzer
python -c "
import glob, zipfile
w = sorted(glob.glob('wheelhouse/*.whl'))[-1]
names = zipfile.ZipFile(w).namelist()
print('wheel:', w)
print('libgenalyzer:', [n for n in names if 'libgenalyzer' in n])
print('vendored fftw :', [n for n in names if 'fftw' in n.lower()])
"
```

Expected: the wheel name contains `py3-none-manylinux`; `libgenalyzer` shows `genalyzer/libgenalyzer.so`; `vendored fftw` shows an entry under `genalyzer.libs/` (e.g. `genalyzer.libs/libfftw3-<hash>.so.3`).

- [ ] **Step 9: Clean up local build output**

```bash
cd /home/tcollins/hal-fleet/genalyzer
rm -rf wheelhouse build deps bindings/python/genalyzer/libgenalyzer.so
git status --short   # confirm no built artifacts are staged
```

- [ ] **Step 10: Commit**

```bash
git add .github/scripts/cibw/ bindings/python/pyproject.toml
git commit -m "ci: add cibuildwheel config and per-platform native-build scripts"
```

---

## Task 5: GitHub Actions workflow — build wheels + sdist and publish

Replace the two existing deploy workflows (which build/upload a broken pure-Python wheel) with one matrix workflow that builds platform wheels for Linux (x86_64 + aarch64), macOS (Intel + Apple Silicon), and Windows, builds an sdist, and publishes to TestPyPI (on push to `main`) and PyPI (on tags).

**Files:**
- Create: `.github/workflows/wheels.yml`
- Delete: `.github/workflows/deploy-pypi.yml`
- Delete: `.github/workflows/deploy-pypi-testing.yml`

**Interfaces:**
- Consumes: `[tool.cibuildwheel]` config + `before-all` scripts (Task 4); the `setup.py` shim (Task 3).
- Produces: `*.whl` per platform + one `*.tar.gz` sdist; uploads to the package indexes.

- [ ] **Step 1: Create the workflow**

Create `.github/workflows/wheels.yml`:

```yaml
name: Wheels

on:
  push:
    branches: [main]
    tags: ["v*", "*.*.*"]
  pull_request:
  workflow_dispatch:

jobs:
  build_wheels:
    name: Wheels on ${{ matrix.os }} (${{ matrix.archs }})
    runs-on: ${{ matrix.os }}
    strategy:
      fail-fast: false
      matrix:
        include:
          - os: ubuntu-latest
            archs: x86_64
          - os: ubuntu-24.04-arm
            archs: aarch64
          - os: macos-13      # Intel
            archs: x86_64
          - os: macos-14      # Apple Silicon
            archs: arm64
          - os: windows-latest
            archs: AMD64
    steps:
      - uses: actions/checkout@v4

      # Windows-only: set up MSVC (for lib.exe) and prepare FFTW import libs/DLLs.
      - name: Set up MSVC (Windows)
        if: runner.os == 'Windows'
        uses: ilammy/msvc-dev-cmd@v1
      - name: Prepare FFTW (Windows)
        if: runner.os == 'Windows'
        shell: pwsh
        run: ./.github/scripts/cibw/setup_fftw_windows.ps1

      - name: Build wheels
        uses: pypa/cibuildwheel@v2.21.3
        with:
          package-dir: bindings/python
        env:
          CIBW_ARCHS: ${{ matrix.archs }}

      - uses: actions/upload-artifact@v4
        with:
          name: cibw-wheels-${{ matrix.os }}-${{ matrix.archs }}
          path: ./wheelhouse/*.whl

  build_sdist:
    name: Build sdist
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Build sdist
        run: pipx run build --sdist
        working-directory: bindings/python
      - uses: actions/upload-artifact@v4
        with:
          name: cibw-sdist
          path: bindings/python/dist/*.tar.gz

  publish_testpypi:
    name: Publish to TestPyPI
    needs: [build_wheels, build_sdist]
    runs-on: ubuntu-latest
    if: github.event_name == 'push' && github.ref == 'refs/heads/main'
    steps:
      - uses: actions/download-artifact@v4
        with:
          pattern: cibw-*
          path: dist
          merge-multiple: true
      - uses: pypa/gh-action-pypi-publish@release/v1
        with:
          user: __token__
          password: ${{ secrets.TEST_PYPI_TOKEN }}
          repository-url: https://test.pypi.org/legacy/
          skip-existing: true

  publish_pypi:
    name: Publish to PyPI
    needs: [build_wheels, build_sdist]
    runs-on: ubuntu-latest
    if: github.event_name == 'push' && startsWith(github.ref, 'refs/tags')
    steps:
      - uses: actions/download-artifact@v4
        with:
          pattern: cibw-*
          path: dist
          merge-multiple: true
      - uses: pypa/gh-action-pypi-publish@release/v1
        with:
          user: __token__
          password: ${{ secrets.PYPI_TOKEN }}
```

Notes for the implementer:
- `package-dir: bindings/python` makes the cibuildwheel action run from the repo root with `bindings/python` as the package, so `{project}` = repo root (where CMake lives) and the `[tool.cibuildwheel]` config is read from `bindings/python/pyproject.toml`.
- The sdist is built from `bindings/python` (pure-Python fallback; it does not contain the C sources and will only build against a system-installed libgenalyzer — matching today's behavior).
- This reuses the existing `secrets.PYPI_TOKEN` / `secrets.TEST_PYPI_TOKEN` already configured for the old workflows. (Optional future hardening: switch to PyPI Trusted Publishing with `permissions: id-token: write` and an `environment:`; out of scope here to avoid a one-time PyPI config dependency.)
- `ubuntu-24.04-arm` is a native ARM runner (no QEMU). If native ARM runners are unavailable for this org, replace that matrix entry with QEMU: keep `ubuntu-latest`, add `docker/setup-qemu-action@v3` with `platforms: arm64`, and set `CIBW_ARCHS: aarch64`.

- [ ] **Step 2: Delete the superseded deploy workflows**

```bash
cd /home/tcollins/hal-fleet/genalyzer
git rm .github/workflows/deploy-pypi.yml .github/workflows/deploy-pypi-testing.yml
```

- [ ] **Step 3: Validate the workflow YAML syntax**

```bash
python -c "import yaml,sys; yaml.safe_load(open('.github/workflows/wheels.yml')); print('wheels.yml: valid YAML')"
```

Expected: `wheels.yml: valid YAML`. (If `actionlint` is available, also run `actionlint .github/workflows/wheels.yml` for deeper checks.)

- [ ] **Step 4: Commit**

```bash
git add .github/workflows/wheels.yml
git commit -m "ci: build and publish platform wheels + sdist, retire pure-python deploy"
```

---

## Task 6: Documentation and `.gitignore` hygiene

Ensure built artifacts never get committed and document the new install/build story.

**Files:**
- Modify: `.gitignore`
- Modify: `README.md` (installation section)

**Interfaces:**
- Consumes: nothing.
- Produces: ignore rules for staged libraries and wheel output; user-facing install docs.

- [ ] **Step 1: Ignore staged libraries and wheel output**

Check current ignores, then append the missing ones:

```bash
cd /home/tcollins/hal-fleet/genalyzer
grep -qxF "wheelhouse/" .gitignore || printf '\n# cibuildwheel output and staged native libs\nwheelhouse/\ndeps/\nbindings/python/genalyzer/libgenalyzer.so\nbindings/python/genalyzer/libgenalyzer.dylib\nbindings/python/genalyzer/libgenalyzer.dll\n' >> .gitignore
```

Verify the entries are present:

```bash
tail -n 8 .gitignore
```

Expected: the new block is shown.

- [ ] **Step 2: Document the install/build story in the README**

Locate the installation section and add a short paragraph. First find it:

```bash
grep -n -i "pip install\|installation\|install" README.md | head
```

Then add (under the existing pip-install guidance — adapt the exact wording to the surrounding prose using the technical-writing-style skill):

```markdown
## Installing the Python package

`pip install genalyzer` installs a platform wheel that already bundles the
compiled `libgenalyzer` library and its FFTW dependency — no system library or
compiler is required. Wheels are published for Linux (x86_64, aarch64), macOS
(Intel and Apple Silicon), and Windows (x64).

If you install from source (the sdist, or `pip install .`), you must build and
install `libgenalyzer` system-wide first (see the build instructions above); the
Python loader falls back to locating it on the system library path.
```

- [ ] **Step 3: Commit**

```bash
cd /home/tcollins/hal-fleet/genalyzer
git add .gitignore README.md
git commit -m "docs: document bundled-wheel install and ignore built artifacts"
```

---

## Verification Checklist (run after all tasks)

- [ ] **Unit tests pass:** `cd bindings/python && python -m pytest tests/test_libload.py -v` → 5 passed.
- [ ] **Linux smoke wheel imports cleanly** (from Task 4 Step 7-8): wheel is `py3-none-manylinux*`, contains `genalyzer/libgenalyzer.so` and a vendored `genalyzer.libs/libfftw3*`, and `test-command` printed `import OK`.
- [ ] **No build artifacts staged:** `git status --short` shows no `*.whl`, `wheelhouse/`, `deps/`, or `genalyzer/libgenalyzer.*`.
- [ ] **Lint clean:** `cd bindings/python && ruff check genalyzer/ tests/ && ruff format --check genalyzer/ tests/`.
- [ ] **Existing Python tests still pass** against a system-installed library: `cd bindings/python && pip install ".[tools,pytest,mcp]" && python -m pytest -vs tests` (requires `libgenalyzer` installed per Task 2 Step 7).
- [ ] **CI dry run:** push the branch and confirm the `Wheels` workflow builds all five matrix legs green and produces the expected artifacts (no publish on a feature branch).

---

## Self-Review Notes (coverage against the task)

- **"PyPI package is pure-Python and relies on `find_library('genalyzer')`"** → Task 2 rewrites both loaders (`pygenalyzer.py`, `simplified_beta.py`) to load the bundled library first, keeping `find_library` only as a fallback.
- **"no `.so`/`.dll` shipped"** → Task 4 builds and stages the library; Task 3 packages it; cibuildwheel repair vendors FFTW.
- **"no manylinux/macOS/Windows wheel matrix"** → Task 5 adds the full matrix (Linux x86_64+aarch64, macOS Intel+ARM, Windows x64) plus sdist and publishing.
- **macOS gap** (the old `pygenalyzer.py` raised on `darwin`) → fixed in Task 2 (`_libload` handles `darwin` → `libgenalyzer.dylib`).
- **Portability hazard** (`-march=native`) → fixed in Task 1.
- **`broken import for end users`** → the `test-command` in Task 4 (`import genalyzer; import genalyzer.simplified_beta`) runs on every interpreter in CI, failing the build if the bundled import does not work.
```
