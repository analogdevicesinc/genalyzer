<p align="center">
  <img src="https://raw.githubusercontent.com/analogdevicesinc/genalyzer/main/doc/_static/genalyzer_logo.png" width="500" alt="Genalyzer">
</p>

# genalyzer-fftw

GPL FFTW-enabled native runtime for the
[`genalyzer`](https://pypi.org/project/genalyzer/) Python package.

This package contains the platform-native `libgenalyzer` library built with FFTW
support, plus the runtime FFTW libraries needed by the wheel. It is intentionally
separate from the `genalyzer` Python API package so users can choose between the
published GPL FFTW runtime and their own compatible native build.

## Installation

Most users should install this package through the `genalyzer` extra:

```bash
python -m pip install "genalyzer[fftw]"
```

Direct installation is also supported:

```bash
python -m pip install genalyzer genalyzer-fftw
```

The `genalyzer` Python package prefers the bundled runtime from
`genalyzer-fftw`. If this package is not installed, `genalyzer` falls back to
searching the system library path for a compatible `libgenalyzer`.

## What This Package Provides

- `libgenalyzer` built for the target platform.
- Bundled FFTW runtime libraries required by `libgenalyzer`.
- Wheel metadata and third-party notices for the GPL FFTW-enabled runtime.

Published wheels are built for supported CPython and platform combinations via
`cibuildwheel`.

## Quick Smoke Test

```python
import numpy as np
import genalyzer as gn

samples = np.ones(8, dtype=np.complex128)
result = gn.fft(samples, 1, 8, gn.Window.NO_WINDOW)

np.testing.assert_allclose(result[0], 1)
np.testing.assert_allclose(result[1:], 0, atol=1e-12)
```

## Licensing

This distribution is `GPL-2.0-or-later`.

The wheels bundle FFTW. FFTW is GPL unless a separate commercial FFTW license has
been obtained, so the published FFTW-enabled runtime wheels use the project's
GPL license option. The ADI BSD source license option does not apply to these
published wheels.

See the files shipped with this distribution for details:

- `LICENSE`
- `FFTW_COPYING`
- `THIRD_PARTY_LICENSES.md`
- `FFTW_SOURCE.md`

## Links

- Python API package: https://pypi.org/project/genalyzer/
- Documentation: https://analogdevicesinc.github.io/genalyzer/
- Source: https://github.com/analogdevicesinc/genalyzer
- Support: https://ez.analog.com/sw-interface-tools/f/q-a
