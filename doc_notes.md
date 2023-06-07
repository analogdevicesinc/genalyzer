# gn_config_fa_auto()/gn_config_fa()
Either gn_config_fa_auto() or gn_config_fa() is one of the first calls made to configure genalyzer for Fourier analysis.

## gn_fa_create
gn_fa_create() is called first.

### Mechanics
The only argument passed to it is a char pointer, an "object key". Internally, add_object() method of manager class is called. 

Inputs to *add_object()*:
1) an object key 
2) a static shared_ptr object of fourier_analysis class 
3) a boolean to indicate whether to replace an existing key with a new one if you are passing the same object key

What it does?
Inserts the object key into *object_map*.

*object_map* is a static map object. A map has elements which are accessed through keys. The key-value pairs of *object_map* are strings and a variable called *pointer* which is an alias of a shared_pointer to objects of a class named *object*.

Returns success or raises exception.

### Functionality
gn_fa_create() takes a char array and initializes a map with the key set to this char array and the value set to a pointer to *fourier_analysis* object.

## gn_fa_max_tone
Next, gn_fa_max_tone() is called to configure component key for Fourier analysis.

### Mechanics
Aside from an "object key", a "component key" is also associated with the various components of interest. They have associated with them, certain pre-defined tags such as 
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
1) Retrieves the fourier analysis object associated with the "object key". So, it is assumed that before calling *gn_fa_max_tone()*, there is a fourier analysis object that is mapped to this "object key".
2) Calls *add_max_tone()* method of *fourier_analysis* class.

To call *gn_fa_max_tone()* (which calls *add_max_tone()* internally) the allowed tags are only those associated with either signal component or user-designated distortion or noise component (e.g. WorstOther). That is, to conduct Fourier analysis for other components of interest, use *gn_fa_fixed_tone()*

### Functionality


Things to add in future (if necessary):
1) Details on *get_fa_object()* function that retrieves the object of *fourier_analysis* class given the "object key".