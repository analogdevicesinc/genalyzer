Using Genalyzer
===============
As mentioned previously, Genalyzer is a C++ library designed to support computation of RF performance metrics using either simulation data or data captured from a physical instrument. Bindings are provided to enable users to write C- or Python-based scripts to compute the desired performance metrics. In both cases, the overall structure of a C-example script that links to genalyzer library is similar, as shown in the next section. The overall structure in a simulation-only scenario consists of three stages:

* Test Configuration
* Waveform Generation
* Metric Computation

In the data-capture scenario, where the response of a data-converter is available in the form of text files, the structure would be 

* Test Configuration
* Waveform Loading
* Metric Computation

That is, only the second step in the procedure involved would change. The overall structure of a C-example that utilizes genalyzer library in the simulation-only scenario is shown by the following example.
``` c
#include "cgenalyzer.h"

int main(int argc, char *argv[]) {  
  // opaque config struct that will contain config settings
  // config c = NULL;

  /* configuration */
  // config_tone_meas(...);

  /* waveform generation and quantize */
  // gen_tone(...);
  // quantize(...);

  /* compute metrics */
  // fsnr_val = metric(..., "FSNR");

  return 0;
}```

.. literalinclude:: general_example_skelton1.c
	:language: C

Similarly, the overall structure of a C-example in the data-capture scenario is shown by the following example.

.. literalinclude:: general_example_skelton2.c
	:language: C

.. include:: 02a_test_configuration.rst
.. include:: 02b_waveform_generation.rst
.. include:: 02c_perf_metric_computation.rst