# genalyzer: Python Bindings

This package contains the python bindings for genalyzer, a library for computing data-converter performance metrics.

## FFTW Binary Wheel Variants and Licensing

The platform-wheel variants built for supported Python and operating-system combinations link the native `libgenalyzer` library against FFTW and use wheel repair tools to bundle the required FFTW runtime. FFTW is GPL unless a separate commercial FFTW license has been obtained. The published FFTW-inclusive wheels therefore select the project's **GPL-2.0-or-later** license option, and their package metadata reports that license explicitly.

The ADI BSD source license option does **not** apply to these wheels. It is available only when `libgenalyzer` is built with FFTW under a separate compatible commercial license, or with another FFT implementation/license arrangement that permits the ADI BSD terms. Such a non-GPL build must be distributed separately with matching metadata; it must not reuse the FFTW-wheel artifacts produced by this repository's CI. See `THIRD_PARTY_LICENSES.md`, shipped with each distribution, for the wheel-specific third-party notices.

## Optional Features

To support pytest functionality, genalyzer includes pytest fixture to help manage plots and generate reports.

This is heavily based off [pytest-reporter-html1](https://github.com/christiansandberg/pytest-reporter-html1/tree/master). See [LICENSE](https://github.com/christiansandberg/pytest-reporter-html1/blob/master/LICENSE) for related licensing information. 

## Pytest Testing (of plugin)

Tests were heavily ported as well from [pytest-reporter-html1](https://github.com/christiansandberg/pytest-reporter-html1). File headers include attribution.
