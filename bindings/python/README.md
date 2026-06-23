# genalyzer: Python Bindings

This package contains the python bindings for genalyzer, a library for computing data-converter performance metrics.

## Binary Wheel Licensing

The default platform-wheel build links the native `libgenalyzer` library against FFTW and uses wheel repair tools to make the runtime dependencies available from the installed package. FFTW is GPL unless a separate commercial FFTW license has been obtained, so wheels built by the default cibuildwheel configuration should be treated as **GPL-2.0-or-later** distributions.

The ADI BSD license option applies only when `libgenalyzer` is built with FFTW under a separate compatible license, or with another FFT implementation/license arrangement that permits the ADI BSD terms. See `THIRD_PARTY_LICENSES.md`, shipped with the package, for the wheel-specific third-party notices.

## Optional Features

To support pytest functionality, genalyzer includes pytest fixture to help manage plots and generate reports.

This is heavily based off [pytest-reporter-html1](https://github.com/christiansandberg/pytest-reporter-html1/tree/master). See [LICENSE](https://github.com/christiansandberg/pytest-reporter-html1/blob/master/LICENSE) for related licensing information. 

## Pytest Testing (of plugin)

Tests were heavily ported as well from [pytest-reporter-html1](https://github.com/christiansandberg/pytest-reporter-html1). File headers include attribution.
