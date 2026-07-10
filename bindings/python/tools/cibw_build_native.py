# Copyright (C) 2024-2026 Analog Devices, Inc.
#
# SPDX short identifier: ADIBSD OR GPL-2.0-or-later
"""Build libgenalyzer and stage it inside the Python package for cibuildwheel."""

from __future__ import annotations

import os
import platform
import shutil
import subprocess
import sys
from pathlib import Path

PACKAGE_ROOT = Path(__file__).resolve().parents[1]
REPO_ROOT = PACKAGE_ROOT.parents[1]
PACKAGE_DIR = REPO_ROOT / "bindings" / "python-fftw" / "genalyzer_fftw"


def _run(command: list[str], **kwargs) -> None:
    print("+", " ".join(command), flush=True)
    subprocess.run(command, check=True, **kwargs)


def _native_build_dir() -> Path:
    build_tag = os.environ.get(
        "CIBW_BUILD", f"py{sys.version_info.major}{sys.version_info.minor}"
    )
    safe_tag = "".join(
        char if char.isalnum() or char in ".-_" else "-" for char in build_tag
    )
    return REPO_ROOT / "build" / f"cibw-{safe_tag}"


def _clean_staged_libraries() -> None:
    for pattern in (
        "libgenalyzer.so*",
        "libgenalyzer.dylib*",
        "libgenalyzer.dll",
        "genalyzer.dll",
        "libfftw3*.dll",
    ):
        for library_path in PACKAGE_DIR.glob(pattern):
            library_path.unlink()


def _cmake_configure_args(build_dir: Path) -> list[str]:
    args = [
        "cmake",
        "-S",
        str(REPO_ROOT),
        "-B",
        str(build_dir),
        "-DCMAKE_BUILD_TYPE=Release",
        "-DBUILD_TESTS_EXAMPLES=OFF",
        "-DGENALYZER_BUILD_FRAMEWORKS=OFF",
    ]

    if sys.platform == "win32":
        fftw_dir = REPO_ROOT / "deps" / "fftw3"
        args.extend(
            [
                "-G",
                "Ninja",
                f"-DFFTW_INCLUDE_DIRS={fftw_dir}",
                f"-DFFTW_LIBRARIES={fftw_dir / 'libfftw3-3.lib'}",
            ]
        )

    return args


def _cmake_env() -> dict[str, str]:
    env = os.environ.copy()
    if sys.platform != "win32":
        # Avoid user-local helper scripts shadowing toolchain executables such as
        # GNU as(1) in local developer environments.
        env["PATH"] = os.pathsep.join(
            [
                "/usr/local/sbin",
                "/usr/local/bin",
                "/usr/sbin",
                "/usr/bin",
                "/sbin",
                "/bin",
                env.get("PATH", ""),
            ]
        )
    return env


def _stage_outputs(build_dir: Path) -> None:
    if sys.platform == "win32":
        candidates = [
            build_dir / "bindings" / "c" / "src" / "libgenalyzer.dll",
            build_dir / "bindings" / "c" / "src" / "Release" / "libgenalyzer.dll",
        ]
    elif sys.platform == "darwin":
        candidates = sorted(
            (build_dir / "bindings" / "c" / "src").glob("libgenalyzer*.dylib*")
        )
    else:
        candidates = sorted(
            (build_dir / "bindings" / "c" / "src").glob("libgenalyzer.so*")
        )

    copied = []
    for source_path in candidates:
        if source_path.exists():
            dest_path = PACKAGE_DIR / source_path.name
            if source_path.is_symlink():
                target = os.readlink(source_path)
                if dest_path.exists() or dest_path.is_symlink():
                    dest_path.unlink()
                dest_path.symlink_to(target)
            else:
                shutil.copy2(source_path, dest_path)
            copied.append(dest_path)

    if sys.platform == "win32":
        fftw_dir = REPO_ROOT / "deps" / "fftw3"
        for dll_path in fftw_dir.glob("libfftw3*.dll"):
            dest_path = PACKAGE_DIR / dll_path.name
            shutil.copy2(dll_path, dest_path)
            copied.append(dest_path)

    if not copied:
        raise FileNotFoundError(f"No libgenalyzer artifact found under {build_dir}")

    print("Staged native libraries:")
    for path in copied:
        print(f"  {path.relative_to(REPO_ROOT)}")


def main() -> None:
    print(f"Building native Genalyzer library on {platform.platform()}")
    build_dir = _native_build_dir()
    _clean_staged_libraries()

    env = _cmake_env()
    _run(_cmake_configure_args(build_dir), env=env)
    build_command = ["cmake", "--build", str(build_dir), "--target", "genalyzer"]
    if sys.platform == "win32":
        build_command.extend(["--config", "Release"])
    else:
        build_command.extend(["-j", str(os.cpu_count() or 2)])
    _run(build_command, env=env)
    _stage_outputs(build_dir)


if __name__ == "__main__":
    main()
