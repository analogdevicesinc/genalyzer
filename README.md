# genalyzer: Library of DSP functions for RF measurements

Genalyzer is a C++ library that facilitates the computation of commonly used data-converter RF performance metrics in a standards complaint manner. Genalyzer supports generation of waveforms for characterizing data-converters as well as the computation of performance metrics given the time- or frequency-domain response of data-converters to such waveforms. In other words, users not only have the option of selecting whether or not to opt for waveform generation to characterize data-converters, but are also able to utilize time-series data or FFT of samples captured from a data-converter to directly compute the desired performance metrics.


# Licenses

Genalyzer has two license models. The first is [GPL-v2](https://github.com/analogdevicesinc/genalyzer/blob/master/LICENSE) which is often used when linking against the GPL-v2 version of FFTW. Alternatively, if you have your own FFTW license, the [ADI BSD license](https://github.com/analogdevicesinc/genalyzer/blob/master/LICENSE_ADIBSD) can be used. The provided Windows builds are GPL as they include FFTW.

## Docs and Icons

The genalyzer documentation utilizes and provides attribution to the following as required by their licenses:
- [Font Awesome Free License for doc logo](https://fontawesome.com/license/free)
