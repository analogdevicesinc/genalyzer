# Windowing and Single Side Bins
In this tutorial, we will study a common scenario that arises when spectral analysis is compromised due to _leakage_. We look at ways to remedy the situation by applying a non-rectangular window function before computing the FFT and appropriately configuring the number of single side bins within Genalyzer. The waveform we analyze will be a ``375 KHz`` complex sinusoidal tone sampled at ``4 MSPS``. The only impairment in this waveform will be quantization noise. At the end of this tutorial, the reader will have gained an understanding on how to select the number of single side bins for various _components_ after creating a _test_, in order to compute performance metrics such as SFDR, FSNR, SNR, NSD etc. This tutorial follows the discussion on [spectral analysis](#spectral-analysis). So, please read that page first to become familiar with the workflow of Genalyzer.

As before, we will follow the three-stage workflow shown below.
```{eval-rst} 
.. mermaid::

    graph LR;      
        A[Generate/Import Waveform] --> B[Compute FFT];
        B -->C1[Configure Spectral Analysis];

        subgraph SA[Spectral Analysis]
          C1[Configure] -->C2[Run];
        end

        style SA fill:#ffffff
```
We will first generate the waveform to be analyzed, compute its FFT, and calculate various performance metrics by running spectral analysis. Please refer to the [leakage and spectral-analysis example](https://github.com/analogdevicesinc/genalyzer/blob/main/bindings/python/examples/gn_doc_spectral_analysis2.py) Python script to follow the discussion on this page.

## Leakage
In the [leakage and spectral-analysis example](https://github.com/analogdevicesinc/genalyzer/blob/main/bindings/python/examples/gn_doc_spectral_analysis2.py) Python script, we generate the complex-sinusoidal waveform as follows:
```{code-block} python
# signal configuration
npts = 30000 # number of points in the signal
fs = 4e6 # sample-rate of the data
freq = 375000 # tone frequency
phase = 0.0 # tone phase
ampl_dbfs = -1.0 # amplitude of the tone in dBFS
ampl = (fsr / 2) * 10 ** (ampl_dbfs / 20) # amplitude of the tone in linear scale

# generate signal for analysis
wf = gn.cos(npts, fs, ampl, freq, phase)
awfq = gn.sin(npts, fs, ampl, freq, phase)
qwfi = gn.quantize(awfi, fsr, qres, qnoise, code_fmt)
qwfq = gn.quantize(awfq, fsr, qres, qnoise, code_fmt)
```
A time-domain plot of the complex-sinusoidal tone is shown below for reference. 

```{figure} figures/complex_sinusoidal_waveform2.png

Time-domain plot of a ``375 KHz`` complex sinusoidal tone sampled at ``4 MSPS``.
```
As shown in this plot, the snapshot of the signal is aperiodic and data is sampled over an incomplete period at the end of the data record. This discontinuity causes _leakage_ resulting in a smearing effect that spreads the energy of a tone over a wider frequency range, with the appearance of frequency components that are not present in the original signal. This is readily seen in the FFT plot of the complex-sinusoidal tone shown below. 
```{figure} figures/fft2.png

FFT plot of a ``375 KHz`` complex sinusoidal tone sampled at ``4 MSPS``.
```

### Windowing To Reduce Leakage
Note that in the FFT plot shown previously, the FFT of the time-domain waveform was computed as is with the underlying assumption that the waveform is periodic. In practice, since this assumption is generally invalid and the snapshot of a wavform contain incomplete periods rendering the signal aperiodic, a window function is applied to overcome leakage. A window function shapes the time-domain waveform such that the first and last samples are exactly zero, and the samples in between are multiplied according to a mathematical function that is symmetric around the center and tapers off away from the center. Thus, by applying a window function, an aperiodic waveform is forced to be periodic. When computing the FFT of a waveform to which a window function has been applied, a weighting factor must also be taken into account so that the correct FFT signal amplitude level is recovered after the windowing. Genalyzer takes this effect into account. 

In the [leakage and spectral-analysis example](https://github.com/analogdevicesinc/genalyzer/blob/main/bindings/python/examples/gn_doc_spectral_analysis2.py) Python script, we used Blackman-Harris window as follows:
```{code-block} python
# signal configuration
npts = 30000 # number of points in the signal
fs = 4e6 # sample-rate of the data
freq = 375000 # tone frequency
phase = 0.0 # tone phase
ampl_dbfs = -1.0 # amplitude of the tone in dBFS
qnoise_dbfs = -60.0  # quantizer noise in dBFS
fsr = 2.0 # full-scale range of I/Q components of the complex tone
ampl = (fsr / 2) * 10 ** (ampl_dbfs / 20) # amplitude of the tone in linear scale
qnoise = 10 ** (qnoise_dbfs / 20) # quantizer noise in linear scale
qres = 12  # data resolution
code_fmt = gn.CodeFormat.TWOS_COMPLEMENT # integer data format

# FFT configuration
navg = 1 # number of FFT averages
nfft = int(npts/navg) # FFT-order
window = gn.Window.BLACKMAN_HARRIS # window function to apply
axis_type = gn.FreqAxisType.DC_CENTER # axis type

# generate signal for analysis
awfi = gn.cos(npts, fs, ampl, freq, phase)
awfq = gn.sin(npts, fs, ampl, freq, phase)
qwfi = gn.quantize(awfi, fsr, qres, qnoise, code_fmt)
qwfq = gn.quantize(awfq, fsr, qres, qnoise, code_fmt)

# compute FFT
fft_cplx = gn.fft(qwfi, qwfq, qres, navg, nfft, window, code_fmt)
```
The FFT plot of the tone where Blackman-Harris window has been applied prior to computing the FFT is shown below for reference. 
```{figure} figures/fft.png

FFT plot of a ``375 KHz`` complex sinusoidal tone sampled at ``4 MSPS``.
```