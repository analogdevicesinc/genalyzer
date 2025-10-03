#!/bin/bash
# Script to build genalyzer-fftw wheel locally using FFTW from pyFFTW package
# This mimics the GitHub Actions workflow for local testing

set -e  # Exit on error

PIP_CMD=${PIP_CMD:-pip}
echo "PIP command: $PIP_CMD"

echo "=========================================="
echo "Building genalyzer-fftw wheel locally"
echo "Using FFTW bundled in pyFFTW wheels"
echo "=========================================="

# Check if running from repo root
if [ ! -f "CMakeLists.txt" ]; then
    echo "Error: Please run this script from the repository root"
    exit 1
fi

# Install Python dependencies including pyFFTW (which bundles FFTW)
echo ""
echo "Step 1: Installing Python dependencies..."
$PIP_CMD install --only-binary=:all: pyfftw  # Ensure we get the wheel, not build from source
$PIP_CMD install numpy wheel setuptools build

# Extract FFTW library location from pyFFTW
echo ""
echo "Step 2: Locating FFTW libraries in pyFFTW package..."
python bindings/python/genalyzer_fftw/extract_fftw_from_pyfftw.py

# Download FFTW header (pyFFTW wheels don't include headers)
echo ""
echo "Step 2.5: Downloading FFTW header file..."
python bindings/python/genalyzer_fftw/download_fftw_header.py

# Get the FFTW library path
PYFFTW_PATH=$(python -c "import pyfftw; from pathlib import Path; print(Path(pyfftw.__file__).parent)")
echo "PyFFTW package location: $PYFFTW_PATH"

# Determine library subdirectory based on platform
if [ "$(uname)" == "Darwin" ]; then
    # macOS - check for .dylibs directory
    if [ -d "$PYFFTW_PATH/.dylibs" ]; then
        FFTW_LIB_DIR="$PYFFTW_PATH/.dylibs"
    else
        FFTW_LIB_DIR="$PYFFTW_PATH"
    fi
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
    # Linux - check for pyFFTW.libs (newer, sibling to package) or .libs (older, inside package)
    PYFFTW_PARENT=$(dirname "$PYFFTW_PATH")
    if [ -d "$PYFFTW_PARENT/pyFFTW.libs" ]; then
        FFTW_LIB_DIR="$PYFFTW_PARENT/pyFFTW.libs"
    elif [ -d "$PYFFTW_PATH/.libs" ]; then
        FFTW_LIB_DIR="$PYFFTW_PATH/.libs"
    else
        FFTW_LIB_DIR="$PYFFTW_PATH"
    fi
else
    FFTW_LIB_DIR="$PYFFTW_PATH"
fi

echo "FFTW library directory: $FFTW_LIB_DIR"

# List FFTW libraries found
echo ""
echo "FFTW libraries found in pyFFTW:"
if [ "$(uname)" == "Darwin" ]; then
    find "$FFTW_LIB_DIR" -name "*fftw3*.dylib" 2>/dev/null || echo "  No .dylib files found"
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
    find "$FFTW_LIB_DIR" -name "*fftw3*.so*" 2>/dev/null || echo "  No .so files found"
else
    find "$FFTW_LIB_DIR" -name "*fftw3*.dll" 2>/dev/null || echo "  No .dll files found"
fi

# Configure CMake
echo ""
echo "Step 3: Configuring CMake with FFTW from pyFFTW..."

# Set include directory path (absolute path to downloaded header)
FFTW_INCLUDE_DIR="$(pwd)/bindings/python/genalyzer_fftw/include"
echo "FFTW include directory: $FFTW_INCLUDE_DIR"

# Verify header exists
if [ ! -f "$FFTW_INCLUDE_DIR/fftw3.h" ]; then
    echo "ERROR: fftw3.h not found at $FFTW_INCLUDE_DIR/fftw3.h"
    echo "The download may have failed. Please check the error messages above."
    exit 1
fi
echo "✓ fftw3.h found"

# Clean build directory completely to avoid CMake cache issues
echo "Cleaning build directory..."
rm -rf build CMakeCache.txt CMakeFiles

mkdir -p build
cd build

# Set local install prefix MUST be set at configure time, not install time
LOCAL_INSTALL_PREFIX="$(pwd)/install"

echo "Running CMake with:"
echo "  CMAKE_INSTALL_PREFIX=$LOCAL_INSTALL_PREFIX"
echo "  FFTW_ROOT=$FFTW_LIB_DIR"
echo "  FFTW_INCLUDE_DIRS=$FFTW_INCLUDE_DIR"

cmake .. \
    -DCMAKE_INSTALL_PREFIX="$LOCAL_INSTALL_PREFIX" \
    -DFFTW_ROOT="$FFTW_LIB_DIR" \
    -DFFTW_INCLUDE_DIRS="$FFTW_INCLUDE_DIR" \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTS_EXAMPLES=OFF \
    -DBUILD_DOC=OFF

# Build C++ library
echo ""
echo "Step 4: Building C++ library..."
cmake --build . --config Release -j4

# Install to local prefix (prefix was set during configure)
echo ""
echo "Step 5: Installing genalyzer C++ library to local build directory..."
cmake --install . --config Release

echo "✓ Local installation at: $LOCAL_INSTALL_PREFIX"

# Set up environment for Python build to find local libraries
export CMAKE_PREFIX_PATH="$LOCAL_INSTALL_PREFIX:$CMAKE_PREFIX_PATH"
export LD_LIBRARY_PATH="$LOCAL_INSTALL_PREFIX/lib:$LD_LIBRARY_PATH"
export DYLD_LIBRARY_PATH="$LOCAL_INSTALL_PREFIX/lib:$DYLD_LIBRARY_PATH"
export PKG_CONFIG_PATH="$LOCAL_INSTALL_PREFIX/lib/pkgconfig:$PKG_CONFIG_PATH"

cd ..

# Build Python wheel
echo ""
echo "Step 6: Building Python wheel (using local installation)..."
cd bindings/python/genalyzer_fftw

# Clean previous builds
rm -rf dist build *.egg-info

# Build the wheel in development mode pointing to local installation
# Set GENALYZER_PREFIX for setup.py to find the local installation
export GENALYZER_PREFIX="$LOCAL_INSTALL_PREFIX"

python -m build --wheel

# List wheel contents
echo ""
echo "Step 7: Inspecting wheel contents..."
cd dist
for whl in *.whl; do
    echo ""
    echo "Contents of $whl:"
    python -m zipfile -l "$whl" | head -50
    echo "..."
done
cd ..

# Test wheel installation
echo ""
echo "Step 8: Testing wheel installation..."
$PIP_CMD install --force-reinstall dist/*.whl

echo ""
echo "Step 9: Testing import..."
python -c "import genalyzer; print('✓ Genalyzer imported successfully')"
python -c "import pyfftw; print('✓ pyFFTW available')"
python -c "import genalyzer, pyfftw; print('✓ Both packages working together')"

echo ""
echo "=========================================="
echo "✓ Build completed successfully!"
echo "=========================================="
echo ""
echo "Build artifacts:"
echo "  C++ library:   build/install/"
echo "  Python wheel:  bindings/python/genalyzer_fftw/dist/"
echo ""
ls -lh bindings/python/genalyzer_fftw/dist/*.whl
echo ""
echo "Development notes:"
echo "  - C++ library installed locally to build/install/ (no sudo required)"
echo "  - The wheel depends on pyFFTW, which provides FFTW libraries"
echo "  - For iteration, you can rebuild without reinstalling system-wide"
echo ""
