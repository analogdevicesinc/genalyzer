# Harmonics and Single Side Bins
In this tutorial, we look at configuring the number of _single side bins_ for various harmonic components. In the discussion on [spectral analysis](https://analogdevicesinc.github.io/genalyzer/master/spectral_analysis.html) using Genalyzer and the effect of [windowing](https://analogdevicesinc.github.io/genalyzer/master/spectral_analysis_ssb.html) on spectral analysis, we considered synthetic data for a basic overview on setting the number of single side bins. For the purpose of illustrating how to configure them in the case of harmonic components, the focus of this tutorial, it is more interesting to consider data captured through physical hardware. 

Consequently, in the first stage of the three-stage workflow we have seen thus far, we will capture a waveform using [ADALM-PLUTO](https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/adalm-pluto.html). Please refer to the [spectral-analysis using PlutoSDR example](https://github.com/analogdevicesinc/genalyzer/blob/main/bindings/python/examples/gn_doc_spectral_analysis_plutosdr.py) Python script to follow the discussion on this page.
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

## Tone Generation using PlutoSDR's DDS
```{eval-rst} 
.. mermaid::

    graph LR;      
        A[Generate/Import Waveform] --> B[Compute FFT];
        B -->C1[Configure Spectral Analysis];

        subgraph SA[Spectral Analysis]
          C1[Configure] -->C2[Run];
        end

        style SA fill:#ffffff

        style A fill:#9fa4fc
```
The waveform we analyze in this tutorial is generated through PlutoSDR's direct digital synthesis (DDS) and it is captured through the radio's receive port, with the transmit and receive ports connected by an SMA cable. We will generate a ``350 KHz`` complex sinusoidal tone sampled at ``4 MSPS`` and transmit (and receive) it at ``1 GHz`` frequency. The transmit gain will be sufficiently backed-off as shown in the code snippet below. With this setup, in the [spectral-analysis using PlutoSDR example](https://github.com/analogdevicesinc/genalyzer/blob/main/bindings/python/examples/gn_doc_spectral_analysis_plutosdr.py) Python script, we generate the waveform as follows:
```{code-block} python
# Create Pluto object and Configure properties
sdr = adi.Pluto(uri="ip:192.168.2.1")
sdr.rx_rf_bandwidth = 4000000
sdr.tx_lo = 1000000000
sdr.rx_lo = sdr.tx_lo
sdr.tx_cyclic_buffer = True
sdr.tx_hardwaregain_chan0 = -30 # Reduce transmit gain since PlutoSDR ports are connected by a loopback cable
sdr.gain_control_mode_chan0 = "slow_attack"
sdr.rx_buffer_size = 2**15
sdr.sample_rate = 4000000

# Sinusoidal tone settings
freq = 350000 # tone frequency
ampl_dbfs = 0.0 # amplitude of the tone in dBFS
fsr = 2.0 # full-scale range of I/Q components of the complex tone
ampl = (fsr / 2) * 10 ** (ampl_dbfs / 20) # amplitude of the tone in linear scale

# Trasmit complex sinusoidal tone using the DDS
sdr.dds_single_tone(freq, ampl)

# Get data from radio (allow a few buffer pulls for AGC to settle)
for n in range(10):
    x = sdr.rx()
```
For details on the ``adi.Pluto()`` class that is used to interface with PlutoSDR, please see the corresponding documentation [page](https://analogdevicesinc.github.io/pyadi-iio/devices/adi.ad936x.html#) in the [PyADI-IIO](https://analogdevicesinc.github.io/pyadi-iio/) project.

A time-domain plot of the ``350 KHz`` tone is shown below. 

```{figure} figures/complex_sinusoidal_waveform3.png

Time-domain plot of a ``350 KHz`` complex sinusoidal tone sampled at ``4 MSPS``.
```

## Compute FFT
```{eval-rst} 
.. mermaid::

    graph LR;      
        A[Generate/Import Waveform] --> B[Compute FFT];
        B -->C1[Configure Spectral Analysis];

        subgraph SA[Spectral Analysis]
          C1[Configure] -->C2[Run];
        end

        style SA fill:#ffffff

      style B fill:#9fa4fc
```
We next compute the FFT for the purpose of spectral analysis as follows:
```{code-block} python
# FFT configuration
navg = 1 # number of FFT averages
nfft = int(sdr.rx_buffer_size/navg) # FFT-order
window = gn.Window.BLACKMAN_HARRIS # window function to apply
axis_type = gn.FreqAxisType.DC_CENTER # axis type
axis_fmt = gn.FreqAxisFormat.FREQ # axis-format
qres = 12 # PlutoSDR data resolution in bits
code_fmt = gn.CodeFormat.TWOS_COMPLEMENT # integer data format

# Compute FFT
x_re = np.array(x.real).astype(np.int16)
x_im = np.array(x.imag).astype(np.int16)
fft_cplx = gn.fft(x_re, x_im, qres, navg, nfft, window, code_fmt)
freq_axis = gn.freq_axis(nfft, axis_type, sdr.sample_rate, axis_fmt)
fft_db = gn.db(fft_cplx)
if gn.FreqAxisType.DC_CENTER == axis_type:
    fft_db = gn.fftshift(fft_db)
```
The FFT plot obtained as a result is shown below. 
```{figure} figures/fft_pluto.png

FFT plot of a ``350 KHz`` complex sinusoidal tone sampled at ``4 MSPS``.
```

## Spectral Analysis
### Configure Single Side Bins
```{eval-rst} 
.. mermaid::

    graph LR;      
        A[Generate/Import Waveform] --> B[Compute FFT];
        B -->C1[Configure Spectral Analysis];

        subgraph SA[Spectral Analysis]
          C1[Configure] -->C2[Run];
        end

      style C1 fill:#9fa4fc
      style SA fill:#ffffff
```
As before, due to the aperiodicity of the waveform snapshot for which we computed the FFT, there is no single bin that corresponds to the signal component and DC components. This can be seen from the two figures below.
```{figure} figures/fft_pluto_DC.png

Zoomed-in FFT plot of DC component for the working example.
```
```{figure} figures/fft_pluto_A.png

Zoomed-in FFT plot of the ``350 KHz`` complex sinusoidal tone sampled at ``4 MSPS``.
```
For this example, we can conclude that approximately ``12`` bins on either side contain the energy of the fundamental tone, whereas approximately ``8`` single side bins are sufficient for the DC component.

In a similar manner to DC and fundamental harmonic components, Genalyzer can be configured with a default number of single side bins that is applied to the rest of the harmonic components. 
```{note}
Aside from ``DC`` and ``SIGNAL``, the component categories supported in Genalyzer for configuring the number of single-side bins are ``DEFAULT`` and ``WO``. See [here](#genalyzer.FaSsb).
```
To identify the number that seems reasonable, we look at the zoomed-in plots of various other harmonic components as shown below.
```{figure} figures/fft_pluto_mA.png

Zoomed-in FFT plot of the image of ``350 KHz`` complex sinusoidal tone sampled at ``4 MSPS``.
```
```{figure} figures/fft_pluto_2A.png

Zoomed-in FFT plot of the second-order harmonic of ``350 KHz`` complex sinusoidal tone sampled at ``4 MSPS``.
```
```{figure} figures/fft_pluto_m2A.png

Zoomed-in FFT plot of the image of second-order harmonic of ``350 KHz`` complex sinusoidal tone sampled at ``4 MSPS``.
```
```{figure} figures/fft_pluto_3A.png

Zoomed-in FFT plot of the third-order harmonic of ``350 KHz`` complex sinusoidal tone sampled at ``4 MSPS``.
```
```{figure} figures/fft_pluto_m3A.png

Zoomed-in FFT plot of the image of third-order harmonic of ``350 KHz`` complex sinusoidal tone sampled at ``4 MSPS``.
```
From these plots, we conclude that setting the default number of single side bins to ``4`` is reasonable.

In [spectral-analysis using PlutoSDR example](https://github.com/analogdevicesinc/genalyzer/blob/main/bindings/python/examples/gn_doc_spectral_analysis_plutosdr.py) Python script, we create a _test_ and associate various _components_ to this _test_ as follows:
```{code-block} python
# Fourier analysis configuration
test_label = "fa"
gn.fa_create(test_label)

num_harmonics = 3 # number of harmonics to analyze
ssb_fund = 6 # number of single-side bins for the signal component
ssb_rest = 4 # default number of single-side bins for non-signal components
ssb_dc = 2 # number of single-side bins for the DC-component
signal_component_label = 'A'
gn.fa_ssb(test_label, gn.FaSsb.DEFAULT, ssb_rest)
gn.fa_max_tone(test_label, signal_component_label, gn.FaCompTag.SIGNAL, ssb_fund)
gn.fa_fsample(test_label, sdr.sample_rate)
gn.fa_hd(test_label, num_harmonics)
gn.fa_ssb(test_label, gn.FaSsb.DC, ssb_dc)

# Fourier analysis execution
results = gn.fft_analysis(test_label, fft_cplx, nfft, axis_type)
```

The results dictionary printed to the console output when the [spectral-analysis using PlutoSDR example](https://github.com/analogdevicesinc/genalyzer/blob/main/bindings/python/examples/gn_doc_spectral_analysis_plutosdr.py) Python script is run is shown below.
```{admonition} results
:class: dropdown

``` console
+----------------+
results dictionary
+----------------+
{'-2A:ffinal': -700195.3125,
 '-2A:freq': -700195.3125,
 '-2A:fwavg': 0.0,
 '-2A:i1': 27028.0,
 '-2A:i2': 27036.0,
 '-2A:inband': 1.0,
 '-2A:mag': 3.5004046508361865e-05,
 '-2A:mag_dbc': -78.30336820779617,
 '-2A:mag_dbfs': -89.11763495603948,
 '-2A:nbins': 9.0,
 '-2A:orderindex': 4.0,
 '-2A:phase': 0.0,
 '-2A:phase_c': 0.0,
 '-2A:tag': 2.0,
 '-3A:ffinal': -1050292.96875,
 '-3A:freq': -1050292.96875,
 '-3A:fwavg': 0.0,
 '-3A:i1': 24160.0,
 '-3A:i2': 24168.0,
 '-3A:inband': 1.0,
 '-3A:mag': 0.00015403857103332154,
 '-3A:mag_dbc': -65.43314362859607,
 '-3A:mag_dbfs': -76.24741037683938,
 '-3A:nbins': 9.0,
 '-3A:orderindex': 5.0,
 '-3A:phase': 0.0,
 '-3A:phase_c': 0.0,
 '-3A:tag': 2.0,
 '-A:ffinal': -350097.65625,
 '-A:freq': -350097.65625,
 '-A:fwavg': 0.0,
 '-A:i1': 29896.0,
 '-A:i2': 29904.0,
 '-A:inband': 1.0,
 '-A:mag': 0.006881691254013984,
 '-A:mag_dbc': -32.4318295683772,
 '-A:mag_dbfs': -43.24609631662052,
 '-A:nbins': 9.0,
 '-A:orderindex': 2.0,
 '-A:phase': 0.0,
 '-A:phase_c': 0.0,
 '-A:tag': 2.0,
 '2A:ffinal': 700195.3125,
 '2A:freq': 700195.3125,
 '2A:fwavg': 0.0,
 '2A:i1': 5732.0,
 '2A:i2': 5740.0,
 '2A:inband': 1.0,
 '2A:mag': 4.6145799560611906e-05,
 '2A:mag_dbc': -75.90308974502547,
 '2A:mag_dbfs': -86.71735649326878,
 '2A:nbins': 9.0,
 '2A:orderindex': 3.0,
 '2A:phase': 0.0,
 '2A:phase_c': 0.0,
 '2A:tag': 2.0,
 'A:ffinal': 350097.65625,
 'A:freq': 350097.65625,
 'A:fwavg': 0.0,
 'A:i1': 2862.0,
 'A:i2': 2874.0,
 'A:inband': 1.0,
 'A:mag': 0.2879298311683271,
 'A:mag_dbc': 0.0,
 'A:mag_dbfs': -10.814266748243316,
 'A:nbins': 13.0,
 'A:orderindex': 1.0,
 'A:phase': 0.0,
 'A:phase_c': 0.0,
 'A:tag': 1.0,
 'ab_i1': 0.0,
 'ab_i2': 32767.0,
 'ab_nbins': 32768.0,
 'ab_rss': 0.28801984170316575,
 'ab_width': 4000000.0,
 'abn': -98.67028225534297,
 'analysistype': 1.0,
 'carrierindex': 1.0,
 'clk_nbins': 0.0,
 'clk_rss': 0.0,
 'datasize': 32768.0,
 'dc:ffinal': 0.0,
 'dc:freq': 0.0,
 'dc:fwavg': 0.0,
 'dc:i1': 32766.0,
 'dc:i2': 2.0,
 'dc:inband': 1.0,
 'dc:mag': 0.00011611699835998072,
 'dc:mag_dbc': -67.88781723707687,
 'dc:mag_dbfs': -78.70208398532019,
 'dc:nbins': 5.0,
 'dc:orderindex': 0.0,
 'dc:phase': 0.0,
 'dc:phase_c': 0.0,
 'dc:tag': 0.0,
 'dist_nbins': 36.0,
 'dist_rss': 0.006883658701230589,
 'fbin': 122.0703125,
 'fdata': 4000000.0,
 'fsample': 4000000.0,
 'fshift': 0.0,
 'fsnr': 53.52294576190136,
 'hd_nbins': 36.0,
 'hd_rss': 0.006883658701230589,
 'ilgt_nbins': 0.0,
 'ilgt_rss': 0.0,
 'ilos_nbins': 0.0,
 'ilos_rss': 0.0,
 'ilv_nbins': 0.0,
 'ilv_rss': 0.0,
 'imd_nbins': 0.0,
 'imd_rss': 0.0,
 'maxspurindex': 2.0,
 'nad_nbins': 32750.0,
 'nad_rss': 0.007199170433970423,
 'nfft': 32768.0,
 'noise_nbins': 32714.0,
 'noise_rss': 0.0021079131439236884,
 'nsd': -119.54354567518098,
 'sfdr': 32.4318295683772,
 'signal_nbins': 13.0,
 'signal_rss': 0.2879298311683271,
 'signaltype': 1.0,
 'sinad': 32.040084147313706,
 'snr': 42.70867901365804,
 'thd_nbins': 36.0,
 'thd_rss': 0.006883658701230589,
 'userdist_nbins': 0.0,
 'userdist_rss': 0.0,
 'wo:ffinal': 392089.84375,
 'wo:freq': 392089.84375,
 'wo:fwavg': 0.0,
 'wo:i1': 3208.0,
 'wo:i2': 3216.0,
 'wo:inband': 1.0,
 'wo:mag': 0.0012785352416563765,
 'wo:mag_dbc': -47.05147918259395,
 'wo:mag_dbfs': -57.86574593083727,
 'wo:nbins': 9.0,
 'wo:orderindex': 6.0,
 'wo:phase': 0.0,
 'wo:phase_c': 0.0,
 'wo:tag': 8.0}
 ```

 Similarly, the magnitude spectrum plot for the PlutoSDR example considered so far, with DC, signal, and harmonic components labeled, is shown below. 

```{figure} figures/spectral_analysis_summary_pluto.png

Magnitude spectrum of the FFT for a ``350 KHz`` complex sinusoidal tone sampled at ``4 MSPS`` that is transmitted/received through PlutoSDR.
```