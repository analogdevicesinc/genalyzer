Usage Model
===========
Overview
--------
Within Genalyzer, bindings to the C++ library are provided to enable users to write C-, Python-, or MATLAB-based scripts to compute the desired performance metrics. In all three cases, the overall structure of a script that links to genalyzer is similar, as shown in more detail in the next subsection. The overall structure in a simulation-only scenario consists of three stages:

* Configure test
* Generate waveform
* Compute metric

In the scenario where either the ADC codes or the FFT of the captured ADC codes are available, the structure would be 

* Configure test
* Load waveform
* Compute metric

In other words, only the second step would change. 

The overall structure of a C-example that utilizes genalyzer library in the first scenario is shown by the following example.
``` c
#include "cgenalyzer.h"

int main(int argc, char *argv[]) {  
  // opaque config struct that will contain config settings
  gn_config_quantize_struct c = NULL;  

  // configure test
  err_code = gn_config_calloc(&c);
  err_code = gn_config_gen_tone(..., c);
  err_code = gn_config_quantize(..., c);
  
  // generate waveform and quantize
  err_code = gn_gen_real_tone(..., c);
  err_code = gn_quantize(..., c);

  // compute metrics
  err_code = gn_get_fa_single_result(&sfdr, "sfdr", ..., c);

  // free memory
  gn_config_free(c);
  
  return 0;
}
```
As shown in this (incomplete) illustrative example, a configuration struct is passed to various stages without having to pass the same relevant parameters over and over. 

The overall structure of a C-example in the second scenario is shown by the following example.
``` c
#include "cgenalyzer.h"

int main(int argc, char *argv[]) {  
  // opaque config struct that will contain config settings
  gn_config_quantize_struct c = NULL;  

  // configure test
  err_code = gn_config_calloc(&c);
  err_code = gn_config_fft(..., c);
  
  // read ADC codes from file
  err_code = read_array_from_json_file(filename, "adc_output_i", adc_i, ...);
  err_code = read_array_from_json_file(filename, "adc_output_q", adc_q, ...);

  // compute FFT
  err_code = gn_fftz(&fft_out, adc_i, adc_q, c);

  // compute metrics
  err_code = gn_get_fa_single_result(&sfdr, "sfdr", fft_out, c);

  // free memory
  gn_config_free(c);
  
  return 0;
}
```
If on the other hand, the FFT is pre-computed and genalyzer is expected to compute the desired performance metric alone, then only the configuration of FFT related parameters is needed in order for genalyzer to compute the desired performance metric. Note that in the following example, it is assumed that the FFT data is available in the form of a json file. It can be replaced by any other means of loading this data in ASCII or binary form.
``` c
#include "cgenalyzer.h"

int main(int argc, char *argv[]) {  
  // opaque config struct that will contain config settings
  gn_config_quantize_struct c = NULL;  

  // configuration
  err_code = gn_config_calloc(&c);
  err_code = gn_config_fft(..., c);
  
  // read FFT of ADC codes from file
  err_code = read_array_from_json_file(filename, "fft_test_vec", fft_out, ...);

  // compute metrics
  err_code = gn_get_fa_single_result(&sfdr, "sfdr", fft_out, c);

  // free memory
  gn_config_free(c);
  
  return 0;
}
```
As highlighted by all three illustrative examples above, prior to configuring the test scenario, memory is allocated for the configuration struct by invoking, <tt>gn_config_calloc()</tt> and to free the allocated memory, <tt>gn_config_free()</tt> is called.

Test Configuration
------------------

As indicated previously, test configuration is driven through an opaque struct. The public members that are accessible to the user for the purpose of specifying either the waveform or the FFT or the analysis settings are shown in the table below. 

| Catgory | Member Name | Datatype | Description | Notes |
|:-------:|:-----------:|:--------:|:-----------:|:-----:|
| **Waveform and FFT settings** | <tt>ttype</tt>      | ENUM | Tone type | Options:<br/> <tt>REAL_COSINE</tt><br/> <tt>REAL_SINE</tt><br/> <tt>COMPLEX_EXP</tt> |
|  | <tt>npts</tt>      | size_t | Num. of sample points in the waveform | |
|  | <tt>sample_rate</tt> | double | Sample Rate |  |
|  | <tt>tone_freq</tt> | double* | Tone frequency |  |
|  | <tt>tone_ampl</tt> | double* | Tone amplitude |  |
|  | <tt>tone_phase</tt> | double* | Tone phase |  |
|  | <tt>num_tones</tt> | size_t | Num. of tones |  |
|  | <tt>fsr</tt> | double | ADC full-scale range |  |
|  | <tt>qres</tt> | int | Quantization resolution |  |
|  | <tt>noise_rms</tt> | double | Noise RMS value |  |
|  | <tt>code_format</tt> | ENUM | Code format of ADC/DAC codes | Options: <br/><tt>GnCodeFormatOffsetBinary</tt><br/> <tt>GnCodeFormatTwosComplement</tt> |
|  | <tt>nfft</tt> | size_t | FFT order | |
|  | <tt>fft_navg</tt> | size_t | Num. of FFT averages | |
|  | <tt>data_rate</tt> | double | Data rate | |
|  | <tt>shift_freq</tt> | double | Shift frequency | |
|  | <tt>win</tt> | ENUM | Window function | Options: <br/><tt>GnWindowBlackmanHarris</tt><br/> <tt>GnWindowHann</tt><br/> <tt>GnWindowNoWindow</tt> |
|  | <tt>ramp_start</tt> | double | Start value of ramp | |
|  | <tt>ramp_stop</tt> | double | Stop value of ramp | |
| **Fourier analysis settings** | <tt>obj_key</tt> | char* | Object key | |
|  | <tt>comp_key</tt> | char* | Component key | |
|  | <tt>ssb_fund</tt> | int | Single side bin - Fundamental | |
|  | <tt>ssb_rest</tt> | int | Single side bin - Others | |
|  | <tt>max_harm_order</tt> | int | Max. order of harmonic      | |
| **NL analysis settings** | <tt>dnla_signal_type</tt> | ENUM | DNL analysis signal type | Options: <br/><tt>GnDnlSignalRamp</tt><br/> <tt>GnDnlSignalTone</tt> |
|  | <tt>inla_fit</tt> | ENUM | INL analysis fit      | Options: <br/><tt>GnInlLineFitBestFit</tt> <br/><tt>GnInlLineFitEndFit</tt> <br/><tt>GnInlLineFitNoFit</tt> |

As shown by this table, not all parameters are necessary in all use-cases. Consequently, each of the above <tt>struct</tt> members are accessible through <tt>gn_config_set_*</tt> functions. For instance, in order to configure sample rate, the function, <tt>gn_config_set_sample_rate</tt> would be used. Similarly for the other struct members. Additionally, the functions, <tt>gn_config_gen_tone</tt>, <tt>gn_config_gen_ramp</tt>, <tt>gn_config_quantize</tt>, <tt>gn_config_fft</tt>, and <tt>gn_config_fa</tt> can be used to configure a set of parameters that are needed in certain specific use-cases. Further details on the function signatures for each of these configuration functions can be found on the API reference page.

The following table describes a few use-cases and the relevant members of the configuration struct that need to be set in order to generate a waveform or to perform the desired analysis using genalyzer.

| Use-case | <tt>gn_config</tt> members to configure | Pre-requite function call | Function call |
|:-------:|:-----------|:-----------:|:-----------:|
|  Generate single-/multi-tone waveform | <ul><li><tt>ttype</tt></li><li><tt>npts</tt></li><li><tt>sample_rate</tt></li><li><tt>num_tones</tt></li><li><tt>tone_freq</tt></li><li><tt>tone_ampl</tt></li><li><tt>tone_phase</tt></li></ul>  | <tt>gn_config_gen_tone()</tt>| <tt>gn_gen_tone()</tt>|
|  Generate ramp waveform | <ul><li><tt>npts</tt></li><li><tt>ramp_start</tt></li><li><tt>ramp_stop</tt></li></ul> | <tt>gn_config_gen_ramp()</tt><sup>1</sup> | <tt>gn_gen_ramp()</tt>|
|  Quantize waveform | <ul><li><tt>npts</tt></li><li><tt>fsr</tt></li><li><tt>qres</tt></li><li><tt>noise_rms</tt></li></ul> | <tt>gn_config_quantize()</tt><sup>2</sup> | <tt>gn_quantize()</tt>|
|  Compute FFT | <ul><li><tt>npts</tt></li><li><tt>qres</tt></li><li><tt>fft_navg</tt></li><li><tt>nfft</tt></li><li><tt>win</tt></li></ul> | <tt>gn_config_fft()</tt><sup>2</sup> | <tt>gn_fftz()</tt>|
|  Compute histogram | <ul><li><tt>npts</tt></li><li><tt>qres</tt></li></ul> | <tt>gn_config_histz_nla()</tt><sup>2</sup> | <tt>gn_histz()</tt>|
|  Compute DNL | <ul><li><tt>npts</tt></li><li><tt>qres</tt></li></ul> | <tt>gn_config_histz_nla()</tt><sup>2</sup> | <tt>gn_dnlz()</tt>|
|  Compute INL | <ul><li><tt>npts</tt></li><li><tt>qres</tt></li></ul> | <tt>gn_config_histz_nla()</tt><sup>2,</sup><sup>3</sup> | <tt>gn_inlz()</tt>|
|  Do Fourier analysis | <ul><li><tt>nfft</tt></li></ul>  | <tt>gn_config_fa()</tt><sup>4</sup> |  <tt>gn_get_fa_single_result()</tt> <br/>or<br/> <tt>gn_get_fa_results()</tt> |
|  Do waveform analysis | <ul><li><tt>npts</tt></li></ul>  | <tt>gn_config_set_npts()</tt> |  <tt>gn_get_wfa_results()</tt> |
|  Do histogram analysis | <ul><li><tt>npts</tt></li><li><tt>qres</tt></li></ul> | <tt>gn_config_histz_nla()</tt><sup>2</sup> |  <tt>gn_get_ha_results()</tt> |
|  Do DNL analysis | <ul><li><tt>npts</tt></li><li><tt>qres</tt></li></ul> | <tt>gn_config_histz_nla()</tt><sup>2</sup> |  <tt>gn_get_dnla_results()</tt> |
|  Do INL analysis | <ul><li><tt>npts</tt></li><li><tt>qres</tt></li></ul> | <tt>gn_config_histz_nla()</tt><sup>2</sup> |  <tt>gn_get_inla_results()</tt> |

<sup>1</sup><tt>noise_rms=0.0</tt> is used. It can be overridden by calling <tt>gn_config_set_noise_rms()</tt> prior to calling <tt>gn_gen_ramp()</tt>.

<sup>2</sup><tt>code_format=GnCodeFormatTwosComplement</tt> is used. It can be overridden by calling <tt>gn_config_set_code_format()</tt> prior to calling the corresponding pre-requisite function call.

<sup>3</sup><tt>inla_fit=GnInlLineFitBestFit</tt> is used. It can be overridden by calling <tt>gn_config_set_inla_fit()</tt> prior to calling <tt>gn_config_histz_nla()</tt>.

<sup>4</sup>The following default settings are used.<ul><li><tt>ssb_fund = 0</tt></li><li><tt>ssb_rest = 0</tt></li><li><tt>max_harm_order = 3</tt></li><li><tt>axis_type=GnFreqAxisTypeDcCenter</tt></li></ul> They can be overridden by calling <tt>gn_config_set_ssb_fund()</tt>, <tt>gn_config_set_ssb_rest()</tt>, <tt>gn_config_set_max_harm_order()</tt>, <tt>gn_config_set_axis_type()</tt> prior to calling <tt>gn_get_fa_results()</tt> or <tt>gn_get_fa_single_result()</tt>.

Note that for certain use-cases, the pre-requisite function calls need not be invoked. For instance, given an analog waveform, if the user intends to quantize it and then compute the histogram, calling <tt>gn_config_quantize()</tt> will set the <tt>gn_config</tt> struct parameters needed prior to invoking <tt>gn_histz()</tt>.

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