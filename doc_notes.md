# Configuration

## *gn_config_fa_auto()/gn_config_fa()*
Either *gn_config_fa_auto()* or *gn_config_fa()* is one of the first calls made to configure genalyzer for Fourier analysis.

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

*There probably needs to be an enum at the top-level C-wrapper level to give user the flexibility to conduct analysis for any of the four combinations. Currently, only case 3 is supported, by default.*

**Functionality**
The norm of input provided is computed, if the input is complex. This norm is used for Fourier analysis.

### analyze_impl
*analyze_impl()* method of *fourier_analysis* class is the second-level method that is called by *analyze()* and this method does the bulk of Fourier analysis.

**Mechanics**
*analyze_impl()* does the following tasks:
1. Setup analysis band
2. Generate components (i.e., generate the containers needed to store results for various components)
    * DC component and (optional) converter offset 
    * (both optional) Interleaving offset component and clock component - Under what conditions are these enabled? Do you need an example that shows these components being analyzed? 
    * User component(s) and auto component
        * user components are components such as those associated with a fixed tone that you set based on a freq. location, or a max tone (found automatically)
        * auto components are: 
            * the fundamental image component (for complex data, complex analysis this component is made active), 
            * harmonic distortion components (basic C API sets max number of harmonic distortions to analyze to 3. This number is used to generate second order component and its image and one third-order component (Why one?).)
            * interleaving gain and timing components - both not set at basic C API level
            * intermodulation distorion components - these don't seem to be added currently by the basic C API. Under what conditions are they enabled? Need an example that shows intermods. Seems like *gn_fa_imd()* needs to be set to accomplish this.
            * worst-other components - seems like one worst-other component is added by default for analysis. How do you add more? Answer: By calling *gn_fa_wo()*.
3. Then, measure each component (WOs are handled separately, we'll see why) and save.
    * DC component and max-tone component are treated differently compared to every other kind of component which is treated as a fixed-tone component. 
        * Measuring DC component broadly involves the following steps:
            * Determine left and right bins for the DC component. The number of bins is generally equal to 1, with some adjustments to handle special-cases.
            * A mask container is used next. Need to see if I need to understand more details in this mask-creation step.
            * Sum up the magnitude-squared data within the left and right bins determined.
            * Save a bunch of meta-data like component label, component's left and right bin index values, number of bins used for computing value, value computed etc.
        * Measuring max-tone component broadly involves the following steps:
            * Finding the index of max-tone.
            * Determine left and right bins for the max-tone component. The number of bins is generally equal to 2*ssb_fund+1 (ssb_fund is set to 120 by default in the basic C API), with some adjustments to handle special-cases.
            * A mask container is used next. Need to see if I need to understand more details in this mask-creation step.
            * Sum up the magnitude-squared data within the left and right bins determined.
            * Save a bunch of meta-data like component label, component's left and right bin index values, number of bins used for computing value, value computed etc.
            * *Note*: Measuring max-tone component requires the following: *fbin*, *fdata*, *fs*, *fshift*, to be set. Some seem optional (?), for instance *fshift*. Need to find out when it is needed.
        * Measuring fixed-tone component broadly involves the following steps:
            * Find the index of fixed-tone. If it is a negative-frequency component, do a wrap-around operation.
            * Determine left and right bins for the fixed-tone component. The number of bins is generally equal to 1, with some adjustments to handle special-cases. 
            * A mask container is used next. Need to see if I need to understand more details in this mask-creation step.
            * Sum up the magnitude-squared data within the left and right bins determined.
            * Save a bunch of meta-data like component label, component's left and right bin index values, number of bins used for computing value, value computed etc.
            * *Note*: Measuring fixed-tone component requires the following: *fbin*, *fdata*, *fs*, *fshift*, to be set. Some seem optional (?), for instance *fshift*. Need to find out when it is needed.
        * Measuring worst-other component is same as measuring fixed-tone component, with the only difference being the finding of the largest tone outside of those already configured by all other components.
    * Saving involves adding the containers that are keeping all the info about measured components to a larger *fourier_analysis_results* container.
4. Reorder the list of maxspurs (not looked into in much detail, although it seems like a minor thing). A tone is a candidate for maxspur if:
    * it is in-band and non-Signal (these are assumed)
    * it is not DC, unless DC is treated as distortion (dc_as_dist is true)
    * it does not overlap any Signal tones (this needs explanation)
5. Now, with the identified components and using their bin ranges, compute the magnitude value in dBc, *phase* associated with that component (does not apply to max-tone component which is defined to be spread over several bins, unlike others which have only one bin), and *phase_c* computed as *phase - carrier_phase*.
6. An adjustment step is done for complex analysis - *ffinal adjustment* (not looked into in much detail, seems like a minor thing).
7. Now compute all the metrics
    * **Noise-and-distortion**: The sum of everything in the magnitide data except 2*ssb_fund+1 bins over which the max-tone component is defined to be spread over.
    * **Noise**: The sum of everything in the magnitide data except 2*ssb_fund+1 bins (over which the max-tone component is defined to be spread over) and all the bins associated with the components selected for analysis.
    * **Signal**: Assumed to be associated with max-tone. So, sum of magnitude data over 2*ssb_fund+1 bins identified.
    * Full-Scale-to-Noise ratio, a.k.a "SNRFS" (dB), FSNR: 10*log10(1/**Noise**)
    * Signal-to-Noise ratio (dB), SNR: 10*log10(**Signal**/**Noise**)
    * Signal-to-Noise-and-Distortion ratio (dB), SINAD: 10*log10(**Signal**/**Noise-and-distortion**)
    * Spurious-Free Dynamic Range (dB), SFDR: 10*log10(**carrier**/**maxspur**) *Need to look at how the carrier and spur component*
    * Average Bin Noise (dBFS), ABN: 10*log10(**Noise**/Number of noise bins) 
    * Noise Spectral Density (dBFS/Hz), NSD: 10*log10(**Noise**/analysis band-width) 

**Functionality**
1. Analysis band setup involves setting some very basic parameters of analysis band.
2. There's a complicated container that is used internally. It is a tuple of [keys, components, interleaver_offset_clock_keys] (These are more descriptive names, not actual variable names). The third item in this tuple is a pointer to dc componenet (*fa_dc*) that is a subclass of Fourier analysis component (*fourier_analysis_component*). One container of this kind is associated with every components that are added. That is one for dc component, one for user component (only one is added by *gn_config()* and *gn_config_auto()*). The Fourier analysis DC component has a few things associated with it: an enum tag, a string label, number of single-side bins used to analyze component. The Fourier analysis user component has a few things associated with it: an enum tag, a string label, the bin width (the frequency resolution of the FFT-data being analyzed), number of single-side bins used to analyze component. The Fourier analysis fundamental image component has a few things associated with it: an enum tag, a string label, and number of single-side bins used to analyze component. The Fourier analysis worst-other component has only the number of single-side bins associated with it.
3. The components added on default by *gn_config_fa_auto()* currently are dc (labeled "0"), signal component ("A"), fundamental image component ("-A"), second-order components ("2A" and "-2A"), third-order component ("-3A"), and worst-other ("wo").

*Note:* Max. FFT order supported by genalyzer is 2^29. What are all the settings where this becomes relevant?

1. Should enabling comverter offset, interleaving offset, and clock components be exposed at the basic C API level. If it is already done at the advanced C API level, how?
