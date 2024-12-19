// Copyright (C) 2024 Analog Devices, Inc.
//
// SPDX short identifier: ADIBSD OR GPL-2.0-or-later
#include "genalyzer_cwrapper.h"
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

int
main (int argc, char *argv[])
{
  // opaque config struct that will contain config settings
  // config c = NULL;

  /* configuration */
  // config_tone_meas(&c, FREQ, ...);
  // config_tone_meas(&c, TIME,...);

  /* load waveform generation*/
  // read_file_to_array(...);

  /* compute metrics */
  // fsnr_val = metric(..., "FSNR");

  return 0;
}