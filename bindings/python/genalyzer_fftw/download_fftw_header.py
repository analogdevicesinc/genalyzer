#!/usr/bin/env python3
"""
Download FFTW header file since pyFFTW wheels don't include headers.
This downloads fftw3.h from the official FFTW repository.
"""

import sys
import urllib.request
from pathlib import Path


def download_fftw_header(version="3.3.10", output_dir=None):
    """Download fftw3.h header file."""

    if output_dir is None:
        output_dir = Path(__file__).parent / "include"
    else:
        output_dir = Path(output_dir)

    output_dir.mkdir(exist_ok=True)
    output_file = output_dir / "fftw3.h"

    # Check if already exists
    if output_file.exists():
        print(f"FFTW header already exists at: {output_file}")
        return str(output_dir)

    # URL to download fftw3.h from official FFTW source
    url = f"http://www.fftw.org/fftw-{version}.tar.gz"

    # Alternative: Download from GitHub mirror
    github_url = f"https://raw.githubusercontent.com/FFTW/fftw3/master/api/fftw3.h"

    print(f"Downloading FFTW header file...")
    print(f"Source: {github_url}")

    try:
        with urllib.request.urlopen(github_url) as response:
            content = response.read()

        with open(output_file, 'wb') as f:
            f.write(content)

        print(f"Successfully downloaded fftw3.h to: {output_file}")
        return str(output_dir)

    except Exception as e:
        print(f"Error downloading header: {e}")
        print("\nAlternative: You can manually download fftw3.h from:")
        print(f"  https://github.com/FFTW/fftw3/blob/master/api/fftw3.h")
        print(f"  and place it in: {output_dir}/")
        return None


def main():
    """Main entry point."""
    print("=" * 70)
    print("Downloading FFTW header file")
    print("=" * 70)

    include_dir = download_fftw_header()

    if include_dir:
        print(f"\nInclude directory: {include_dir}")
        print(f"\nTo use in CMake:")
        print(f"  cmake -DFFTW_INCLUDE_DIRS={include_dir} ..")
        return 0
    else:
        return 1


if __name__ == "__main__":
    sys.exit(main())
