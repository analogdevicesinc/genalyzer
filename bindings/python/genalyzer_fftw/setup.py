#!/usr/bin/env python3
"""
Custom setup.py for genalyzer-fftw package.
This builds the C++ extension using CMake with FFTW from pyFFTW wheel.
PyFFTW wheels from PyPI bundle FFTW libraries using delocate/auditwheel.
"""

import os
import sys
import subprocess
import platform
from pathlib import Path
from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext


class CMakeExtension(Extension):
    def __init__(self, name, sourcedir=""):
        Extension.__init__(self, name, sources=[])
        self.sourcedir = os.path.abspath(sourcedir)


class CMakeBuild(build_ext):
    def run(self):
        try:
            subprocess.check_output(["cmake", "--version"])
        except OSError:
            raise RuntimeError(
                "CMake must be installed to build the following extensions: "
                + ", ".join(e.name for e in self.extensions)
            )

        for ext in self.extensions:
            self.build_extension(ext)

    def build_extension(self, ext):
        # Find pyFFTW and extract FFTW library locations
        try:
            import pyfftw
            pyfftw_path = Path(pyfftw.__file__).parent
            print(f"Found pyFFTW package at: {pyfftw_path}")

            # PyFFTW wheels bundle FFTW libs in various directories depending on platform
            system = platform.system()

            # Determine library pattern and possible directories
            if system == "Windows":
                lib_pattern = "*fftw3*.dll"
                possible_lib_dirs = [pyfftw_path]
            elif system == "Darwin":
                lib_pattern = "*fftw3*.dylib"
                possible_lib_dirs = [
                    pyfftw_path / ".dylibs",  # macOS (delocate)
                    pyfftw_path,
                ]
            else:  # Linux
                lib_pattern = "*fftw3*.so*"
                possible_lib_dirs = [
                    pyfftw_path.parent / "pyFFTW.libs",  # Newer auditwheel style (sibling to package)
                    pyfftw_path / ".libs",  # Older auditwheel style (inside package)
                    pyfftw_path,
                ]

            # Search for FFTW libraries in possible directories
            fftw_libs = []
            for lib_dir in possible_lib_dirs:
                if lib_dir.exists():
                    found = list(lib_dir.glob(lib_pattern))
                    if found:
                        fftw_libs = found
                        break

            if not fftw_libs:
                raise RuntimeError(
                    f"Could not find FFTW libraries in pyFFTW package at {pyfftw_path}. "
                    "The pyFFTW package may have been built from source. "
                    "Try reinstalling a pre-built wheel: pip install --force-reinstall --only-binary=:all: pyfftw"
                )

            fftw_lib_dir = fftw_libs[0].parent
            fftw_root = str(fftw_lib_dir)
            print(f"Found FFTW libraries at: {fftw_lib_dir}")
            print(f"FFTW library files found: {[lib.name for lib in fftw_libs]}")

            # PyFFTW wheels don't include headers, so we need to download them
            print("\nChecking for FFTW header file...")
            header_dir = Path(__file__).parent / "include"
            header_file = header_dir / "fftw3.h"

            if not header_file.exists():
                print("Downloading FFTW header (pyFFTW wheels don't include headers)...")
                header_dir.mkdir(exist_ok=True)
                # Download fftw3.h from GitHub
                import urllib.request
                header_url = "https://raw.githubusercontent.com/FFTW/fftw3/master/api/fftw3.h"
                try:
                    with urllib.request.urlopen(header_url) as response:
                        content = response.read()
                    with open(header_file, 'wb') as f:
                        f.write(content)
                    print(f"Downloaded fftw3.h to: {header_file}")
                except Exception as e:
                    raise RuntimeError(
                        f"Failed to download FFTW header: {e}\n"
                        "Please manually download fftw3.h from "
                        "https://github.com/FFTW/fftw3/blob/master/api/fftw3.h "
                        f"and place it in {header_dir}/"
                    )
            else:
                print(f"Using existing FFTW header: {header_file}")

            fftw_include_dir = str(header_dir)

        except ImportError:
            raise RuntimeError(
                "pyFFTW must be installed before building genalyzer-fftw. "
                "Install it with: pip install pyfftw"
            )

        extdir = os.path.abspath(os.path.dirname(self.get_ext_fullpath(ext.name)))

        # Create build directory
        build_temp = Path(self.build_temp)
        build_temp.mkdir(parents=True, exist_ok=True)

        cmake_args = [
            f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={extdir}",
            f"-DPYTHON_EXECUTABLE={sys.executable}",
            f"-DFFTW_ROOT={fftw_root}",
            f"-DFFTW_INCLUDE_DIRS={fftw_include_dir}",
            "-DBUILD_TESTS_EXAMPLES=OFF",
            "-DBUILD_DOC=OFF",
        ]

        # Check if GENALYZER_PREFIX is set (for local development builds)
        genalyzer_prefix = os.environ.get("GENALYZER_PREFIX")
        if genalyzer_prefix:
            print(f"Using local genalyzer installation: {genalyzer_prefix}")
            cmake_args.append(f"-DCMAKE_PREFIX_PATH={genalyzer_prefix}")

        # Add library path for linking
        if system != "Windows":
            cmake_args.append(f"-DCMAKE_BUILD_RPATH={fftw_lib_dir}")
            cmake_args.append(f"-DCMAKE_INSTALL_RPATH={fftw_lib_dir}")

        cfg = "Debug" if self.debug else "Release"
        build_args = ["--config", cfg]

        if sys.platform == "win32":
            cmake_args += [
                f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_{cfg.upper()}={extdir}",
            ]
            build_args += ["--", "/m"]
        else:
            cmake_args += [f"-DCMAKE_BUILD_TYPE={cfg}"]
            build_args += ["--", "-j4"]

        env = os.environ.copy()

        # Run CMake configure
        subprocess.check_call(
            ["cmake", ext.sourcedir] + cmake_args, cwd=self.build_temp, env=env
        )

        # Run CMake build
        subprocess.check_call(
            ["cmake", "--build", "."] + build_args, cwd=self.build_temp, env=env
        )


# Run the setup
if __name__ == "__main__":
    # Get the project root (3 levels up from this file)
    project_root = Path(__file__).parent.parent.parent.parent

    setup(
        ext_modules=[CMakeExtension("genalyzer._genalyzer", sourcedir=str(project_root))],
        cmdclass={"build_ext": CMakeBuild},
    )
