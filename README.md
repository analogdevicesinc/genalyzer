<p align="center">
<img src="https://raw.githubusercontent.com/analogdevicesinc/genalyzer/main/doc/_static/genalyzer_logo.png" width="500" alt="Genalyzer Logo"> </br>
</p>

<p align="center">
<a href="https://github.com/analogdevicesinc/genalyzer/actions">
<img src="https://github.com/analogdevicesinc/genalyzer/actions/workflows/test.yml/badge.svg" alt="Build Status">
</a>

<a href="https://github.com/analogdevicesinc/genalyzer/actions">
<img src="https://github.com/analogdevicesinc/genalyzer/actions/workflows/test-win.yml/badge.svg" alt="Build Status">
</a>

<a href="https://analogdevicesinc.github.io/genalyzer">
<img alt="Doc" src="https://img.shields.io/badge/Doc-on%20GitHub%20Pages-blue.svg">
</a>

<a href="https://ez.analog.com/sw-interface-tools/f/q-a">
<img alt="EngineerZone" src="https://img.shields.io/badge/Support-on%20EngineerZone-blue.svg">
</a>

<a href="https://wiki.analog.com/resources/tools-software/linux-software/genalyzer">
<img alt="Analog Wiki" src="https://img.shields.io/badge/Wiki-on%20wiki.analog.com-blue.svg">
</a>
</p>


# genalyzer: Library of DSP functions for RF measurements

Genalyzer is a C++ library that facilitates the computation of commonly used data-converter RF performance metrics in a standards complaint manner. Genalyzer supports generation of waveforms for characterizing data-converters as well as the computation of performance metrics given the time- or frequency-domain response of data-converters to such waveforms. In other words, users not only have the option of selecting whether or not to opt for waveform generation to characterize data-converters, but are also able to utilize time-series data or FFT of samples captured from a data-converter to directly compute the desired performance metrics.


# Licenses

Genalyzer has two license models. The first is [GPL-v2](https://github.com/analogdevicesinc/genalyzer/blob/master/LICENSE) which is often used when linking against the GPL-v2 version of FFTW. Alternatively, if you have your own FFTW license, the [ADI BSD license](https://github.com/analogdevicesinc/genalyzer/blob/master/LICENSE_ADIBSD) can be used. The provided Windows builds are GPL as they include FFTW.

## pytest Plugin

To help with testing and reporting, Genalyzer provides a pytest plugin that allows you to create and attach plots directly to your test reports. This is particularly useful for visualizing signal analysis results during automated testing.

The plugin has two main features:
1. A fixture called `gn_plot_manager` that provides methods to create and attach plots to your test reports.
2. Custom HTML report templates that integrate with pytest's reporting system to display the attached plots.

The approach is heavily based off [pytest-reporter-html1](https://github.com/christiansandberg/pytest-reporter-html1) as a reference implementation. See [LICENSE](https://github.com/christiansandberg/pytest-reporter-html1/blob/master/LICENSE) for related licensing information.

## Docs and Icons

The genalyzer documentation utilizes and provides attribution to the following as required by their licenses:
- [Font Awesome Free License for doc logo](https://fontawesome.com/license/free)
