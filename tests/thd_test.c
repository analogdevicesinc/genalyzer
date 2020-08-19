/*
 * BSD 3-Clause License
 *
 * Copyright (c) 2020, Analog Devices, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <assert.h>
#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "genalyzer/genalyzer.h"

#define LEN 4096
#define TOL 0.5
#define THD_DBFS_TARGET -44.0
#define MAX_IN 2048

int main() {
  // Create two complex signals with different amplitudes
  double i0[LEN];
  double q0[LEN];
  int k;
  double t;

  double f_c0 = 10;
  double f_c1 = 30;
  double f_c2 = 50;
  double f_c3 = 70;
  double f_s = 1000;
  double thd_dBFS;

  for (k = 0; k < LEN; k++) {
    t = k / (f_s);
    i0[k] = 2.0*cos(2 * M_PI * f_c0 * t) + 0.01*cos(2 * M_PI * f_c1 * t) + 0.005*cos(2 * M_PI * f_c2 * t) + 0.0025*cos(2 * M_PI * f_c3 * t);
    q0[k] = 2.0*sin(2 * M_PI * f_c0 * t) + 0.01*sin(2 * M_PI * f_c1 * t) + 0.005*cos(2 * M_PI * f_c2 * t) + 0.0025*cos(2 * M_PI * f_c3 * t);
  }
  thd_dBFS = thd_cdouble(i0, q0, MAX_IN, LEN);

  assert(fabs(thd_dBFS - THD_DBFS_TARGET) < TOL);
  return 0;
}
