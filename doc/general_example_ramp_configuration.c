  // configuration
  config_ramp_nl_meas(&c,
                      npts, // # of data points
                      fs, // sample rate
                      fsr, // full-scale range
                      res, // ADC resolution: unused configuration setting
                      start,
                      stop,
                      0.0
                      );