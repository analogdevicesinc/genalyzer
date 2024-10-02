# Genalyzer Through Examples
## A Minimal Working Example
The workflow of Genalyzer is best understood with the help of a working example. We use Python to illustrate this workflow in the current section. The data that we will process using Genalyzer for this purpose is captured using an ADALM-PLUTO. It consists of a single tone at ``100 KHz`` sampled at ``3 MSPS``.
<br/><br/>

### Compute FFT
We begin with the initial steps involved, that is, setting up the configuration parameters, importing data from the json file containing the data, and performing FFT on this data. Since Genalyzer fundamentally relies of spectral analysis, computing FFT is the first important step. Expand the code-snippet below for details.
<details>
  <summary>Code Snippet</summary>

``` Python
def main():
    import numpy as np
    import genalyzer.advanced as gn
    import os, json, glob

    #
    # Setup
    #
    navg = 1 # number of FFT averages
    nfft = 32768 # FFT-order
    qres = 12  # data resolution
    code_fmt = gn.CodeFormat.TWOS_COMPLEMENT # integer data format
    window = gn.Window.NO_WINDOW # window function to apply
    
    #
    # Import signal for analysis
    #
    test_dir = os.path.join(*["..", "..", "..", "tests", "test_vectors"])
    loc = os.path.dirname(__file__)
    f = glob.glob(os.path.join(loc, test_dir, "test_Pluto_DDS_data_1658159639196.json"))
    a = open(f[0])
    data = json.load(a)
    qwfi = data["test_vec_i"]
    qwfi = np.array([int(i) for i in qwfi])
    qwfq = data["test_vec_q"]
    qwfq = np.array([int(i) for i in qwfq])
    fft_cplx = gn.fft(qwfi, qwfq, qres, navg, nfft, window, code_fmt)

if __name__ == "__main__":
    main()
```
</details>

We will expand this code snippet as we go along understanding the various capabilities available with Genalyzer. A couple of points to note from above. 

For the integer data presumably extracted from a data-converter, users need to configure the corresponding code-format. Genalyzer supports two code formats for this purpose: offset binary and two's-complement. 

The enumerations that map to these formats are shown [here] (https://analogdevicesinc.github.io/genalyzer/master/reference_advanced.html#_CPPv412GnCodeFormat). The corresponding enumerations to use in C and MATLAB are shown here and here.

A user of Genalyzer can apply one of three window types for the signal whose FFT is being computed. The supported types are: Blackman-Harris, Hanning, and rectangular (labeled no-window within Genalyzer) windows.

The enumerations that map to these window types are shown [here] (https://analogdevicesinc.github.io/genalyzer/master/reference_advanced.html#_CPPv48GnWindow). The corresponding enumerations to use in C and MATLAB are shown here and here.

Upon loading the ``.json`` file containing captured data and importing the I/Q sample sinto ``numpy`` arrays, we compute the FFT. Aside from the code format and windowing functions to use, the other arguments that ``fft()`` takes are shown here (provide link to Python API's fft() function). As shown in the reference page for ``fft()``, it takes a variable number of arguments to handle several usecases. The usecases are split based on how the I/Q samples are represented, _i.e._, whether the samples are complex-valued, or interleaved, or split into I and Q separately. These categories are further split based on whether the samples are represented as floating-point or fixed-point integer data. In the latter case, the resolution in bits and code-format are the additional required inputs. The supported datatypes for the input data vector are: ``complex128``, ``float64``, ``int16``, ``int32``, ``int64``. 

Genalyzer's ``fft()`` computes FFT for complex-valued data only. To compute FFT for real-valued data, use ``rfft()``. Additional details [here] (link to ``rfft()`` API).
<br/><br/>

### Conduct Spectral Analysis
#### Configure Genalyzer
In genalyzer, spectral analysis is managed with the help of a _manager_ and _components_. The following expanded code-snippet shows how this is done. Expand the code-snippet below for details.
<details>
  <summary>Code Snippet</summary>

``` Python
def main():
    import numpy as np
    import genalyzer.advanced as gn
    import os, json, glob

    #
    # Setup
    #
    navg = 1 # number of FFT averages
    nfft = 32768 # FFT-order
    qres = 12  # data resolution
    code_fmt = gn.CodeFormat.TWOS_COMPLEMENT # integer data format
    window = gn.Window.NO_WINDOW # window function to apply
    ssb_fund = 100 # number of single-side bins
    
    #
    # Import signal for analysis
    #
    test_dir = os.path.join(*["..", "..", "..", "tests", "test_vectors"])
    loc = os.path.dirname(__file__)
    f = glob.glob(os.path.join(loc, test_dir, "test_Pluto_DDS_data_1658159639196.json"))
    a = open(f[0])
    data = json.load(a)
    qwfi = data["test_vec_i"]
    qwfi = np.array([int(i) for i in qwfi])
    qwfq = data["test_vec_q"]
    qwfq = np.array([int(i) for i in qwfq])
    fft_cplx = gn.fft(qwfi, qwfq, qres, navg, nfft, window, code_fmt)

    #
    # Fourier analysis configuration
    #
    object_key = "fa"
    component_key = 'A'
    gn.fa_create(object_key)
    gn.fa_fsample(object_key, fs)
    gn.fa_max_tone(object_key, component_key, gn.FaCompTag.SIGNAL, ssb_fund)

if __name__ == "__main__":
    main()
```
</details>

##### Create ``fourier_analysis`` Object
The first step in configuration stage is to call the ``fa_create()`` function. Under the hood, Genalyzer adds a key-value pair to a  ``static`` ``map`` container to manage the metrics to be computed. The key is the string argument passed through ``fa_create()``, and the mapped value is a shared-pointer to an instance of ``fourier_analysis`` class. This key is now used to configure, compute, and access all the RF metrics computed using the methods available through ``fourier_analysis`` class. The intent behind using a ``map`` container is to be easily able to associate multiple keys to different snapshots of the data being analyzed and have the RF metrics for each of those snapshots available. 
<br/><br/>

##### Set Sample-Rate
Then, set the sample-rate of the data vector.
<br/><br/>

##### Identify Principal Tone
The next step is to identify the principal tone and label it. This is done by calling either ``fa_fixed_tone()`` or ``fa_max_tone()`` functions. In both cases, the first two arguments are *keys*. The first *key* is the key-value to the ``fourier_analysis`` object that will contain a list of measurements to be computed through Genalyzer and the second *key* is a string that is to be associated with the principal signal component for computing these measurements. In both cases, the third argument is a tag which maps to the component type. The available tags are shown [here] (https://analogdevicesinc.github.io/genalyzer/master/reference_advanced.html#_CPPv411GnFACompTag). To identify a component as the principal tone, we use ``GnFACompTagSignal``. 

In the case of ``fa_fixed_tone()``, the fourth argument users provide to Genalyzer is the frequency where the principal tone is located, whereas in the case of ``fa_max_tone()``, the tone with the highest magnitude is interpreted as the principal tone. 

The final argument is the number of single-side bins for the principal signal component, which will be explained in sufficient detail in the next subsection. 
<br/><br/>
#### Run FFT Analysis

The next stage is to run ``fft_analysis()`` function and conduct Fourier analysis. See the expanded code-snippet below.
<details>
  <summary>Code Snippet</summary>

``` Python
def main():
    import numpy as np
    import genalyzer.advanced as gn
    import os, json, glob

    #
    # Setup
    #
    plot = True # plot: Y/N
    navg = 1 # number of FFT averages
    nfft = 32768 # FFT-order
    qres = 12  # data resolution
    code_fmt = gn.CodeFormat.TWOS_COMPLEMENT # integer data format
    window = gn.Window.NO_WINDOW # window function to apply
    ssb_fund = 100 # number of single-side bins
    axis_type = gn.FreqAxisType.DC_CENTER # axis type
    fs = 3e6 # sample-rate of the data
    axis_fmt = gn.FreqAxisFormat.FREQ
    
    #
    # Import signal for analysis
    #
    test_dir = os.path.join(*["..", "..", "..", "tests", "test_vectors"])
    loc = os.path.dirname(__file__)
    f = glob.glob(os.path.join(loc, test_dir, "test_Pluto_DDS_data_1658159639196.json"))
    a = open(f[0])
    data = json.load(a)
    qwfi = data["test_vec_i"]
    qwfi = np.array([int(i) for i in qwfi])
    qwfq = data["test_vec_q"]
    qwfq = np.array([int(i) for i in qwfq])
    fft_cplx = gn.fft(qwfi, qwfq, qres, navg, nfft, window, code_fmt)

    #
    # Fourier analysis configuration
    #
    object_key = "fa"
    component_key = 'A'
    gn.fa_create(object_key)
    gn.fa_fsample(object_key, fs)
    gn.fa_max_tone(object_key, component_key, gn.FaCompTag.SIGNAL, ssb_fund)

    #
    # Fourier analysis execution
    #
    results = gn.fft_analysis(object_key, fft_cplx, nfft, axis_type)

    #
    # Plot
    #
    if plot:
        import matplotlib.pyplot as pl
        from matplotlib.patches import Rectangle as MPRect

        freq_axis = gn.freq_axis(nfft, axis_type, fs, axis_fmt)
        fft_db = gn.db(fft_cplx)
        if gn.FreqAxisType.DC_CENTER == axis_type:
            fft_db = gn.fftshift(fft_db)
        
        annots = gn.fa_annotations(results, axis_type, axis_fmt)
        print("annotation keys in results dictionary:", annots.keys())
        print("")
        
        print('------------------')
        print("results dictionary")
        print('------------------')
        for nest in annots:
            print(nest)            
            for sub_nest in annots[nest]:
                print(sub_nest)
            print("")
        
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
            pl.annotate(label, xy=(x*scale_MHz, y), ha="center", va="bottom")
        pl.savefig('foo.png')

if __name__ == "__main__":
    main()
```
</details>

Note that while we selected ``DC_CENTER`` as the final argument to ``fft_analysis()`` above, ``DC_LEFT`` is also supported, which (as the name suggests) moves DC to the left of the axis. For real FFT, use ``REAL`` as the axis-type.

Although we need to configure a few more settings to get meaningful results, let us first use Genalyzer's Fourier analysis annotation utility to understand a few points. The command-line output and the plot ```foo.png``` generated by the above code snippet is shown below for the purpose of discussion.
<details>
  <summary>Console Output</summary>

``` console
foo@bar:~/genalyzer/bindings/python$ python3 examples/gn_doc_helper.py
annotation keys in results dictionary: dict_keys(['labels', 'lines', 'ab_boxes', 'tone_boxes'])

------------------
results dictionary
------------------
labels
(0.0, -61.38584925353796, 'dc')
(100067.138671875, -9.61374207372258, 'A')
(-100067.138671875, -62.23836952385361, '-A')
(200134.27734375, -80.98298244170492, '2A')
(-200134.27734375, -90.17770944842849, '-2A')
(-300201.416015625, -89.2080984222927, '-3A')
(400268.5546875, -88.68213451654215, '4A')
(-400268.5546875, -93.30420247904526, '-4A')
(500335.693359375, -98.43170613516526, '5A')
(600402.83203125, -93.32599185536986, '6A')
(-600402.83203125, -93.68652139087268, '-6A')
(-91.552734375, -57.29554026718038, 'wo')

lines
(-3000000.0, -80.13895002145804, 3000000.0, -80.13895002145804)

ab_boxes

tone_boxes
(-45.7763671875, -300, 91.552734375, 238.61415074646203)
(90866.0888671875, -300, 18402.099609375, 290.38625792627744)
(-100112.9150390625, -300, 91.552734375, 237.7616304761464)
(200088.5009765625, -300, 91.552734375, 219.0170175582951)
(-200180.0537109375, -300, 91.552734375, 209.8222905515715)
(-300247.1923828125, -300, 91.552734375, 210.7919015777073)
(400222.7783203125, -300, 91.552734375, 211.31786548345787)
(-400314.3310546875, -300, 91.552734375, 206.69579752095473)
(500289.9169921875, -300, 91.552734375, 201.56829386483474)
(600357.0556640625, -300, 91.552734375, 206.67400814463014)
(-600448.6083984375, -300, 91.552734375, 206.31347860912732)
(-137.3291015625, -300, 91.552734375, 242.70445973281963)

```
</details>

![foo](../../../bindings/python/foo.png)

Annotation keys help illustrate how Genalyzer works under the hood to compute various quatities. They also help users identify the components that contribute to the computed performance metrics. Four kinds of annotations can be identified: _labels_, _lines_, _ab\_boxes_, and _tone_boxes_. 

_Labels_ are organized as a list of tuples. Each tuple consists of the frequency location, the magnitude, and the component label for the feature identified. For instance, ``fa_max_tone()`` identifies the principal tone at ``100067.138671875 MHz``. Due to the fact that the dataset was not coherently sampled, there is no bin corresponding to ``100 KHz``, and the closest bin is noted. As expected, the principal component is labeled `A`. Additionally, its image and harmonics upto the `6`-th order are also identified by default. Note that for an odd-ordered harmonic, `3`rd, `5`th, and so on, Genalyzer takes only the maximum of the harmonic and its image into account, whereas for an even-ordered harmonic, it takes both into account. Moreover, an additional _worst-other_ component is also considered in computing various metrics, and it is labeled ``wo``.