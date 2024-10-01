# Genalyzer Through Examples
## A Minimal Working Example
The workflow of Genalyzer is best understood with the help of a working example. We use Python to illustrate this workflow in the current section. The data that we will process using Genalyzer for this purpose is captured using an ADALM-PLUTO.
<br/><br/>

### Compute FFT
We begin with the initial steps involved, that is, setting up the configuration parameters, importing data from the json file containing the data, and performing FFT on this data. Since Genalyzer fundamentally relies of spectral analysis, computing the FFT is the first important step. Expand the code-snippet below for details.
<details>
  <summary>Code-Snippet</summary>

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
<br/><br/>

We will expand this code snippet as we go along understanding the various capabilities available with Genalyzer. A couple of points to note from above. 

For the integer data presumably extracted from a data-converter, users need to configure the corresponding code-format. Genalyzer supports two code formats for this purpose: 

* offset binary and 
* two's-complement. 

The enumerations that map to these formats are shown [here] (https://analogdevicesinc.github.io/genalyzer/master/reference_advanced.html#_CPPv412GnCodeFormat). The corresponding enumerations to use in C and MATLAB are shown here and here.

A user of Genalyzer can apply one of three window types for the signal whose FFT is being computed. The supported types are: 

* Blackman-Harris 
* Hanning
* rectangular (labeled no-window within Genalyzer).

The enumerations that map to these window types are shown [here] (https://analogdevicesinc.github.io/genalyzer/master/reference_advanced.html#_CPPv48GnWindow). The corresponding enumerations to use in C and MATLAB are shown here and here.

Upon loading the ```.json``` file containing captured data and importing the I/Q sample sinto numpy arrays, we compute the FFT. Aside from the code format and windowing functions to use, the other arguments that ```fft()``` takes are shown here (provide link to Python API's fft() function). As shown in the refernce page for ```fft()```, it takes a variable number of arguments to handle several usecases. The usecases are split based on whether the user wishes to compute FFT for floating-point complex data, or floating-point data, or fixed-point integer data. In the latter case, the resolution in bits and code-format are the required inputs. The supported datatypes for the input data vector are: ```complex128```, ```float64```, ```int16```, ```int32```, ```int64```. These two categories are further split based on whether the input I/Q samples are interleaved or split into separate inputs.  
<br/><br/>

### Conduct Spectral Analysis
#### Configure Genalyzer
In genalyzer, spectral analysis is managed with the help of a _manager_ and _components_. The following expanded code-snippet shows how this is done. Expand the code-snippet below for details.
<details>
  <summary>Code-Snippet</summary>

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
    gn.fa_max_tone(object_key, component_key, gn.FaCompTag.SIGNAL, ssb_fund)

if __name__ == "__main__":
    main()
```
</details>

##### Create ```fourier_analysis``` Object
The first step here is to call the ```fa_create()``` function. Under the hood, Genalyzer adds a key-value pair to a  ```static``` ```map``` container to manage the metrics to be computed. The key is the string argument passed through ```fa_create()```, and the mapped value is a shared-pointer to an instance of ```fourier_analysis``` class. This key is now used to configure, compute, and access all the RF metrics computed. The intent behind using a ```map``` container is to be easily able to associate multiple keys to different snapshots of the data being analyzed and have the RF metrics for each of those snapshots available. 
<br/><br/>

##### Identify Principal Tone
The next step in configuration stage is to identify the principal tone and label it. This is done by calling either ```fa_fixed_tone()``` or ```fa_max_tone()``` functions. In both cases, the first two arguments are *keys*. The first *key* is the key-value to the ```fourier_analysis``` object that will contain a list of measurements to be computed through Genalyzer and the second *key* is a string that is to be associated with the principal signal component for computing these measurements. In both cases, the third argument is a tag which maps to the component type. The available tags are shown [here] (https://analogdevicesinc.github.io/genalyzer/master/reference_advanced.html#_CPPv411GnFACompTag). To identify a component as the signal tone, we use ```GnFACompTagSignal```. 

In the case of ```fa_fixed_tone()```, the fourth argument users provide to Genalyzer is the frequency where the principal tone is located, whereas in the case of ```fa_max_tone()```, the tone with the highest magnitude is interpreted as the principal tone. 

The final argument is the number of single-side bins for the principal signal component, which will be explained in sufficient detail in the next subsection. 
<br/><br/>
#### Run FFT Analysis
The expanded code-snippet with a call to Genalyzer's ```fft_analysis()``` function is shown below.
<details>
  <summary>Code-Snippet</summary>

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
    fs = 3e6 # 
    fdata = fs / 1 # 
    
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

        freq_axis = gn.freq_axis(nfft, axis_type, fdata, axis_fmt)
        fft_db = gn.db(fft_cplx)
        if gn.FreqAxisType.DC_CENTER == axis_type:
            fft_db = gn.fftshift(fft_db)
        fig = pl.figure(1)
        fig.clf()
        pl.plot(freq_axis*1e-6, fft_db)
        pl.grid(True)
        pl.xlabel('frequency (MHz)')
        pl.xlabel('magnitude (dBFs)')
        pl.xlim(freq_axis[0]*1e-6, freq_axis[-1]*1e-6)
        pl.ylim(-140.0, 20.0)
        annots = gn.fa_annotations(results, axis_type, axis_fmt)
        print(annots)
        for x, y, label in annots["labels"]:
            print(label)
        for x, y, label in annots["labels"]:
            pl.annotate(label, xy=(x, y), ha="center", va="bottom")
        pl.savefig('foo.png')

if __name__ == "__main__":
    main()
```
</details>
<br/><br/>

Note that while we selected ```DC_CENTER``` as the final argument to ```fft_analysis()``` above, ```DC_LEFT``` is also supported. For real FFT, use ```REAL``` as the axis-type.

Although we need to configure a few more settings to get meaningful results, let us first use Genalyzer's Fourier analysis annotaion features to understand a few points. The plot ```foo.png``` generated by the above code snippet is shown below for the purpose of discussion.
![foo](../../../bindings/python/foo.png)