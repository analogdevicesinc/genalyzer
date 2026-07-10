# Third-party license notices for genalyzer Python distributions

This file summarizes third-party components that may be included in or required
by `genalyzer` Python distributions. It is intended to ship with source and wheel
distributions so downstream users can identify the effective license obligations
for the artifact they installed.

## Binary wheels built by the default cibuildwheel configuration

The default cibuildwheel configuration in this source tree builds `libgenalyzer`
against FFTW and repairs the wheel so required native runtime libraries are
available to the installed package. FFTW is distributed under the GNU General
Public License unless a separate commercial FFTW license has been obtained.

Because the default wheel build links against FFTW, binary wheels produced by
that configuration are **GPL-2.0-or-later** distributions. Their wheel metadata
selects that license explicitly. The ADI BSD license option described by this
project applies only when the native library is built with FFTW under a separate
compatible commercial license, or with another FFT implementation/license
arrangement that permits use under the ADI BSD terms.

Do not relabel or publish the CI-built FFTW-linked wheels as ADI-BSD artifacts.

## FFTW

- Project: FFTW, the Fastest Fourier Transform in the West
- Website: https://www.fftw.org/
- Source code: https://www.fftw.org/download.html
- License: GNU General Public License, version 2 or later, unless a separate
  commercial FFTW license has been obtained from the FFTW authors.

The default Linux/macOS/Windows wheel build paths install or download FFTW from
system/package-distribution sources and link `libgenalyzer` against it. Wheel
repair tools such as `auditwheel`, `delocate`, and `delvewheel` may copy FFTW
runtime libraries into the final wheel.

## pytest-reporter-html1-derived pytest report templates

- Project: pytest-reporter-html1
- Repository: https://github.com/christiansandberg/pytest-reporter-html1
- License: MIT
- Local license text: `dependencies/LICENSE_pytest-reporter-html1`

The genalyzer pytest reporting templates and related tests are based on
`pytest-reporter-html1`, as documented in the Python package README.

## genalyzer project licenses

The genalyzer source tree contains both license texts at the repository root:

- `LICENSE`: GNU General Public License, version 2
- `LICENSE_ADIBSD`: ADI BSD-style license

The repository source supports both licensing modes depending on how the native
library is built and which FFTW license applies. The Python distribution metadata
selects GPL-2.0-or-later because the default platform wheels link against and
bundle GPL FFTW. A separately produced build using the ADI BSD option must carry
different metadata and must not include GPL FFTW.
