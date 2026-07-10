# FFTW corresponding source

Release builds use FFTW source version 3.3.10 on macOS, fetched from
https://fftw.org/fftw-3.3.10.tar.gz with SHA-256
`56c932549852cddcfafdab3820b0200c7742675be92179e59e6215b340e26467`.
The build recipe is `bindings/python/tools/cibw_install_fftw_macos.sh`.
Linux and Windows currently use the versions identified by their CI build logs and recipes; release publication must retain those inputs and offer matching source alongside wheel artifacts.
