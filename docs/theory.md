Using Genalyzer
===============
As mentioned previously, Genalyzer is a C++ library designed to support computation of RF performance metrics using either simulation data or data captured from a physical instrument. Bindings are provided to enable users to write C- or Python-based scripts to compute the desired performance metrics. In both cases, the overall structure of a C-example script that links to genalyzer library is similar, as shown in the next section. The overall structure in a simulation-only scenario consists of three stages:

* Test Configuration
* Waveform Generation
* Metric Computation

In the data-capture scenario, where the response of a data-converter is available in the form of text files, the structure would be 

* Test Configuration
* Waveform Loading
* Metric Computation

That is, only the second step in the procedure involved would change. The overall structure of a C-example that utilizes genalyzer library in the simulation-only scenario is shown by the following example.
``` c
#include "cgenalyzer.h"

int main(int argc, char *argv[]) {  
  // opaque config struct that will contain config settings
  // config c = NULL;

  /* configuration */
  // config_tone_meas(...);

  /* waveform generation and quantize */
  // gen_tone(...);
  // quantize(...);

  /* compute metrics */
  // fsnr_val = metric(..., "FSNR");

  return 0;
}
```
Similarly, the overall structure of a C-example in the data-capture scenario is shown by the following example.
``` c
#include "cgenalyzer.h"

int main(int argc, char *argv[]) {  
  // opaque config struct that will contain config settings
  // config c = NULL;

  /* configuration */
  // config_tone_meas(...);

  /* load waveform generation*/
  // read_file_to_array(...);

  /* compute metrics */
  // fsnr_val = metric(..., "FSNR");

  return 0;
}
```

Test Configuration
------------------
As indicated in the overall skelton C-script that interfaces with genalyzer, the first stage involves configuring an opaque struct that is provided to the users in order to describe the measurement scenario. Here, users can select between three `config_*_meas()` function calls in order to select between tone-, ramp- and noise-based performance metric calculations. The opaque struct maintains the state of the test scenario and is passed to the waveform generation and performance metric computation stages subsequently.

**Tone-based Test Configuration**<br />
In tone-based test configuration, `config_tone_meas()` allows users to indicate test settings such as, number of tones, their frequencies, phases, scales, whether real sinusoidal or complex-exponential waveforms are to be generated, the sample-rate, full-scale range of the converter, its resolution etc. Similarly, users can indicate whether the data that will be loaded in the second stage is time-series data or interleaved FFT samples. Moreover, in data-capture scenario i.e., when FFT data is provided to Genalyzer in the second step, the tone frequency, scale and phase information is not required. This test configuration is used for measurements such as SFDR, THD, TIL etc.

An example of the `config_tone_meas()` function call is shown by the following example.
``` c
// configuration
config_tone_meas(&c,
				domain_wf,
				type_wf,
				nfft, // FFT order
				navg, // # of FFTs averaged
				fs, // sample rate
				fsr, // full-scale range
				0, // ADC resolution: unused configuration setting
				freq, // tone frequency, # of array elements = num_tones
				scale, // tone scale, # of array elements = num_tones
				phase, // tone phase, # of array elements = num_tones
				num_tones, // # of tones
				false,
				false,
				false
				);
```

**Ramp-based Test Configuration**<br />
In ramp-based test configuration, `config_ramp_meas()` allows users to indicate test settings such as, the starting and ending values of the ramp waveform in addition to the full-scale range and the resolution of the data converter under test. This test configuration is primarily used in non-linearity based measurements.

An example of the `config_ramp_meas()` function call is shown by the following example.
``` c
// configuration
config_ramp_nl_meas(&c,
					npts, // # of data points
					fs, // sample rate
					fsr, // full-scale range
					res, // ADC resolution: unused configuration setting
					start,
					stop,
					0.0
					);
```

**Noise-based Test Configuration**<br/>
In noise-based test configuration, `config_noise_meas()` allows users to indicate test settings such as, the noise power level in addition to the full-scale range and the resolution of the data converter under test. This test configuration is primarily used for computing noise spectral density. 

An example of the `config_noise_meas()` function call is shown by the following example.
``` c
// configuration
config_noise_meas(&c,
				COMPLEX_NOISE,
				nfft,
				navg,
				fs,
				fsr,
				res,
				noise_pwr_lvl,
				update_fsample,
				update_fdata,
				update_fshift);
```

Waveform Generation
-------------------
The second step, as shown previously, involves either generating the waveform and quantizing it or loading the waveform from a file. The following options are currently supported.

- cosine/sine and complex exponential waveforms of arbitrary frequency and sample rate
- ramp waveform
- Gaussian noise waveform of desired noise power spectral density level

In all three cases, the waveform generation functions take two arguments, the opaque configuration struct that has been populated in the first step and a double pointer that will contain the analog waveform generated. The subsequent call to `quantize()` takes the configuration struct, the analog input waveform and the output quantized waveform obtained from quantizeing the analog waveform based on the resolution contained in the configuration struct.

Alternatively, if data is loaded from a file, the configuration struct generated in the first stage will serve as a descriptor of that data. Note that indicating whether time or frequency data is loaded is indicated simply by an argument to the `config_tone_meas()` call in the first step.
``` c
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include "genalyzer_cwrapper.h"

int main(int argc, char *argv[]) {  
  // opaque config struct that will contain config settings
  // config c = NULL;

  /* configuration */
  // config_tone_meas(&c, FREQ, ...);
  // config_tone_meas(&c, TIME,...);

  /* load waveform generation*/
  // read_file_to_array(...);

  /* compute metrics */
  // fsnr_val = metric(..., "FSNR");

  return 0;
}
```

Performance Metric Computation
------------------------------
The final stage involves simply computing the desired performance metric. Metric computation is done by a call to ``metric()`` where, the last argument is one of: 

  * `"FSNR"`
  * `"NSD"`
  * `"SFDR"`
  * `"SINAD"`
  * `"SNR"`
  * `"TD"`
  * `"THD"`
  * `"TIL`

Again, the test configuration contained in the opaque struct obtained from the first step is helpful in determinging whether the 