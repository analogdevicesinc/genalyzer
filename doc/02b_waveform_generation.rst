Waveform Generation
-------------------
The second step, as shown previously, involves either generating the waveform and quantizing it or loading the waveform from a file. The following options are currently supported.

- cosine/sine and complex exponential waveforms of arbitrary frequency and sample rate
- ramp waveform
- Gaussian noise waveform of desired noise power spectral density level

In all three cases, the waveform generation functions take two arguments, the opaque configuration struct that has been populated in the first step and a double pointer that will contain the analog waveform generated. The subsequent call to quantize() takes the configuration struct, the analog input waveform and the output quantized waveform obtained from quantizeing the analog waveform based on the resolution contained in the configuration struct.

Alternatively, if data is loaded from a file, the configuration struct generated in the first stage will serve as a descriptor of that data. Note that indicating whether time or frequency data is loaded is indicated simply by an argument to the ``config_tone_meas()`` call in the first step.

.. literalinclude:: general_example_skelton3.c
	:language: C
