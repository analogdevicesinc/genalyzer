// Copyright (C) 2024 Analog Devices, Inc.
//
// SPDX short identifier: ADIBSD OR GPL-2.0-or-later
// configuration
config_ramp_nl_meas (&c,
                     npts, // # of data points
                     fs,   // sample rate
                     fsr,  // full-scale range
                     res,  // ADC resolution: unused configuration setting
                     start, stop, 0.0);