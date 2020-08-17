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

static void fftshift(void *dest, void *src, size_t len) {
  memcpy(dest, src + (len / 2), len / 2);
  memcpy(dest + (len / 2), src, len / 2);
}

static double win_hanning(int j, int n) {
  double a = 2.0 * M_PI / (n - 1), w;
  w = 0.5 * (1.0 - cos(a * j));
  w = 1;
  return (w);
}

void spectrum_cdouble(const double *real, const double *imag,
                    const uint16_t max_input, const uint16_t samples, double *log_spectrum) {
  // Apply window
  // window = signal.kaiser(N, beta=38)
  // # x = multiply(x, window)
  // in_c[cnt] = (real * win[cnt] + I * imag * win[cnt]) / 2048;

  // Use FFT to get the amplitude of the spectrum
  // ampl = 1 / N * absolute(fft(x))
  // ampl = 20 * log10(ampl / ref + 10 ** -20)

  fftw_complex *in_c, *out;
  fftw_plan plan_forward;
  double sfdr_dBFS;
  double *win, *lin_spectrum_noshift, *log_spectrum_noshift;
  int i;
  uint16_t fund_indx, peak_indx;

  win = (double *)fftw_malloc(sizeof(double) * samples);
  lin_spectrum_noshift = (double *)fftw_malloc(sizeof(double) * samples);
  log_spectrum_noshift = (double *)fftw_malloc(sizeof(double) * samples);
  in_c = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * samples);
  out = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * (samples + 1));

  plan_forward =
      fftw_plan_dft_1d(samples, in_c, out, FFTW_FORWARD, FFTW_ESTIMATE);

  for (i = 0; i < samples; i++) win[i] = win_hanning(i, samples);

  // Apply window to data
  for (i = 0; i < samples; i++) {
    in_c[i] = (real[i] * win[i] + I * *(imag+i) * win[i]) / 1;
  }

  // FFT data
  fftw_execute(plan_forward);
  fftw_destroy_plan(plan_forward);

  // Normalize to dBFS
  for (i = 0; i < samples; ++i) {
    lin_spectrum_noshift[i] = cabs(out[i] / samples);
    log_spectrum_noshift[i] = 20 * log10(lin_spectrum_noshift[i] / max_input);
  }

  // Shift FFT
  fftshift(log_spectrum, log_spectrum_noshift, sizeof(double) * samples);
  
  // cleanup
  fftw_free(win);
  fftw_free(lin_spectrum_noshift);
  fftw_free(log_spectrum_noshift);
  fftw_free(in_c);
  fftw_free(out);
}
