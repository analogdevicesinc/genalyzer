# Configuration

## gn_config_fa_auto()/gn_config_fa()
Either gn_config_fa_auto() or gn_config_fa() is one of the first calls made to configure genalyzer for Fourier analysis.

### gn_fa_create
gn_fa_create() is called first.

**Mechanics**
The only argument passed to it is a char pointer, an "object key". Internally, add_object() method of manager class is called. 

Inputs to *add_object()*:
1) an object key 
2) a static shared_ptr object of fourier_analysis class 
3) a boolean to indicate whether to replace an existing key with a new one if you are passing the same object key

What it does?
Inserts the object key into *object_map*.

*object_map* is a static map object. A map has elements which are accessed through keys. The key-value pairs of *object_map* are strings and a variable called *pointer* which is an alias of a shared_pointer to objects of a class named *object*.

Returns success or raises exception.

**Functionality**
gn_fa_create() takes a char array and initializes a map with the key set to this char array and the value set to a pointer to *fourier_analysis* object.

### gn_fa_max_tone
Next, *gn_fa_max_tone()* is called by *gn_config_fa_auto()* (or *gn_config_fa()*) to configure a max-tone "component key" for Fourier analysis.

**Mechanics**
In genalyzer, aside from an "object key", a "component key" is also associated with the various components of interest. They have associated with them, certain pre-defined tags such as 
1) DC component (always Bin 0)
2) Signal component
3) Harmonic distortion
4) Intermodulation distortion
5) Interleaving offset component
6) Interleaving gain/timing/BW component
7) Clock component
8) User-designated distortion
9) Noise component (e.g. WorstOther)

Inputs to *gn_fa_max_tone()*:
1) an object key 
2) a component key 
3) a component tag
4) number of single-side bins

What it does?
1) Retrieves the fourier analysis object associated with the "object key". So, it is assumed that before calling *gn_fa_max_tone()*, there is a fourier analysis object in genalyzer that is mapped to this "object key".
2) Calls *add_max_tone()* method of *fourier_analysis* class.

To call *gn_fa_max_tone()* (which calls *add_max_tone()* internally) the allowed tags are only those associated with either signal component or user-designated distortion or noise component (e.g. WorstOther). So, to conduct Fourier analysis for other components of interest, use *gn_fa_fixed_tone()*.

Internal to *fourier_analysis* class is a component map (*comp_map*) container. *add_max_tone()* takes component key (a char array) and associates with it a (unique) pointer to an object of *fourier_analysis_component* class which creates a record of the component's tag, location on frequency-axis, the width, and the number of single-side bins. (What are the last two quantities?)

**Functionality**
*gn_fa_max_tone()* sets up internal data-structures to conduct analysis based on the max tone. (No analysis is done yet.)

### gn_fa_hd
*gn_fa_hd()* is another component labeling API function called by *gn_config_fa_auto()* (or *gn_config_fa()*) to configure a harmonic-distortion "component key" for Fourier analysis.

**Mechanics**
Calls *set_hd()* method of *fourier_analysis* class.

Inputs to *gn_fa_hd()*:
1) an object key 
2) max. harmonic order

Similar to *gn_fa_max_tone()*, but simpler compared to setting up *fourier_analysis_component* for max-tone analysis, an internal variable is updated on the max harmonic order.

**Functionality**
*gn_fa_hd()* assigns the max harmonic order value it is passed to an internal class property (*m_hd*).

### gn_fa_ssb
*gn_fa_ssb()* is another component labeling API function called by *gn_config_fa_auto()* (or *gn_config_fa()*) to configure single-side bins associated with the "object key" in question using which we are configuring Fourier analysis.

**Mechanics**
Calls *set_ssb()* method of *fourier_analysis* class.

Inputs:
1) an object key 
2) ssb-group label (an enum)
3) Number of single-side bins

Note: In both *gn_config_fa_auto()* and *gn_config_fa()*, a default single-side bin number is set. Other options include setting ssbs for DC component, signal components, worst-other components.

**Functionality**
*gn_fa_ssb()* assigns the number of single-side bins associated with an ssb-group to an internal class property (*m_ssb_def*).

**Need discussion on single-side bins**

### Others
*gn_fa_fsample()*, *gn_fa_fdata()*, *gn_fa_fshift()*, *gn_fa_conv_offset()* similar set internal class properties. Everything so far is to simply set internal containers and class properties for fourier analysis.

Question:
1. How is a line such as `m_user_keys.push_back(key);` in fourier_analysis.cpp possible? You can access the members of a static object without referring to the object itself?

Things to add in future (if necessary):
1) Details on *get_fa_object()* function that retrieves the object of *fourier_analysis* class given the "object key".

# Fourier Analysis
*gn_get_fa_results()* is the core API function that does Fourier Analysis.

*Skipping allocating memory for results keys, result key sizes, and result values...*

## gn_fft_analysis()
Among other things, *gn_fft_analysis()* takes interleaved FFT data. So, users calling this function directly need to be aware of this requirement.

### analyze
*analyze()* method of *fourier_analysis* class is the top-level method that does Fourier analysis.

**Mechanics**
Allocates memory based on four cases.
1. Real data, complex analysis
2. Real data, real analysis
3. Complex data, complex analysis
4. Complex data, real analysis

For the first two cases, the assumption is that input array is mean-square FFT data (phase not available).

*There probably needs to be an enum at the top-level to give user the flexibility to conduct analysis for any of the four combinations. Currently, only case 3 is supported, by default.*

**Functionality**
The norm of input provided is computed, if the input is complex. This norm is used for Fourier analysis.

### analyze_impl
*analyze_impl()* method of *fourier_analysis* class is the second-level method that is called by *analyze()* and this method does the bulk of Fourier analysis.

**Mechanics**
*analyze_impl()* does the following tasks:
1. Setup analysis band

**Functionality**