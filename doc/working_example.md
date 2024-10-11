# Genalyzer Through Examples
## A Minimal Working Example
The workflow of Genalyzer is best understood with the help of a working example. We use Python to illustrate this workflow in the current section. The data that we will process using Genalyzer for this example is also generated using Genalyzer. It consists of a ``300 KHz`` tone sampled at ``3 MSPS``. The only source of noise in this synthetic data is quantization noise.
<br/><br/>

### Compute FFT
We begin with the initial steps involved, _i.e.,_, configuring Genalyzer and performing FFT on the generated data. Since spectral analysis is the core of Genalyzer, computing FFT is the first important step. Expand the code-snippet below for details.
<details>
  <summary>Code Snippet</summary>

``` Python
def main():
    import genalyzer.advanced as gn

    #
    # Signal 
    #
    npts = 30000 # number of points in the signal
    freq = 300000 # tone frequency
    phase = 0.0 # tone phase
    ampl_dbfs = -1.0 # amplitude of the tone in dBFS
    qnoise_dbfs = -63.0  # quantizer noise in dBFS
    fsr = 2.0 # full-scale range of I/Q components of the complex tone
    ampl = (fsr / 2) * 10 ** (ampl_dbfs / 20) # amplitude of the tone in linear scale
    qnoise = 10 ** (qnoise_dbfs / 20) # quantizer noise in linear scale
    qres = 12  # data resolution
    code_fmt = gn.CodeFormat.TWOS_COMPLEMENT # integer data format
    fs = 3e6 # sample-rate of the data

    #
    # FFT configuration
    #
    navg = 1 # number of FFT averages
    nfft = int(npts/navg) # FFT-order
    window = gn.Window.NO_WINDOW # window function to apply
    
    #
    # Genarate signal for analysis
    #
    awfi = gn.cos(npts, fs, ampl, freq, phase)
    awfq = gn.sin(npts, fs, ampl, freq, phase)
    qwfi = gn.quantize(awfi, fsr, qres, qnoise, code_fmt)
    qwfq = gn.quantize(awfq, fsr, qres, qnoise, code_fmt)
    
    #
    # Compute FFT
    #
    fft_cplx = gn.fft(qwfi, qwfq, qres, navg, nfft, window, code_fmt)

if __name__ == "__main__":
    main()
```
</details>

We will expand this code snippet to illustrate the various capabilities of Genalyzer. We postpone the discussion on how to generate synthetic data using Genalyzer and the related knobs available. We begin first with the FFT-computing step in the above snippet.

The arguments that ``fft()`` takes are shown on [here](#genalyzer.advanced.advanced.fft). As shown on this page, it takes a variable number of arguments to handle several usecases. The usecases are split based on how the I/Q samples are represented, _i.e._, whether the samples are complex-valued, or interleaved I/Q, or split into I and Q separately. These categories are further split based on whether the samples are represented as floating-point or fixed-point integer data. In the latter case, the resolution in bits and code-format are the required inputs. The supported datatypes for the input data vector are: ``complex128``, ``float64``, ``int16``, ``int32``, ``int64``. 

```{note}
Genalyzer supports two code formats: offset binary and two's-complement. The enumerations that map to these formats are shown [here](#genalyzer.advanced.advanced.CodeFormat).
```
```{note}
Genalyzer supports three window types that can be applied to the signal prior to computing FFT: Blackman-Harris, Hanning, and rectangular. The enumerations that map to these window types are shown [here](#genalyzer.advanced.advanced.Window).
```
```{important}
Genalyzer doesn't support an overlap window between different snapshots that are averaged to generate the FFT. As a result, ``fft()`` expects the number of complex-valued samples in the input to be equal to the number of averages times the FFT order.
```
```{seealso}
Genalyzer's ``fft()`` computes FFT for complex-valued data only. To compute FFT for real-valued data, use ``rfft()``. Additional details [here](#genalyzer.advanced.advanced.rfft).
```
<br/><br/>

### Conduct Spectral Analysis
Before conducting any spectral analysis, we configure a _manager_ to store _keys_ (more details follow in the subsections below). Only then, we run spectral analysis to calculate RF performance metrics. 
#### Configure Genalyzer
The following expanded code-snippet shows how Genalyzer is configured.
<details>
  <summary>Code Snippet</summary>

``` Python
def main():
    import genalyzer.advanced as gn
    
    #
    # Signal 
    #
    npts = 30000 # number of points in the signal
    freq = 300000 # tone frequency
    phase = 0.0 # tone phase
    ampl_dbfs = -1.0 # amplitude of the tone in dBFS
    qnoise_dbfs = -63.0  # quantizer noise in dBFS
    fsr = 2.0 # full-scale range of I/Q components of the complex tone
    ampl = (fsr / 2) * 10 ** (ampl_dbfs / 20) # amplitude of the tone in linear scale
    qnoise = 10 ** (qnoise_dbfs / 20) # quantizer noise in linear scale
    qres = 12  # data resolution
    code_fmt = gn.CodeFormat.TWOS_COMPLEMENT # integer data format
    fs = 3e6 # sample-rate of the data

    #
    # FFT configuration
    #
    navg = 1 # number of FFT averages
    nfft = int(npts/navg) # FFT-order
    qres = 12  # data resolution
    code_fmt = gn.CodeFormat.TWOS_COMPLEMENT # integer data format
    window = gn.Window.NO_WINDOW # window function to apply
    
    #
    # Genarate signal for analysis
    #
    awfi = gn.cos(npts, fs, ampl, freq, phase)
    awfq = gn.sin(npts, fs, ampl, freq, phase)
    qwfi = gn.quantize(awfi, fsr, qres, qnoise, code_fmt)
    qwfq = gn.quantize(awfq, fsr, qres, qnoise, code_fmt)
    
    #
    # Compute FFT
    #
    fft_cplx = gn.fft(qwfi, qwfq, qres, navg, nfft, window, code_fmt)

    #
    # Fourier analysis configuration
    #
    object_key = "fa" # object-key
    signal_tone_key = 'A' # signal-key
    ssb_fund = 0 # number of single-side bins
    gn.fa_create(object_key)
    gn.fa_fsample(object_key, fs)
    gn.fa_max_tone(object_key, signal_tone_key, gn.FaCompTag.SIGNAL, ssb_fund)

if __name__ == "__main__":
    main()
```
</details>

##### Create ``fourier_analysis`` Object
The first step in configuring Genalyzer is to call the ``fa_create()`` function with an _object\_key_. This key is used for all configuring Genalyzer further, and for computing and retrieving the metrics.
```{admonition} Aside
:class: note

Under the hood, Genalyzer adds a key-value pair to a  ``static`` ``map`` container to manage the metrics to be computed. The key is the string argument passed through ``fa_create()``, and the mapped value is a shared-pointer to an instance of ``fourier_analysis`` class. This key is then used to further configure Genalyzer, and to compute and retrieve the metrics through ``fourier_analysis`` class. The intent behind using a ``map`` container is to be easily able to associate multiple keys to different snapshots of the data being analyzed and to have the metrics for each of those snapshots available. 
```

##### Set Sample-Rate
Set the sample-rate of the data vector using ``fa_fsample``. As shown on this [page](#genalyzer.advanced.advanced.fa_fsample), we associate this configuration step with an _object\_key_.
<br/><br/>

##### Identify Signal Tone
The next step is to identify the signal tone and label it. This is done by calling either [``fa_fixed_tone()``](#genalyzer.advanced.advanced.fa_fixed_tone) or [``fa_max_tone()``](#genalyzer.advanced.advanced.fa_max_tone) functions. In both cases, the first two arguments are _keys_. The first _key_ is the _object\_key_ mentioned previously, and the second _key_ is a string that is to be associated with the signal component whose magnitude is measured and used subsequently when computing performance metrics. In both cases, the third argument is a tag which maps to the component type. To identify a component as the signal tone, we use ``GnFACompTagSignal``. The other tags available are shown [here](#genalyzer.advanced.advanced.FaCompTag). As the name indicates, ``fa_max_tone()`` interprets the tone with the highest magnitude as the signal tone.

```{note}
Since ``fa_fixed_tone()`` requires that the frequency of a desired tone is provided as an argument, it is useful for taking into account any desired tone that is pertinent to analysis.
```

The number of single-side bins (SSBs) for the signal component is an important configuration step, and it will be explained in sufficient detail in another subsection. 
<br/><br/>
#### Run FFT Analysis
##### Signal Component
Although we need to configure a few more settings to get meaningful results, we'll run spectral analysis using [``fft_analysis()``](#genalyzer.advanced.advanced.fft_analysis) and use Genalyzer's Fourier analysis annotation utility to understand a couple of points. See the expanded code-snippet below.
<details>
  <summary>Code Snippet</summary>

``` Python
def main():
    import numpy as np
    import genalyzer.advanced as gn
    import pprint
    import matplotlib.pyplot as pl
    from matplotlib.patches import Rectangle as MPRect
    from tabulate import tabulate

    #
    # Signal 
    #
    npts = 30000 # number of points in the signal
    freq = 300000 # tone frequency
    phase = 0.0 # tone phase
    ampl_dbfs = -1.0 # amplitude of the tone in dBFS
    qnoise_dbfs = -63.0  # quantizer noise in dBFS
    fsr = 2.0 # full-scale range of I/Q components of the complex tone
    ampl = (fsr / 2) * 10 ** (ampl_dbfs / 20) # amplitude of the tone in linear scale
    qnoise = 10 ** (qnoise_dbfs / 20) # quantizer noise in linear scale
    qres = 12  # data resolution
    code_fmt = gn.CodeFormat.TWOS_COMPLEMENT # integer data format
    fs = 3e6 # sample-rate of the data

    #
    # FFT configuration
    #
    navg = 1 # number of FFT averages
    nfft = int(npts/navg) # FFT-order
    qres = 12  # data resolution
    code_fmt = gn.CodeFormat.TWOS_COMPLEMENT # integer data format
    window = gn.Window.NO_WINDOW # window function to apply
    
    #
    # Genarate signal for analysis
    #
    awfi = gn.cos(npts, fs, ampl, freq, phase)
    awfq = gn.sin(npts, fs, ampl, freq, phase)
    qwfi = gn.quantize(awfi, fsr, qres, qnoise, code_fmt)
    qwfq = gn.quantize(awfq, fsr, qres, qnoise, code_fmt)
    
    #
    # Compute FFT
    #
    fft_cplx = gn.fft(qwfi, qwfq, qres, navg, nfft, window, code_fmt)

    #
    # Fourier analysis configuration
    #
    object_key = "fa"
    signal_tone_key = 'A'
    ssb_fund = 0 # number of single-side bins
    gn.mgr_remove(object_key)
    gn.fa_create(object_key)
    gn.fa_fsample(object_key, fs)
    gn.fa_max_tone(object_key, signal_tone_key, gn.FaCompTag.SIGNAL, ssb_fund)

    #
    # Fourier analysis execution
    #
    axis_type = gn.FreqAxisType.DC_CENTER # axis type        
    results = gn.fft_analysis(object_key, fft_cplx, nfft, axis_type)
    print('type of results is - ', type(results),"\n")
    
    #
    # Print results
    #
    axis_fmt = gn.FreqAxisFormat.FREQ # axis-format
    freq_axis = gn.freq_axis(nfft, axis_type, fs, axis_fmt)
    fft_db = gn.db(fft_cplx)
    if gn.FreqAxisType.DC_CENTER == axis_type:
        fft_db = gn.fftshift(fft_db)
    
    annots = gn.fa_annotations(results, axis_type, axis_fmt)
    print("annotation keys in results dictionary:", annots.keys(),"\n")
    
    print('annots["labels"]: ')
    labels_head = ('frequency (Hz)', 'magnitude (dBFs)', 'component label')
    labels_table = tabulate(annots["labels"], headers=labels_head, tablefmt="grid")
    print(labels_table, "\n")

    print('annots["tone_boxes"]: ')
    c1 = [x[0] for x in annots["tone_boxes"]]
    c2 = [x[2] for x in annots["tone_boxes"]]
    tone_boxes_head = ('box left boundary (Hz)', 'width (Hz)')
    tone_boxes_table = tabulate(map(list, zip(*(c1, c2))), headers=tone_boxes_head, tablefmt="grid")
    print(tone_boxes_table, "\n")

if __name__ == "__main__":
    main()
```
</details>

```{note}
The enumerations supported for frequency-axis type are [here](#genalyzer.advanced.advanced.FreqAxisType).
```

The console output generated by the above code snippet is shown below for the purpose of discussion.
<details>
  <summary>Console Output</summary>

``` console
foo@bar:~/genalyzer/bindings/python$ python3 examples/gn_doc_helper.py
type of results is -  <class 'dict'>

annotation keys in results dictionary: dict_keys(['labels', 'lines', 'ab_boxes', 'tone_boxes'])

annots["labels"]:
+------------------+--------------------+-------------------+
|   frequency (Hz) |   magnitude (dBFs) | component label   |
+==================+====================+===================+
|          0       |          -69.2329  | dc                |
+------------------+--------------------+-------------------+
|     300000       |           -1.00002 | A                 |
+------------------+--------------------+-------------------+
|    -300000       |         -105.065   | -A                |
+------------------+--------------------+-------------------+
|     600000       |         -106.263   | 2A                |
+------------------+--------------------+-------------------+
|    -600000       |         -106.772   | -2A               |
+------------------+--------------------+-------------------+
|    -900000       |         -105.838   | -3A               |
+------------------+--------------------+-------------------+
|          1.2e+06 |         -108.289   | 4A                |
|                  |                    | -6A               |
+------------------+--------------------+-------------------+
|         -1.2e+06 |          -99.8873  | -4A               |
|                  |                    | 6A                |
+------------------+--------------------+-------------------+
|         -1.5e+06 |          -98.1642  | 5A                |
+------------------+--------------------+-------------------+
|     313000       |          -91.5684  | wo                |
+------------------+--------------------+-------------------+

annots["tone_boxes"]:
+--------------------------+--------------+
|   box left boundary (Hz) |   width (Hz) |
+==========================+==============+
|            -50           |          100 |
+--------------------------+--------------+
|         299950           |          100 |
+--------------------------+--------------+
|        -300050           |          100 |
+--------------------------+--------------+
|         599950           |          100 |
+--------------------------+--------------+
|        -600050           |          100 |
+--------------------------+--------------+
|        -900050           |          100 |
+--------------------------+--------------+
|              1.19995e+06 |          100 |
+--------------------------+--------------+
|             -1.20005e+06 |          100 |
+--------------------------+--------------+
|             -1.50005e+06 |          100 |
+--------------------------+--------------+
|             -1.20005e+06 |          100 |
+--------------------------+--------------+
|              1.19995e+06 |          100 |
+--------------------------+--------------+
|         312950           |          100 |
+--------------------------+--------------+

```
</details>
<br/><br/>

As shown in the console output, the type of ``results``, _i.e._, the output of ``fft_analysis()`` is a dictionary. This dictionary consists of several classes of keys and their mapped values. We begin with details on _annotation_ keys. Using ``fa_annotations()`` function, we list the various components that contribute to the computed performance metrics. From the console output, four kinds of annotations can be identified: _labels_, _lines_, _ab\_boxes_, and _tone\_boxes_. 

_Labels_ are organized as a list of tuples internally, but they are shown in a table format above for display purposes. Each row consists of the frequency location, the magnitude, and the component label for the feature identified. For instance, ``fa_max_tone()`` identifies the signal tone, labeled `A`, at ``300000 Hz`` as expected. Note that the FFT order is set to ``30000``, and the x-axis spans ``3 MHz``. So, the width of each bin is ``100 Hz``, and the signal tone is located at bin number, ``3000``. 

Since the tone is coherently sampled and the sample-rate is chosen to be an integer multiple of the FFT-order, a bin exactly corresponding to ``300 KHz`` is available. Additionally, a number of auto-generated componets, namely the signal tone's image and harmonics upto the `6`th order are also identified by default. Note that for odd-ordered harmonics, `3`rd, `5`th, and so on, only the maximum of the harmonic and its image are considered, whereas for an even-ordered harmonic, both are taken into account. Similarly, a _worst-other_ component, labeled ``wo``, is also identified.

From the second column of _tone\_boxes_ table and the bin width of ``100 Hz`` mentioned previously, we see that the _box_ around the signal tone has a width of ``100 Hz``, _i.e.,_ ``1`` bin. In other words, by setting ``ssb_fund`` to ``0``, we have configured Genalyzer to accumulate the magnitude of only ``1`` bin as the signal component. The left boundary of this _box_ is ``299950 Hz``, and the right boundary is ``300050 Hz``. For every other component, by default, the magnitude value corresponding to only one bin is noted as that component's contribution in various metrics. In a subsequent expansion of the running example, we consider the case when it becomes necessary to set the number of SSBs to a value greater than ``0``.
```{caution}
The choice of the number of single-side bins (SSBs) is important when the signal is not coherently sampled and when the sample-rate is not an integer multiple of the FFT-order. 
``` 
<br/><br/>

##### Harmonics and Other Components
We now expand the code snippet further and understand how to configure harmonics and other components. To do so, we set the number of harmonics using [``fa_hd()``](#genalyzer.advanced.advanced.fa_hd) to ``3`` and continue to explicitly set the number of SSBs for auto-generated components using [``fa_ssb()``](#genalyzer.advanced.advanced.fa_ssb) to ``0`` initially. We also show how the SSB number for various componets can be configured indivually. The ``results`` dictionary is then printed and the FFT is plotted, as shown below. 

```{note}
The component categories for which the number of SSBs can be set are [here](#genalyzer.advanced.advanced.FaSsb)
```

<details>
  <summary>Code Snippet</summary>

``` Python
def main():
    import numpy as np
    import genalyzer.advanced as gn
    import os, json, glob, pprint
    import matplotlib.pyplot as pl
    from matplotlib.patches import Rectangle as MPRect
    from tabulate import tabulate

    #
    # Signal 
    #
    npts = 30000 # number of points in the signal
    freq = 300000 # tone frequency
    phase = 0.0 # tone phase
    ampl_dbfs = -1.0 # amplitude of the tone in dBFS
    qnoise_dbfs = -60.0  # quantizer noise in dBFS
    fsr = 2.0 # full-scale range of I/Q components of the complex tone
    ampl = (fsr / 2) * 10 ** (ampl_dbfs / 20) # amplitude of the tone in linear scale
    qnoise = 10 ** (qnoise_dbfs / 20) # quantizer noise in linear scale

    #
    # FFT configuration
    #
    navg = 1 # number of FFT averages
    nfft = int(npts/navg) # FFT-order
    qres = 12  # data resolution
    code_fmt = gn.CodeFormat.TWOS_COMPLEMENT # integer data format
    window = gn.Window.NO_WINDOW # window function to apply
    ssb_fund = 0 # number of single-side bins
    axis_type = gn.FreqAxisType.DC_CENTER # axis type
    fs = 3e6 # sample-rate of the data
    axis_fmt = gn.FreqAxisFormat.FREQ # axis-format
    num_harmonics = 3 # number of harmonics to analyze
    ssb_rest = 0 # default number of single-side bins for non-signal components
    ssb_dc = 0 # number of single-side bins for the DC-component
    ssb_wo = 0 # number of single-side bins for the WO-component
    
    #
    # Genarate signal for analysis
    #
    awfi = gn.cos(npts, fs, ampl, freq, phase)
    awfq = gn.sin(npts, fs, ampl, freq, phase)
    qwfi = gn.quantize(awfi, fsr, qres, qnoise, code_fmt)
    qwfq = gn.quantize(awfq, fsr, qres, qnoise, code_fmt)
    
    #
    # Compute FFT
    #
    fft_cplx = gn.fft(qwfi, qwfq, qres, navg, nfft, window, code_fmt)

    #
    # Fourier analysis configuration
    #
    object_key = "fa"
    signal_tone_key = 'A'
    ssb_fund = 0 # number of single-side bins
    num_harmonics = 3 # number of harmonics to analyze
    ssb_rest = 0 # number of single-side bins for non-signal components
    ssb_dc = 0 # number of single-side bins for the DC-component
    ssb_wo = 0 # number of single-side bins for the WO-component
    gn.mgr_remove(object_key)
    gn.fa_create(object_key)
    gn.fa_fsample(object_key, fs)
    gn.fa_max_tone(object_key, signal_tone_key, gn.FaCompTag.SIGNAL, ssb_fund)
    gn.fa_hd(object_key, num_harmonics)
    gn.fa_ssb(object_key, gn.FaSsb.DEFAULT, ssb_rest)
    gn.fa_ssb(object_key, gn.FaSsb.DC, ssb_dc)
    gn.fa_ssb(object_key, gn.FaSsb.WO, ssb_wo)

    #
    # Fourier analysis execution
    #
    results = gn.fft_analysis(object_key, fft_cplx, nfft, axis_type)
    print('type of results is - ', type(results),"\n")
    
    #
    # Print results and plot
    #
    freq_axis = gn.freq_axis(nfft, axis_type, fs, axis_fmt)
    fft_db = gn.db(fft_cplx)
    if gn.FreqAxisType.DC_CENTER == axis_type:
        fft_db = gn.fftshift(fft_db)
    
    annots = gn.fa_annotations(results, axis_type, axis_fmt)
    print("annotation keys in results dictionary:", annots.keys(),"\n")
    
    print('annots["labels"]: ')
    labels_head = ('frequency (Hz)', 'magnitude (dBFs)', 'component label')
    labels_table = tabulate(annots["labels"], headers=labels_head, tablefmt="grid")
    print(labels_table, "\n")

    print('annots["tone_boxes"]: ')
    c1 = [x[0] for x in annots["tone_boxes"]]
    c2 = [x[2] for x in annots["tone_boxes"]]
    tone_boxes_head = ('box left boundary (Hz)', 'width (Hz)')
    tone_boxes_table = tabulate(map(list, zip(*(c1, c2))), headers=tone_boxes_head, tablefmt="grid")
    print(tone_boxes_table, "\n")

    print('+----------------+')
    print("results dictionary")
    print('+----------------+')
    pprint.pprint(results)
    
    # plot
    toneDC_Hz = annots["labels"][0][0]
    toneDC_bin = toneDC_Hz/(fs/nfft)
    toneDC_mag = fft_db[int(toneDC_bin+0.5*nfft)]
    toneA_Hz = annots["labels"][1][0]
    toneA_bin = toneA_Hz/(fs/nfft)    
    toneA_mag = fft_db[int(toneA_bin+0.5*nfft)]
    toneA_im_Hz = annots["labels"][2][0]
    toneA_im_bin = toneA_im_Hz/(fs/nfft)
    toneA_im_mag = fft_db[int(toneA_im_bin+0.5*nfft)]
    tone2A_Hz = annots["labels"][3][0]
    tone2A_bin = tone2A_Hz/(fs/nfft)
    tone2A_mag = fft_db[int(tone2A_bin+0.5*nfft)]
    tone2A_im_Hz = annots["labels"][4][0]
    tone2A_im_bin = tone2A_im_Hz/(fs/nfft)
    tone2A_im_mag = fft_db[int(tone2A_im_bin+0.5*nfft)]
    tone3A_im_Hz = annots["labels"][5][0]
    tone3A_im_bin = tone3A_im_Hz/(fs/nfft)
    tone3A_im_mag = fft_db[int(tone3A_im_bin+0.5*nfft)]
    toneWO_Hz = annots["labels"][6][0]
    toneWO_bin = toneWO_Hz/(fs/nfft)
    toneWO_mag = fft_db[int(toneWO_bin+0.5*nfft)]

    scale_MHz = 1e-6
    fig = pl.figure(1)
    fig.clf()
    pl.plot(freq_axis*scale_MHz, fft_db)
    pl.grid(True)
    pl.xlabel('frequency (MHz)')
    pl.ylabel('magnitude (dBFs)')
    pl.xlim(freq_axis[0]*scale_MHz, freq_axis[-1]*scale_MHz)
    pl.ylim(-140.0, 20.0)
    for x, y, label in annots["labels"]:
        if label == 'dc':
            pl.annotate(label+": ["+f"{toneDC_Hz:.2f}"+" ,"+f"{toneDC_mag:.2f}"+"]", 
                        xy=(x*scale_MHz, toneDC_mag), 
                        xytext=(x*scale_MHz, -5), 
                        horizontalalignment="center",
                        arrowprops=dict(arrowstyle='->',color='red',lw=1))
        elif label == 'A':
            pl.annotate(label+": ["+f"{toneA_Hz:.2f}"+" ,"+f"{toneA_mag:.2f}"+"]", 
                        xy=(x*scale_MHz, toneA_mag), 
                        xytext=(x*scale_MHz, 10), 
                        horizontalalignment="center",
                        arrowprops=dict(arrowstyle='->',color='red',lw=1))
        elif label == '-A':
            pl.annotate(label+": ["+f"{toneA_im_Hz:.2f}"+" ,"+f"{toneA_im_mag:.2f}"+"]", 
                        xy=(x*scale_MHz, toneA_im_mag), 
                        xytext=(x*scale_MHz, -20), 
                        horizontalalignment="center",
                        arrowprops=dict(arrowstyle='->',color='red',lw=1))            
        elif label == '2A':
            pl.annotate(label+": ["+f"{tone2A_Hz:.2f}"+" ,"+f"{tone2A_mag:.2f}"+"]", 
                        xy=(x*scale_MHz, tone2A_mag), 
                        xytext=(x*scale_MHz, -40), 
                        horizontalalignment="center",
                        arrowprops=dict(arrowstyle='->',color='red',lw=1))
        elif label == '-2A':
            pl.annotate(label+": ["+f"{tone2A_im_Hz:.2f}"+" ,"+f"{tone2A_im_mag:.2f}"+"]", 
                        xy=(x*scale_MHz, tone2A_im_mag), 
                        xytext=(x*scale_MHz, -40), 
                        horizontalalignment="center",
                        arrowprops=dict(arrowstyle='->',color='red',lw=1))
        elif label == '-3A':
            pl.annotate(label+": ["+f"{tone3A_im_Hz:.2f}"+" ,"+f"{tone3A_im_mag:.2f}"+"]", 
                        xy=(x*scale_MHz, tone3A_im_mag), 
                        xytext=(x*scale_MHz, -60), 
                        horizontalalignment="center",
                        arrowprops=dict(arrowstyle='->',color='red',lw=1))        
        elif label == 'wo':
            pl.annotate(label+": ["+f"{toneWO_Hz:.2f}"+" ,"+f"{toneWO_mag:.2f}"+"]", 
                        xy=(x*scale_MHz, toneWO_mag), 
                        xytext=(x*scale_MHz, -80), 
                        horizontalalignment="center",
                        arrowprops=dict(arrowstyle='->',color='red',lw=1))
        else:
            pl.annotate(label, xy=(x*scale_MHz, y), ha="center", va="bottom")
    pl.savefig('foo.png')

if __name__ == "__main__":
    main()
```
</details>

The console output as a result of running the code-snippet above is shown below. Note that since a random quantization noise is added to the signal, the console output will be different.
<details>
  <summary>Console Output</summary>

``` console
foo@bar:~/genalyzer/bindings/python$ python3 examples/gn_doc_helper.py
type of results is -  <class 'dict'>

annotation keys in results dictionary: dict_keys(['labels', 'lines', 'ab_boxes', 'tone_boxes'])

annots["labels"]:
+------------------+--------------------+-------------------+
|   frequency (Hz) |   magnitude (dBFs) | component label   |
+==================+====================+===================+
|                0 |         -69.231    | dc                |
+------------------+--------------------+-------------------+
|           300000 |          -0.999975 | A                 |
+------------------+--------------------+-------------------+
|          -300000 |        -105.711    | -A                |
+------------------+--------------------+-------------------+
|           600000 |        -107.723    | 2A                |
+------------------+--------------------+-------------------+
|          -600000 |        -108.192    | -2A               |
+------------------+--------------------+-------------------+
|          -900000 |        -106.19     | -3A               |
+------------------+--------------------+-------------------+
|           216800 |         -91.624    | wo                |
+------------------+--------------------+-------------------+

annots["tone_boxes"]:
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
|                   216750 |          100 |
+--------------------------+--------------+

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
</details>
<br/><br/>

As expected, the number of componets in the _labels_ table has reduced, and from the second column of _tone\_boxes_ table, for every component, the number of bins over which the magnitude is accumulated has remained ``1`` (``2 x number of SSBs + 1``). 

From the results dictionary, we see a number of key-value pairs that are computed by Genalyzer. Among them, ``7`` sets of key-value pairs corresponding to the ``7`` components listed in the second column of the _labels_ table in the console output above have the following format:

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

which provide all the relevant information concerning ``-A`` component and its contribution to the metrics computed.

We also see ``13`` sets of key-value pairs which have the following format:

``{PREFIX}_nbins`` : Number of bins associated with PREFIX

``{PREFIX}_rss`` : Root-sum-square associated with PREFIX

where, ``{PREFIX}`` is ``ab`` (analysis band), ``clk`` (clock component), ``dist`` (total distortion), ``hd`` (harmonic distortion), ``ilgt`` (interleaving gain/timing/BW component), ``ilos`` (interleaving offset component), ``ilv`` (ILOS and ILGT components), ``imd`` (Intermodulation Distortion), ``nad`` (noise and distortion), ``noise`` (total noise), ``signal`` (signal component), ``thd`` (Total Harmonic Distortion), and ``userdist`` (user designated sitortion). Note that we haven't yet configured Genalyzer to calculate many of these components. 

As an example, the key-value pairs in the above format for ``dist`` component are as follows:

``'dist_nbins': 4.0,``

``'dist_rss': 1.6721915487139874e-05,``

where, as expected, the number of bins associated with distortion are ``4``, and the root-sum-square of all distortion is seen to be ``1.6721915487139874e-05`` (``-95.5343 dB``). We can similarly read from the conducted analysis that ``noise_rss`` is ``0014244886171634404`` (``-56.9268 dB``), and  ``nad_rss`` is ``0.001424586762147455`` (``-56.9262 dB``), which equals ``dist_rss + noise_rss``. To see how genalyzer computes ``snr``, we see first that ``signal_rss`` is ``0.8912434103542334`` (``-1 dB``). Together with the value of ``noise_rss`` in ``dB``, we can verify that, ``SNR (dB) = 20*log10(signal_rss/noise_rss) = 55.9267469716155 dB``, which matches the value in the console output above. The magnitude spectrum plot labeled with  DC, signal, and harmonic components is shown below.

```{figure} ../../../bindings/python/foo.png

Magnitude spectrum of the FFT showing signal, harmonic, DC, and WO components.
```

In a similar manner, genalyzer computes ``SINAD (dB)`` as ``20*log10(signal_rss/nad_rss)``, ``FSNR (dB)`` as `` 20*log10(1.0/noise_rss)``, ``SFDR (dB)`` as ``20*log10(signal_rss/wo:mag)``, ``NSD (dB)`` as ``10*log10(signal_rss*signal_rss/ab_width)``
