<p align="center">
  <img src="https://raw.githubusercontent.com/analogdevicesinc/genalyzer/main/doc/_static/genalyzer_logo.png" width="500" alt="Genalyzer">
</p>

# genalyzer

Python bindings for Genalyzer, a converter-analysis library from Analog Devices
for generating waveforms and computing data-converter performance metrics from
time-domain or frequency-domain data.

Genalyzer provides Python access to the native `libgenalyzer` engine for common
ADC and DAC analysis workflows, including waveform generation, quantization,
histogram analysis, DNL/INL analysis, FFT processing, and Fourier performance
metrics such as SNR, SINAD, SFDR, and harmonic distortion.

## Installation

For the standard PyPI install with the bundled FFTW-enabled native runtime:

```bash
python -m pip install "genalyzer[fftw]"
```

This installs:

- `genalyzer`: the Python API package.
- `genalyzer-fftw`: the GPL FFTW-enabled native runtime package containing
  `libgenalyzer` and its FFTW runtime dependencies.

If you provide your own compatible `libgenalyzer` on the system library path,
you can install only the Python API package:

```bash
python -m pip install genalyzer
```

## Quick Start

```python
import numpy as np
import genalyzer as gn

nfft = 1024
fs = 1.0e9
freq = gn.coherent(nfft, fs, 70.0e6)
samples = gn.cos(nfft, fs, 0.9, freq, 0.0, 0.0, 0.0)
codes = gn.quantize(samples, 2.0, 12, 0.0, gn.CodeFormat.TWOS_COMPLEMENT)
spectrum = gn.fft(codes, 12, 1, nfft, gn.Window.NO_WINDOW, gn.CodeFormat.TWOS_COMPLEMENT)

gn.mgr_remove("fa")
gn.fa_create("fa")
gn.fa_analysis_band("fa", "fdata*0.0", "fdata*1.0")
gn.fa_fixed_tone("fa", "A", gn.FaCompTag.SIGNAL, freq, 0)
gn.fa_fsample("fa", fs)
gn.fa_fdata("fa", fs)

results = gn.fft_analysis("fa", spectrum, nfft, gn.FreqAxisType.DC_CENTER)
print(f"SFDR: {results['sfdr']:.2f} dB")
```

## Optional Features

Additional extras are available for common workflows:

```bash
python -m pip install "genalyzer[fftw,tools]"
python -m pip install "genalyzer[fftw,pytest]"
python -m pip install "genalyzer[fftw,mcp]"
```

- `tools`: scientific helper dependencies such as SciPy.
- `pytest`: the Genalyzer pytest plugin and report-generation dependencies.
- `mcp`: the Genalyzer MCP server dependencies.
- `cli`: command-line interface dependencies.

The pytest plugin provides fixtures for attaching plots and analysis artifacts to
test reports. It is based in part on
[pytest-reporter-html1](https://github.com/christiansandberg/pytest-reporter-html1);
the related license notice is included with the package.

## Licensing

The `genalyzer` Python API package is distributed with the license expression
`LicenseRef-ADIBSD OR GPL-2.0-or-later`.

The PyPI extra `genalyzer[fftw]` installs `genalyzer-fftw`, which bundles FFTW.
FFTW is GPL unless a separate commercial FFTW license has been obtained, so the
published FFTW-enabled runtime wheels are distributed as `GPL-2.0-or-later`.

The ADI BSD source license option does not apply to the published
FFTW-inclusive runtime wheels. It is available only when `libgenalyzer` is built
with FFTW under a separate compatible commercial license, or with another FFT
implementation/license arrangement that permits the ADI BSD terms. See
`THIRD_PARTY_LICENSES.md`, shipped with each distribution, for third-party
license details.

## Links

- Documentation: https://analogdevicesinc.github.io/genalyzer/
- Source: https://github.com/analogdevicesinc/genalyzer
- Support: https://ez.analog.com/sw-interface-tools/f/q-a
