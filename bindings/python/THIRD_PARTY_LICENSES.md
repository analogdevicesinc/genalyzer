# Third-party license notices for genalyzer Python distributions

This file summarizes third-party components that may be included in or required
by `genalyzer` Python distributions. It is intended to ship with source and wheel
distributions so downstream users can identify the effective license obligations
for the artifact they installed.

## Optional FFTW runtime distribution

The `genalyzer` distribution contains the Python API and does not bundle FFTW.
Users can install the separately named `genalyzer-fftw` runtime with
`pip install "genalyzer[fftw]"`. That platform wheel owns the uniquely named
`genalyzer_fftw` resource package and does not overwrite files from `genalyzer`.

Because that optional runtime links against FFTW, `genalyzer-fftw` is a
**GPL-2.0-or-later** distribution and selects that license explicitly. The main
`genalyzer` API distribution retains the source project's
`LicenseRef-ADIBSD OR GPL-2.0-or-later` expression.

Do not relabel or publish `genalyzer-fftw` wheels as ADI-BSD artifacts.

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
library is built and which FFTW license applies. The separately named runtime distribution selects GPL-2.0-or-later because it
links against and bundles GPL FFTW. A commercial runtime must use another
distribution identity and must carry metadata matching its actual license.
