# Language Examples

## Fourier Analysis

In the following working example, data from an ADALM-PLUTO is used to perform a Fourier analysis as shown below. Note that in the following example, it is assumed that the data is available in the form of a json file. It can be replaced by any other means of loading this data.

````{tab} C
```C
#include "cgenalyzer.h"
#include "../tests/test_genalyzer.h"
#include <stdlib.h>

int main(int argc, const char* argv[])
{
    // read test waveform filename
    const char* test_filename = "../tests/test_vectors/test_Pluto_DDS_data_1658159639196.json";
    
    int err_code;
    int32_t *ref_qwfi, *ref_qwfq;
    double *fft_out;
    size_t results_size;
    char **rkeys;
    double *rvalues, sfdr;

    // read parameters
    tone_type ttype;
    int qres;
    unsigned long long npts, navg, nfft, tmp_win, num_tones;
    double *freq;
    GnWindow win;    
    err_code = read_scalar_from_json_file(test_filename, "wf_type", (void*)(&ttype), UINT64);
    err_code = read_scalar_from_json_file(test_filename, "qres", (void*)(&qres), INT32);
    err_code = read_scalar_from_json_file(test_filename, "npts", (void*)(&npts), UINT64);
    err_code = read_scalar_from_json_file(test_filename, "navg", (void*)(&navg), UINT64);
    err_code = read_scalar_from_json_file(test_filename, "nfft", (void*)(&nfft), UINT64);
    err_code = read_scalar_from_json_file(test_filename, "num_tones", (void*)(&num_tones), UINT64);
    freq = (double*)calloc(num_tones, sizeof(double));
    if (num_tones > 1)
        err_code = read_array_from_json_file(test_filename, "freq", freq, DOUBLE, num_tones);
    else
        err_code = read_scalar_from_json_file(test_filename, "freq", (void*)(freq), DOUBLE);
    err_code = read_scalar_from_json_file(test_filename, "win", (void*)(&tmp_win), UINT64);
    if (tmp_win==1)
        win = GnWindowBlackmanHarris;
    else if (tmp_win==2)
        win = GnWindowHann; 
    else if (tmp_win==3)
        win = GnWindowNoWindow;

    // read reference waveforms    
    ref_qwfi = (int32_t*)malloc(npts*sizeof(int32_t));
    err_code = read_array_from_json_file(test_filename, "test_vec_i", ref_qwfi, INT32, npts);
    ref_qwfq = (int32_t*)malloc(npts*sizeof(int32_t));
    err_code = read_array_from_json_file(test_filename, "test_vec_q", ref_qwfq, INT32, npts);

    // configuration
    gn_config c = NULL;
    err_code = gn_config_fftz(npts, qres, navg, nfft, win, &c);

    // FFT of waveform
    err_code = gn_fftz(&fft_out, ref_qwfi, ref_qwfq, &c);

    // Configure Fourier analysis
    err_code = gn_config_fa(freq[0], &c);
    err_code = gn_get_fa_results(&rkeys, &rvalues, &results_size, fft_out, &c);
    
    printf("\nAll Fourier Analysis Results:\n");
    for (size_t i = 0; i < results_size; i++)
        printf("%4zu%20s%20.6f\n", i, rkeys[i], rvalues[i]);
    
    err_code = gn_get_fa_single_result(&sfdr, "sfdr", fft_out, &c);
    printf("SFDR - %20.6f\n", sfdr);
    
    // free memory
    free(ref_qwfi);
    free(ref_qwfq);
    free(fft_out);
    free(rvalues);
    for (size_t i = 0; i < results_size; ++i)
        free(rkeys[i]);
    free(rkeys);
    gn_config_free(&c);
    
    return 0;
}
```
````
````{tab} Python
```python
import genalyzer, os, json, glob, pprint
import matplotlib.pyplot as plt

test_dir = os.path.join(*["..", "..", "..", "tests", "test_vectors"])
loc = os.path.dirname(__file__)

f = glob.glob(os.path.join(loc, test_dir, "test_Pluto_DDS_data_1658159639196.json"))
a = open(f[0])
data = json.load(a)
if data['num_tones'] == 1:
    freq_list = [data['freq']]
else:
    freq_list = data['freq']

qwfi = data['test_vec_i']
qwfi = [int(i) for i in qwfi]
qwfq = data['test_vec_q']
qwfq = [int(i) for i in qwfq]

# configure
c = genalyzer.config_fftz(data['npts'], data['qres'], data['navg'], data['nfft'], data['win']-1)
genalyzer.config_fa(freq_list[0], c)

# compute FFT
fft_out_i, fft_out_q = genalyzer.fftz(qwfi, qwfq, c)
fft_out = [val for pair in zip(fft_out_i, fft_out_q) for val in pair]

# get all Fourier analysis results
all_results = genalyzer.get_fa_results(fft_out, c)

# get a single Fourier analysis result
sfdr = genalyzer.get_fa_single_result("sfdr", fft_out, c)

# display results
pprint.pprint(all_results)
print('SFDR - ', sfdr)

# free memory
genalyzer.config_free(c)    
```
````

A summary of the most important results displayed and a brief explanation is as follows:

| Result Name | Description|
|:-----------:|:-----------:|
| <tt>fsnr</tt>| Full-scale Signal-to-Noise Ratio |
| <tt>snr</tt>| Signal-to-Noise Ratio |
| <tt>sinad</tt>| Signal-to-Noise and Distortion Ratio |
| <tt>sfdr</tt>| Spurious Free Dynamic Range |
| <tt>abn</tt>| Analysis Band Noise Power |
| <tt>nsd</tt>| Noise Spectral Density |
| <tt>carrierindex</tt>| Index of carrier tone |
| <tt>maxspurindex</tt>| Index of max. spur tone |
| <tt>ab_nbins</tt>| # of bins in analysis band |
| <tt>ab_rss</tt>| Analysis band received signal strength |
| <tt>hd_nbins</tt>| # of bins in used in computing harmonic distortion |
| <tt>hd_rss</tt>| Harmonic distortion band received signal strength |
| <tt>imd_nbins</tt>| # of bins in used in computing inter-modulation distortion |
| <tt>imd_rss</tt>| Inter-modulation distortion band received signal strength |
| <tt>thd_nbins</tt>| # of bins in used in computing total harmonic distortion |
| <tt>thd_rss</tt>| Total harmonic distortion band received signal strength |

Waveform Analysis
-----------------
In the following working example, genalyzer is used to generate a cosine-tone waveform, then the effect of a data-converter on this waveform is simulated and a basic waveform analysis is performed as shown below.

````{tab} C
```C
#include "cgenalyzer.h"
#include <stdlib.h>

int main(int argc, const char* argv[])
{
    // parameters
    tone_type ttype = REAL_COSINE;
    double fsr = 3.0, qnoise = pow(10.0, -60.0 / 20.0);
    int qres = 12;
    double fs = 5000000.0;
    unsigned long long npts = 8192, num_tones = 1;
    double freq[] = {50000.0}, scale[] = {0.5}, phase[] = {0.2};
    
    // waveforms
    double *awf;
    int32_t *qwf;

    // results
    size_t results_size;
    char **rkeys;
    double *rvalues;

    // configuration
    int err_code;
    gn_config c = NULL;
    err_code = gn_config_gen_tone(ttype, npts, fs, num_tones, freq, scale, phase, &c);
    err_code = gn_config_quantize(npts, fsr, qres, qnoise, &c);

    // generate waveform
    err_code = gn_gen_real_tone(&awf, &c);
    
    // quantize waveform
    err_code = gn_quantize(&qwf, awf, &c);
    
    // do waveform analysis
    err_code = gn_get_wfa_results(&rkeys, &rvalues, &results_size, qwf, &c);        

    // print results
    printf("All Waveform Analysis Results:\n");
    for (size_t i = 0; i < results_size; i++)
        printf("%4zu%20s%20.6f\n", i, rkeys[i], rvalues[i]);

    // free memory
    free(qwf);
    free(awf);
    for (size_t i = 0; i < results_size; i++)
        free(rkeys[i]);
    free(rkeys);
    gn_config_free(&c);

    return 0;
}
````
````{tab} Python
```python
import genalyzer, pprint

c = genalyzer.config_gen_tone(0, 8192, 5000000.0, 1, [50000.0], [0.5], [0.2])
genalyzer.config_quantize(8192, 3.0, 12, pow(10.0, -60.0 / 20.0), c)

awf = genalyzer.gen_real_tone(c)
qwf = genalyzer.quantize(awf, c)
wfa_results = genalyzer.get_wfa_results(qwf, c)
pprint.pprint(wfa_results)
genalyzer.config_free(c)
```
````

A summary of the results displayed and a brief explanation is as follows:

| Result Name | Description| Notes
|:-----------:|:-----------:|:-----------:|
| <tt>min</tt>| Min. value |
| <tt>max</tt>| Max. value |
| <tt>mid</tt>| Mid value |
| <tt>range</tt>| Numerical range | <tt>max-min</tt>|
| <tt>avg</tt>| Mean value | |
| <tt>rms</tt>| RMS value | |
| <tt>rmsac</tt>| RMS (AC) | Evaluated as <tt>rms<sup>2</sup>-avg<sup>2</sup></tt>|
| <tt>min_index</tt>| Index of min. value | |
| <tt>max_index</tt>| Index of max. value | |

Histogram Analysis
------------------
In the following working example, genalyzer is used to generate a ramp waveform, then the effect of a data-converter on this waveform is simulated and a basic histogram analysis is performed as shown below.

````{tab} C
```C
#include "cgenalyzer.h"
#include <stdlib.h>
#include <math.h>

int main(int argc, const char* argv[])
{
    // parameters
    double fsr = 3.0, qnoise = pow(10.0, -60.0 / 20.0);
    int qres = 12;
    unsigned long long npts = 8192, ramp_start = 0, ramp_stop = 2;
    
    // waveforms
    double *awf;
    int32_t *qwf;

    // results
    uint64_t *hist;
    size_t results_size, hist_len;
    char **rkeys;
    double *rvalues;

    // configuration
    int err_code;
    gn_config c = NULL;
    err_code = gn_config_gen_ramp(npts, ramp_start, ramp_stop, &c);
    err_code = gn_config_quantize(npts, fsr, qres, qnoise, &c);

    // generate waveform
    err_code = gn_gen_ramp(&awf, &c);
    
    // quantize waveform
    err_code = gn_quantize(&qwf, awf, &c);
    
    // compute histogram
    err_code = gn_histz(&hist, &hist_len, qwf, &c);
    
    // do waveform analysis
    err_code = gn_get_ha_results(&rkeys, &rvalues, &results_size, hist, &c);
    
    // print results
    printf("All Waveform Analysis Results:\n");
    for (size_t i = 0; i < results_size; i++)
        printf("%4zu%20s%20.6f\n", i, rkeys[i], rvalues[i]);

    // free memory
    free(qwf);
    free(awf);
    for (size_t i = 0; i < results_size; i++)
        free(rkeys[i]);
    free(rkeys);
    gn_config_free(&c);

    return 0;
}
```
````
````{tab} Python
```python
import genalyzer, pprint

c = genalyzer.config_gen_ramp(8192, 0, 2)
genalyzer.config_quantize(8192, 3.0, 12, pow(10.0, -60.0 / 20.0), c)

awf = genalyzer.gen_ramp(c)
qwf = genalyzer.quantize(awf, c)
hist = genalyzer.histz(qwf, c)
ha_results = genalyzer.get_ha_results(hist, c)
pprint.pprint(ha_results)
genalyzer.config_free(c)
```
````

A summary of the results displayed and a brief explanation is as follows:

| Result Name | Description|
|:-----------:|:-----------:|
| <tt>sum</tt>| Sum of histogram hits |
| <tt>first_nz_index</tt>| First non-zero bin index |
| <tt>last_nz_index</tt>| Last non-zero bin index |
| <tt>nz_range</tt>| Non-zero bin range|