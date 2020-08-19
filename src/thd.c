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

#include <complex.h>
#include <fftw3.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#define NOISE 0.001

double thd_cdouble(const double *real, const double *imag,
                    const uint16_t max_input, const uint16_t samples) {
  // Apply window
  // window = signal.kaiser(N, beta=38)
  // # x = multiply(x, window)
  // in_c[cnt] = (real * win[cnt] + I * imag * win[cnt]) / 2048;

  // Use FFT to get the amplitude of the spectrum
  // ampl = 1 / N * absolute(fft(x))
  // ampl = 20 * log10(ampl / ref + 10 ** -20)

  double thd_dBFS, thd_num, thd_den;
  int i;
  uint16_t fund_indx, peak_indx;
  uint16_t fhs_indx[6];
  double fhs[6];

  double *log_spectrum;
  log_spectrum = (double *)fftw_malloc(sizeof(double) * samples);
  spectrum_cdouble(real, imag, max_input, samples, log_spectrum);

  // Find fundamental
  fhs_indx[0] = find_max(log_spectrum, samples);
  fhs[0] = log_spectrum[fhs_indx[0]];

  // Find peaks
  for (i = 1; i < 6; i++) {
    fhs_indx[i] = find_peak(log_spectrum, samples, fhs_indx, i);
    fhs[i] = log_spectrum[fhs_indx[i]];
  }

  // calculate THD
  thd_num = 0;
  for (i = 0; i < 5; i++)
    thd_num = thd_num + pow(pow(10.0, fhs[i+1]/20.0), 2.0);
  thd_dBFS = 20.0 * log10(sqrt(thd_num)/pow(10.0, fhs[0]/20.0));

  // cleanup
  free(log_spectrum);

  return thd_dBFS;
}
