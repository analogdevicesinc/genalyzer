
  // configuration
  config_tone_meas(&c,
                   domain_wf,
                   type_wf,
                   nfft, // FFT order
                   navg, // # of FFTs averaged
                   fs, // sample rate
                   fsr, // full-scale range
                   0, // ADC resolution: unused configuration setting
                   freq, // tone frequency, # of array elements = num_tones
                   scale, // tone scale, # of array elements = num_tones
                   phase, // tone phase, # of array elements = num_tones
                   num_tones, // # of tones
                   false,
                   false,
                   false
                   );