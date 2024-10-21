# Spectral Analysis
In this tutorial, we will use Genalyzer to conduct spectral analysis of a waveform. The waveform we analyze will be a ``300 KHz`` complex sinusoidal tone sampled at ``3 MSPS``. The only impairment in this waveform will be quantization noise. At the end of this tutorial, the reader will have gained an understanding on how to utilize Genalyzer to compute various RF performance metrics such as SFDR, FSNR, SNR, NSD etc. 

The workflow we follow in this tutorial is generally what is to be followed to use Genalyzer for spectral analysis. It consists of three stages shown in the graph below.

```{eval-rst} 
.. mermaid::
    :name: flowcharts1

    graph LR;      
        A[Generate/Import Waveform] --> B[Compute FFT];
        B -->C1[Configure Spectral Analysis];

        subgraph SA[Spectral Analysis]
          C1[Configure] -->C2[Run];
        end

        style SA fill:#e4d9ff
```

In this tutorial, we will generate the tone waveform mentioned previously. In another example shown here (provide link), we will import a tone waveform captured using ADALM-PLUTO and perform spectral analysis using Genalyzer.

## Tone Generation
```{eval-rst} 
.. mermaid::
    :name: flowcharts2

    graph LR;      
        A[Generate/Import Waveform] --> B[Compute FFT];
        B -->C1[Configure Spectral Analysis];

        subgraph SA[Spectral Analysis]
          C1[Configure] -->C2[Run];
        end

        style SA fill:#e4d9ff

        style A fill:#9fa4fc
```
Genalyzer supports [sine](#genalyzer.advanced.advanced.sin), [cosine](#genalyzer.advanced.advanced.cos), [ramp](#genalyzer.advanced.advanced.ramp), and [Gaussian](#genalyzer.advanced.advanced.gaussian) random waveforms. It also contains a [waveform analysis](#genalyzer.advanced.advanced.wf_analysis) utility to summarize a waveform, generated or otherwise. Please see [this](https://github.com/analogdevicesinc/genalyzer/blob/doc-update-working-example/bindings/python/examples/gn_doc_tone_gen.py) Python script for more details. A time-domain plot of the complex-sinusoidal tone is shown below for reference. 

```{figure} ../../../bindings/python/complex_sinusoidal_waveform.png

Time-domain plot of a ``300 KHz`` complex sinusoidal tone sampled at ``3 MSPS``.
```

## Compute FFT
```{eval-rst} 
.. mermaid::
    :name: flowcharts3

    graph LR;      
        A[Generate/Import Waveform] --> B[Compute FFT];
        B -->C1[Configure Spectral Analysis];

        subgraph SA[Spectral Analysis]
          C1[Configure] -->C2[Run];
        end

        style SA fill:#e4d9ff

      style B fill:#9fa4fc
```
The next step is to compute FFT of the sinusoidal tone since spectral analysis of a waveform is in essence an analysis of its FFT. 

The API for Genalyzer's ``fft()`` can be found [here](#genalyzer.advanced.advanced.fft). It supports several usecases depending on whether the samples are represented in floating- or fixed-point, and on whether the samples are represented as complex-valued, or interleaved I/Q, or split into I and Q streams.

```{note}
Genalyzer supports two fixed-point data formats: offset binary and two's-complement. See [here](#genalyzer.advanced.advanced.CodeFormat).
```
```{note}
Genalyzer supports three window types that can be applied to the signal prior to computing FFT: Blackman-Harris, Hanning, and rectangular. See [here](#genalyzer.advanced.advanced.Window).
```
```{important}
Genalyzer doesn't support an overlap window between different snapshots that are averaged to generate the FFT. As a result, ``fft()`` expects the number of complex-valued samples in the input to be equal to the number of averages times the FFT order.
```
```{seealso}
Genalyzer's ``fft()`` computes FFT for complex-valued data only. To compute FFT for real-valued data, use ``rfft()``. Additional details [here](#genalyzer.advanced.advanced.rfft).
```

The FFT plot of the complex-sinusoidal tone in our working example is shown below for reference. Please see [this](https://github.com/analogdevicesinc/genalyzer/blob/doc-update-working-example/bindings/python/examples/gn_doc_fft.py) Python script for more details. 
```{figure} ../../../bindings/python/fft.png

FFT plot of a ``300 KHz`` complex sinusoidal tone sampled at ``3 MSPS``.
```

## Run Spectral Analysis
```{eval-rst} 
.. mermaid::
    :name: flowcharts4

    graph LR;      
        A[Generate/Import Waveform] --> B[Compute FFT];
        B -->C1[Configure Spectral Analysis];

        subgraph SA[Spectral Analysis]
          C1[Configure] -->C2[Run];
        end

      style SA fill:#9fa4fc
```
Conducting spectral analysis using Genalyzer involves two steps: configuration and analysis. Please refer to [this](https://github.com/analogdevicesinc/genalyzer/blob/doc-update-working-example/bindings/python/examples/gn_doc_spectral_analysis1.py) Python script to follow the discussion in this subsection.

### Configure Genalyzer
```{eval-rst} 
.. mermaid::
    :name: flowcharts4

    graph LR;      
        A[Generate/Import Waveform] --> B[Compute FFT];
        B -->C1[Configure Spectral Analysis];

        subgraph SA[Spectral Analysis]
          C1[Configure] -->C2[Run];
        end

      style C1 fill:#9fa4fc
      style SA fill:#e4d9ff
```
We configure Genalyzer for spectral analysis by creating a _test_ followed by associating _components_ to this _test_. 
#### Create a _test_
To create a _test_, simply call [``fa_create()``](#genalyzer.advanced.advanced.fa_create) as follows:
```{code-block} python
test_label = "fa"
gn.fa_create(test_label)
```
The ``test_label`` ``string`` is key to all further configuration, and for computing and retrieving the metrics.
```{admonition} Aside
:class: dropdown

Under the hood, Genalyzer adds a key-value pair to a  ``static`` ``map`` container to manage the metrics to be computed. The key is the string argument passed through ``fa_create()``, and the mapped value is a shared-pointer to an instance of ``fourier_analysis`` class. This key is then used to further configure Genalyzer, and to compute and retrieve the metrics through ``fourier_analysis`` class. The intent behind using a ``map`` container is to be easily able to associate multiple keys to different snapshots of the data being analyzed and to have the metrics for each of those snapshots available. 
```

#### Associate signal _component_ with an existing _test_
The next step is to identify the signal tone with another label and associate it with the ``test_label`` we created earlier. To do this, simply call [``fa_max_tone()``](#genalyzer.advanced.advanced.fa_max_tone) as follows:
```{code-block} python
signal_component_label = 'A'
gn.fa_max_tone(test_label, signal_component_label, gn.FaCompTag.SIGNAL, ssb_fund)
```
As the name indicates, ``fa_max_tone()`` above interprets the tone with the highest magnitude as the signal component. For a more general way of informing Genalyzer to link a certain tone with a tag, use [``fa_fixed_tone()``](#genalyzer.advanced.advanced.fa_fixed_tone).
```{note}
The _tags_ that Genalyzer supports for Fourier analysis can be found [here](#genalyzer.advanced.advanced.FaCompTag).
```

The number of single-side bins (SSBs) for a _component_ is an important configuration step, and it will be explained in sufficient detail in another subsection. 

### Run Spectral Analysis
```{eval-rst} 
.. mermaid::
    :name: flowcharts4

    graph LR;      
        A[Generate/Import Waveform] --> B[Compute FFT];
        B -->C1[Configure Spectral Analysis];

        subgraph SA[Spectral Analysis]
          C1[Configure] -->C2[Run];
        end

      style C2 fill:#9fa4fc
      style SA fill:#e4d9ff
```

In [this](https://github.com/analogdevicesinc/genalyzer/blob/doc-update-working-example/bindings/python/examples/gn_doc_spectral_analysis1.py) Python script, FFT analysis is run by the following line:
```{code-block} python
results = gn.fft_analysis(test_label, fft_cplx, nfft, axis_type)
```
```{note}
The enumerations supported for frequency-axis type are [here](#genalyzer.advanced.advanced.FreqAxisType).
```

It prints two tables and a dictionary to the console output. With the help of each of them, we will first see how Genalyzer identifies signal tone, its harmonics (and other components), their locations, and their magnitudes. Next, we will look at the dictionary of key-value pairs that [``fft_analysis()``](#genalyzer.advanced.advanced.fft_analysis) returns which contains all the necessary information gathered by Genalyzer to compute various performance metrics. Towards the end of this discussion, we consider one metric, ``snr`` and verify how it is computed by Genalyzer.

#### Tone labels
The first table we look at is the ``labels`` table.
```{admonition} annots["labels"] from console output
:class: dropdown

``` console
+------------------+--------------------+-------------------+
|   frequency (Hz) |   magnitude (dBFs) | component label   |
+==================+====================+===================+
|                0 |          -69.177   | dc                |
+------------------+--------------------+-------------------+
|           300000 |           -1.00001 | A                 |
+------------------+--------------------+-------------------+
|          -300000 |         -102.67    | -A                |
+------------------+--------------------+-------------------+
|           600000 |         -119.239   | 2A                |
+------------------+--------------------+-------------------+
|          -600000 |          -98.8204  | -2A               |
+------------------+--------------------+-------------------+
|          -900000 |         -104.336   | -3A               |
+------------------+--------------------+-------------------+
|           401100 |          -91.4856  | wo                |
+------------------+--------------------+-------------------+
```

Notice that this table shows ``7`` frequencies, their magnitudes, and their labels. In addition to ``dc``, Genalyzer has identified ``5`` components: signal component, its image, two second-order harmonics, and one third-order harmonic along with a ``wo`` (worst-other) component. By default, Genalyzer identifies harmonics upto the `6`th order. In [this](https://github.com/analogdevicesinc/genalyzer/blob/doc-update-working-example/bindings/python/examples/gn_doc_spectral_analysis1.py) Python script, we set the the number of harmonics to take into account to ``3`` with the following line:
```{code-block} python
gn.fa_hd(test_label, num_harmonics)
```
Note that for odd-ordered harmonics, `3`rd, `5`th, and so on, Genalyzer considers only the maximum of the harmonic and its image, whereas for an even-ordered harmonic, both are taken into account.

#### Tone boxes
Next, we look at the ``tone_boxes`` table. 
```{admonition} annots["tone_boxes"] from console output
:class: dropdown

``` console
+--------------------------+--------------+
|   box left boundary (Hz) |   width (Hz) |
+==========================+==============+
|                      -50 |          100 |
+--------------------------+--------------+
|                   299950 |          100 |
+--------------------------+--------------+
|                  -300050 |          100 |
+--------------------------+--------------+
|                   599950 |          100 |
+--------------------------+--------------+
|                  -600050 |          100 |
+--------------------------+--------------+
|                  -900050 |          100 |
+--------------------------+--------------+
|                   401050 |          100 |
+--------------------------+--------------+
```

From ``annots["tone_boxes"]`` table, we see that each of the ``7`` components spans a width of ``100 Hz``. This value equals sample-rate divided by the FFT order we chose in the working example. Because the tone is coherently sampled and we chose the sample-rate to be an integer multiple of the FFT-order, a bin exactly corresponding to the frequency associated with each component is available for identification. Notice also that in [this](https://github.com/analogdevicesinc/genalyzer/blob/doc-update-working-example/bindings/python/examples/gn_doc_spectral_analysis1.py) example, we set the number of single-side bins (SSBs) for every component to ``0``. The effect of this configuration is to take into account the magnitude value corresponding to only one bin (and ``0`` bins on either side) as that component's contribution in various metrics computed. In our current working example, one bin is sufficient. In a subsequent example, we consider the case when it becomes necessary to set the number of SSBs to a value greater than ``0``.
```{caution}
The choice of the number of single-side bins (SSBs) is important when the signal is not coherently sampled and when the sample-rate is not an integer multiple of the FFT-order. 
```

#### Spectral analysis results and ``snr``
Finally, the ``results`` dictionary is shown below. Note that since a random quantization noise is added to the signal, the console output will be different.
```{admonition} results
:class: dropdown

``` console
+----------------+
results dictionary
+----------------+
{'-2A:ffinal': -600000.0,
 '-2A:freq': -600000.0,
 '-2A:fwavg': 0.0,
 '-2A:i1': 24000.0,
 '-2A:i2': 24000.0,
 '-2A:inband': 1.0,
 '-2A:mag': 3.894039999419707e-06,
 '-2A:mag_dbc': -107.19201724160015,
 '-2A:mag_dbfs': -108.19199183475817,
 '-2A:nbins': 1.0,
 '-2A:orderindex': 4.0,
 '-2A:phase': -0.15801304707963199,
 '-2A:phase_c': -0.1580081988755687,
 '-2A:tag': 2.0,
 '-3A:ffinal': -900000.0,
 '-3A:freq': -900000.0,
 '-3A:fwavg': 0.0,
 '-3A:i1': 21000.0,
 '-3A:i2': 21000.0,
 '-3A:inband': 1.0,
 '-3A:mag': 4.903305605217485e-06,
 '-3A:mag_dbc': -105.19024616511464,
 '-3A:mag_dbfs': -106.19022075827266,
 '-3A:nbins': 1.0,
 '-3A:orderindex': 5.0,
 '-3A:phase': -0.13778424236938866,
 '-3A:phase_c': -0.13777939416532536,
 '-3A:tag': 2.0,
 '-A:ffinal': -300000.0,
 '-A:freq': -300000.0,
 '-A:fwavg': 0.0,
 '-A:i1': 27000.0,
 '-A:i2': 27000.0,
 '-A:inband': 1.0,
 '-A:mag': 5.181463519759733e-06,
 '-A:mag_dbc': -104.71097651006858,
 '-A:mag_dbfs': -105.71095110322659,
 '-A:nbins': 1.0,
 '-A:orderindex': 2.0,
 '-A:phase': 2.745213082647473,
 '-A:phase_c': 2.745217930851536,
 '-A:tag': 2.0,
 '2A:ffinal': 600000.0,
 '2A:freq': 600000.0,
 '2A:fwavg': 0.0,
 '2A:i1': 6000.0,
 '2A:i2': 6000.0,
 '2A:inband': 1.0,
 '2A:mag': 4.110110243028916e-06,
 '2A:mag_dbc': -106.72295598977306,
 '2A:mag_dbfs': -107.72293058293107,
 '2A:nbins': 1.0,
 '2A:orderindex': 3.0,
 '2A:phase': -1.7377695604903687,
 '2A:phase_c': -1.7377647122863054,
 '2A:tag': 2.0,
 'A:ffinal': 300000.0,
 'A:freq': 300000.0,
 'A:fwavg': 0.0,
 'A:i1': 3000.0,
 'A:i2': 3000.0,
 'A:inband': 1.0,
 'A:mag': 0.891253545109636,
 'A:mag_dbc': 0.0,
 'A:mag_dbfs': -0.9999745931580131,
 'A:nbins': 1.0,
 'A:orderindex': 1.0,
 'A:phase': -4.848204063289533e-06,
 'A:phase_c': 0.0,
 'A:tag': 1.0,
 'ab_i1': 0.0,
 'ab_i2': 29999.0,
 'ab_nbins': 30000.0,
 'ab_rss': 0.8912547636150009,
 'ab_width': 3000000.0,
 'abn': -101.64742235889281,
 'analysistype': 1.0,
 'carrierindex': 1.0,
 'clk_nbins': 0.0,
 'clk_rss': 0.0,
 'datasize': 30000.0,
 'dc:ffinal': 0.0,
 'dc:freq': 0.0,
 'dc:fwavg': 0.0,
 'dc:i1': 0.0,
 'dc:i2': 0.0,
 'dc:inband': 1.0,
 'dc:mag': 0.0003455004004883887,
 'dc:mag_dbc': -68.23105430434262,
 'dc:mag_dbfs': -69.23102889750064,
 'dc:nbins': 1.0,
 'dc:orderindex': 0.0,
 'dc:phase': -2.351864070892235,
 'dc:phase_c': -2.351859222688172,
 'dc:tag': 0.0,
 'dist_nbins': 4.0,
 'dist_rss': 9.107498218044842e-06,
 'fbin': 100.0,
 'fdata': 3000000.0,
 'fsample': 3000000.0,
 'fshift': 0.0,
 'fsnr': 56.87707848753048,
 'hd_nbins': 4.0,
 'hd_rss': 9.107498218044842e-06,
 'ilgt_nbins': 0.0,
 'ilgt_rss': 0.0,
 'ilos_nbins': 0.0,
 'ilos_rss': 0.0,
 'ilv_nbins': 0.0,
 'ilv_rss': 0.0,
 'imd_nbins': 0.0,
 'imd_rss': 0.0,
 'maxspurindex': 6.0,
 'nad_nbins': 29998.0,
 'nad_rss': 0.001432698646864715,
 'nfft': 30000.0,
 'noise_nbins': 29994.0,
 'noise_rss': 0.0014326696989202339,
 'nsd': -121.64829103472711,
 'sfdr': 90.62403535765044,
 'signal_nbins': 1.0,
 'signal_rss': 0.891253545109636,
 'signaltype': 1.0,
 'sinad': 55.87692839257257,
 'snr': 55.87710389437247,
 'thd_nbins': 4.0,
 'thd_rss': 9.107498218044842e-06,
 'userdist_nbins': 0.0,
 'userdist_rss': 0.0,
 'wo:ffinal': 216800.0,
 'wo:freq': 216800.0,
 'wo:fwavg': 0.0,
 'wo:i1': 2168.0,
 'wo:i2': 2168.0,
 'wo:inband': 1.0,
 'wo:mag': 2.623007319313372e-05,
 'wo:mag_dbc': -90.62403535765044,
 'wo:mag_dbfs': -91.62400995080846,
 'wo:nbins': 1.0,
 'wo:orderindex': 6.0,
 'wo:phase': 1.54886652868337,
 'wo:phase_c': 1.5488713768874334,
 'wo:tag': 8.0}
```

From the results dictionary, we see a number of key-value pairs that are computed by Genalyzer. Among them, ``7`` sets of key-value pairs corresponding to the ``7`` components listed in the third column of the _annots["labels]_ table in the console output above have the following format:

``{TONEKEY}:ffinal`` : Tone final frequency (Hz)

``{TONEKEY}:freq`` : Tone frequency (Hz)

``{TONEKEY}:fwavg`` : Tone weighted-average frequency (Hz)

``{TONEKEY}:i1`` : Tone first index

``{TONEKEY}:i2`` : Tone last index

``{TONEKEY}:inband`` : 1: tone is in-band; 0: tone is out-of-band

``{TONEKEY}:mag`` : Tone magnitude

``{TONEKEY}:mag_dbc`` : Tone magnitude relative to carrier (dBc)

``{TONEKEY}:mag_dbfs`` : Tone magnitude relative to full-scale (dBFS)

``{TONEKEY}:nbins`` : Tone number of bins

``{TONEKEY}:orderindex`` : Tone order index

``{TONEKEY}:phase`` : Tone phase (rad)

``{TONEKEY}:phase_c`` : Tone phase relative to carrier (rad)

``{TONEKEY}:tag`` : Tone tag


For example, the key-value pairs in the above format for ``-A`` component are as follows:

``'-A:ffinal': -300000.0,``

``'-A:freq': -300000.0,``

``'-A:fwavg': 0.0,``

``'-A:i1': 27000.0,``

``'-A:i2': 27000.0,``

``'-A:inband': 1.0,``

``'-A:mag': 1.0513032226581935e-05,``

``'-A:mag_dbc': -98.56536672200804,``

``'-A:mag_dbfs': -99.56544008601296,``

``'-A:nbins': 1.0,``

``'-A:orderindex': 2.0,``

``'-A:phase': 2.2048368960072615,``

``'-A:phase_c': 2.2048362928010907,``

``'-A:tag': 2.0,``

which provide all the relevant information concerning ``-A`` component and its contribution to the metrics computed. More details concerning the key-value pairs in the ``results`` dictionary are available [here](#genalyzer.advanced.advanced.fft_analysis).

To illustrate how genalyzer computes ``snr``, consider the following key-value pairs:

``'noise_nbins': 29994.0,``

``'noise_rss': 0.0014326696989202339,``

which indicate that aside from ``6`` components (``A``, ``-A``, ``2A``, ``-2A``, ``-3A``, and ``wo``), all other bins have been accumulated and the root-sum-square value was computed to be ``0.0014326696989202339`` (``-56.9268 dB``). In our working example, we set the signal amplitude of the tone to be ``-1 dBFS`` (confirmed by ``'signal_rss': 0.8912434103542334``). hence, we can verify that ``SNR (dB) = 20*log10(signal_rss/noise_rss) = 55.9267469716155 dB``, which matches the value in the console output above. In a similar manner, we can verify that genalyzer computes ``SINAD (dB)`` as ``20*log10(signal_rss/nad_rss)``, ``FSNR (dB)`` as `` 20*log10(1.0/noise_rss)``, ``SFDR (dB)`` as ``20*log10(signal_rss/wo:mag)``, and ``NSD (dB)`` as ``10*log10(signal_rss*signal_rss/ab_width)``.

In summary, the magnitude spectrum plot for the working example considered so far, with DC, signal, and harmonic components labeled, is shown below.

```{figure} ../../../bindings/python/foo.png

Magnitude spectrum of the FFT showing signal, harmonic, DC, and WO components.
```
