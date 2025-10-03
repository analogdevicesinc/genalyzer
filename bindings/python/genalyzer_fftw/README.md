# Genalyzer Binding with FFTW from pyFFTW

This directory contains the necessary scripts and configuration files to generate a Genalyzer wheel that uses FFTW libraries bundled in the pyFFTW wheels from PyPI. This allows users to install pre-built binaries with the necessary FFTW dependency without requiring a separate FFTW installation.

## Overview

The `genalyzer-fftw` package uses FFTW libraries that are already bundled in pyFFTW wheels. PyFFTW wheels from PyPI include FFTW libraries processed with `delocate` (macOS) or `auditwheel` (Linux), making them self-contained. This approach provides:

- **Easy installation**: No need to manually install FFTW - pyFFTW provides it
- **Cross-platform support**: Works on Linux, macOS, and Windows
- **Pre-built binaries**: Wheels available on PyPI for quick installation
- **Dependency management**: Leverages existing pyFFTW infrastructure

## Installation

### From PyPI (when published)

```bash
pip install genalyzer-fftw
```

This will automatically install `pyfftw` as a dependency, which provides the FFTW libraries.

### From Source

```bash
# Install pyFFTW (provides FFTW libraries)
pip install pyfftw numpy

# Build and install genalyzer-fftw
cd bindings/python/genalyzer_fftw
pip install .
```

## How It Works

1. **Build Time**: The build process:
   - Installs pyFFTW (which bundles FFTW libraries in `.libs`, `pyFFTW.libs`, or `.dylibs` directories)
   - Downloads FFTW header files (pyFFTW wheels don't include headers, only compiled libraries)
   - Extracts the location of FFTW libraries from the pyFFTW package
   - Links genalyzer against those FFTW libraries

2. **Runtime**: When users install `genalyzer-fftw`:
   - pyFFTW is automatically installed as a dependency
   - pyFFTW provides the bundled FFTW libraries
   - Genalyzer uses FFTW libraries from the pyFFTW package

**Note**: PyFFTW wheels include the compiled FFTW libraries (.so/.dylib/.dll) but not the header files (.h). The build process automatically downloads `fftw3.h` from the FFTW GitHub repository during installation.

## Files

- [pyproject.toml](pyproject.toml): Python package configuration with pyFFTW dependency
- [setup.py](setup.py): Custom build script that extracts FFTW from pyFFTW and builds with CMake
- [extract_fftw_from_pyfftw.py](extract_fftw_from_pyfftw.py): Helper script to locate FFTW libraries in pyFFTW package
- [download_fftw_header.py](download_fftw_header.py): Helper script to download FFTW header files
- [README.md](README.md): This file

## GitHub Actions Workflow

The [.github/workflows/build-fftw-wheels.yml](../../../.github/workflows/build-fftw-wheels.yml) workflow:
- Uses `cibuildwheel` to build wheels for multiple platforms (Linux, macOS, Windows)
- Builds for multiple Python versions (3.8, 3.9, 3.10, 3.11)
- Installs pyFFTW in each build environment (provides bundled FFTW libraries)
- Automatically publishes to PyPI when tags are pushed

## Testing Locally

Use the provided script from the repository root:

```bash
# From repository root
./build_fftw_wheel.sh
```

This script will:
1. Install pyFFTW (which provides bundled FFTW libraries)
2. Extract FFTW library locations from pyFFTW
3. Build the C++ library using FFTW from pyFFTW
4. Create a Python wheel
5. Test the installation

Or manually:

```bash
# Install pyFFTW (provides FFTW)
pip install --only-binary=:all: pyfftw numpy build wheel

# Optional: Inspect pyFFTW's FFTW libraries
python bindings/python/genalyzer_fftw/extract_fftw_from_pyfftw.py

# Build the wheel (setup.py automatically finds FFTW from pyFFTW)
cd bindings/python/genalyzer_fftw
python -m build --wheel

# Install and test
pip install dist/*.whl
python -c "import genalyzer; import pyfftw; print('Success!')"
```

## Key Advantages

1. **No System Dependencies**: Users don't need to install FFTW separately
2. **Consistent FFTW Version**: Everyone gets the same FFTW version from pyFFTW wheels
3. **Simplified Build**: Leverages existing pyFFTW wheel infrastructure
4. **Cross-Platform**: Works wherever pyFFTW wheels are available (Linux, macOS, Windows)