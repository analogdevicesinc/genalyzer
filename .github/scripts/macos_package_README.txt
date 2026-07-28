libgenalyzer macOS package
============================

Runtime requirement
-------------------
Install the native arm64 Homebrew FFTW formula before installing this package:

    brew install fftw

The installer checks for /opt/homebrew/opt/fftw/lib/libfftw3.3.dylib and stops
with an actionable error if the runtime dependency is absent.

Install
-------

    sudo installer -pkg libgenalyzer-<version>-macos-arm64.pkg -target /

The package installs libgenalyzer, C/C++ headers, pkg-config metadata, and
license files under /usr/local.
