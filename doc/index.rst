.. Genalyzer documentation master file, created by
   sphinx-quickstart on Tue Jan 12 16:20:05 2021.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

genalyzer: Data Converter Library
=================================

Library for data converter and general sampled data analysis.


Genalyzer is a C++ library that facilitates the computation of commonly used data-converter RF performance metrics in a standards-complaint manner. Genalyzer supports generation of waveforms for characterizing data-converters as well as the computation of performance metrics given the time- or frequency-domain response of data-converters to such waveforms. In other words, users not only have the option of selecting whether or not to opt for waveform generation to characterize data-convereters, but are also able to utilize time-series data or FFT of samples captured from a data-convereter to directly compute the desired performance metrics.

Thus, Genalyzer can support a simulation-only scenario or a data-capture scenario. In the first case, the response of a data-converter is simulated given its specifications, in order to compute the expected performance metrics. In the second case, the response of a data-converter is available for Genalyzer to process in the form of a text or binary file. Genalyzer facilitates the computation of common data converter performance metrics in a standards-complaint manner. The terminology and definitions for various common performance metrics are adopted from the IEEE Standard for Terminology and Test Methods for Analog-to-Digital Converters (IEEE 1241-2010) and the IEEE Standard for Terminology and Test Methods of Digital-to-Analog Converter Devices (IEEE 1658-2011) both published by Instrumentation and Measurement/Waveform Generation, Measurement, and Analysis - Technical Committee 10 (IM/WM&A - TC10).

Reference
=========

.. toctree::
   :maxdepth: 2
   :caption: Contents:

   01_introduction
   setup
   spectral_analysis
   reference
   reference_advanced
   python/genalyzer



Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
