# Harmonics and Single Side Bins
In this tutorial, we look at configuring the number of _single side bins_ for various harmonic components. In the discussion on [spectral analysis](https://analogdevicesinc.github.io/genalyzer/master/spectral_analysis.html) using Genalyzer and the effect of [windowing](https://analogdevicesinc.github.io/genalyzer/master/spectral_analysis_ssb.html) on spectral analysis, we considered synthetic data for a basic overview on setting single side bins. For the purpose of illustrating how to configure them in the case of harmonic components, the focus of this tutorial, it is more interesting to consider data captured through physical hardware. 

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
The waveform we analyze in this tutorial is captured through PlutoSDR's receive port, and it is generated through the radio's direct digital synthesis (DDS). The waveform we generate will be a ``350 KHz`` complex sinusoidal tone sampled at ``4 MSPS`` and transmitted at ``1 GHz`` frequency. For this experiment, we connected the transmit and receive ports through an SMA cable with the transmit gain sufficiently backed-off as shown in the code snippet below.

With this setup, in the [spectral-analysis using PlutoSDR example](https://github.com/analogdevicesinc/genalyzer/blob/main/bindings/python/examples/gn_doc_spectral_analysis_plutosdr.py) Python script, we generate the waveform as follows:
```{code-block} python
# Create Pluto object and Configure properties
sdr = adi.Pluto(uri="ip:192.168.2.1")
sdr.rx_rf_bandwidth = 4000000
sdr.tx_lo = 1000000000
sdr.rx_lo = sdr.tx_lo
sdr.tx_cyclic_buffer = True
sdr.tx_hardwaregain_chan0 = -30 # Reduce transmit gain when PlutoSDR ports are connected by a loopback cable
sdr.gain_control_mode_chan0 = "slow_attack"
sdr.rx_buffer_size = 2**15
sdr.sample_rate = 3000000

# Sinusoidal tone settings
freq = 200000 # tone frequency
ampl_dbfs = 0.0 # amplitude of the tone in dBFS
fsr = 2.0 # full-scale range of I/Q components of the complex tone
ampl = (fsr / 2) * 10 ** (ampl_dbfs / 20) # amplitude of the tone in linear scale

# Trasmit complex sinusoidal tone using the DDS
sdr.dds_single_tone(freq, ampl)

# Get data from radio (allow a few buffer pulls for AGC to settle)
for n in range(10):
    x = sdr.rx()
```
For details on the ``adi.Pluto()`` class that is used to interface with PlutoSDR, please see the corresponding documentation page in the [PyADI-IIO](https://analogdevicesinc.github.io/pyadi-iio/devices/adi.ad936x.html#) project.

A time-domain plot of the complex-sinusoidal tone is shown below. 

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
We next compute the FFT as follows:
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



```{figure} figures/fft_pluto_A.png

FFT plot of a ``350 KHz`` complex sinusoidal tone sampled at ``4 MSPS``.
```

```{figure} figures/fft_pluto_mA.png

FFT plot of a ``350 KHz`` complex sinusoidal tone sampled at ``4 MSPS``.
```


```{figure} figures/fft_pluto_2A.png

FFT plot of a ``350 KHz`` complex sinusoidal tone sampled at ``4 MSPS``.
```


```{figure} figures/fft_pluto_m2A.png

FFT plot of a ``350 KHz`` complex sinusoidal tone sampled at ``4 MSPS``.
```


```{figure} figures/fft_pluto_3A.png

FFT plot of a ``350 KHz`` complex sinusoidal tone sampled at ``4 MSPS``.


``````{figure} figures/fft_pluto_m3A.png

FFT plot of a ``350 KHz`` complex sinusoidal tone sampled at ``4 MSPS``.
```


```{figure} figures/fft_pluto_DC.png

FFT plot of a ``350 KHz`` complex sinusoidal tone sampled at ``4 MSPS``.
```

```{figure} figures/fft_pluto_WO.png

FFT plot of a ``350 KHz`` complex sinusoidal tone sampled at ``4 MSPS``.
```