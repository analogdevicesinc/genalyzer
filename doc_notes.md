## gn_config_fa_auto()/gn_config_fa()
Either gn_config_fa_auto() or gn_config_fa() is one of the first calls made to configure genalyzer for Fourier analysis using genalyzer.

### Mechanics
gn_fa_create() is called and the object key, a string is passed to it. Internally, add_object() method of manager class is called. add_object() takes three arguments: 
1) an object key 
2) a static shared_ptr object of fourier_analysis class 
3) a boolean to indicate whether to replace an existing key with a new one if you are passing the same object key

### Functionality