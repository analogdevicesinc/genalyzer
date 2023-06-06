Test Configuration
------------------
.. As indicated in the overall skelton C-script that interfaces with genalyzer, the first stage involves configuring an opaque struct that is provided to the users in order to describe the measurement scenario. Here, users can select between three ``config_*_meas()'' function calls in order to select between tone-, ramp- and noise-based performance metric calculations. The opaque struct maintains the state of the test scenario and is passed to the waveform generation and performance metric computation stages subsequently.

Tone-based Test Configuration
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
In tone-based test configuration, ``config_tone_meas()`` allows users to indicate test settings such as, number of tones, their frequencies, phases, scales, whether real sinusoidal or complex-exponential waveforms are to be generated, the sample-rate, full-scale range of the converter, its resolution etc. Similarly, users can indicate whether the data that will be loaded in the second stage is time-series data or interleaved FFT samples. Moreover, in data-capture scenario i.e., when FFT data is provided to Genalyzer in the second step, the tone frequency, scale and phase information is not required. This test configuration is used for measurements such as SFDR, THD, TIL etc.

An example of the ``config_tone_meas()`` function call is shown by the following example.

.. literalinclude:: general_example_tone_configuration.c
	:language: C

Ramp-based Test Configuration
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
In ramp-based test configuration, ``config_ramp_meas()`` allows users to indicate test settings such as, the starting and ending values of the ramp waveform in addition to the full-scale range and the resolution of the data converter under test. This test configuration is primarily used in non-linearity based measurements.

An example of the ``config_ramp_meas()`` function call is shown by the following example.

.. literalinclude:: general_example_ramp_configuration.c
	:language: C

Noise-based Test Configuration
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
In noise-based test configuration, ``config_noise_meas()`` allows users to indicate test settings such as, the noise power level in addition to the full-scale range and the resolution of the data converter under test. This test configuration is primarily used for computing noise spectral density. 

An example of the ``config_noise_meas()`` function call is shown by the following example.

.. literalinclude:: general_example_noise_configuration.c
	:language: C