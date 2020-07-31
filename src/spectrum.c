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

uint16_t find_peak_from_left(double *array, uint16_t start, uint16_t stop) {
  uint16_t index;
  bool found_valley = false;
  uint16_t peak_indx = stop;

  for (index = start; index < stop; index++) {
    if (!found_valley)
      found_valley = array[index] < array[index + 1];
    else {
      if (array[index] > array[index + 1]) {
        peak_indx = index;
        break;
      }
    }
  }
  return peak_indx;
}

uint16_t find_max(double *array, uint16_t len) {
  uint16_t c, index;
  double max;

  max = array[0];
  index = 0;

  for (c = 1; c < len; c++) {
    if (array[c] > max) {
      index = c;
      max = array[c];
    }
  }

  return index;
}

double sfdr_cdouble(const double *real, const double *imag,
                    const uint16_t max_input, const uint16_t samples) {
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
  double *win, *abs_fft, *ampl, *ampl_shifted;
  int i;
  uint16_t fund_indx, peak_indx;

  win = (double *)fftw_malloc(sizeof(double) * samples);
  abs_fft = (double *)fftw_malloc(sizeof(double) * samples);
  ampl = (double *)fftw_malloc(sizeof(double) * samples);
  ampl_shifted = (double *)fftw_malloc(sizeof(double) * samples);
  in_c = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * samples);
  out = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * (samples + 1));

  plan_forward =
      fftw_plan_dft_1d(samples, in_c, out, FFTW_FORWARD, FFTW_ESTIMATE);

  for (i = 0; i < samples; i++) win[i] = win_hanning(i, samples);

  // Apply window to data
  for (i = 0; i < samples; i++) {
    in_c[i] = (real[i] * win[i] + I * imag[i] * win[i]) / 1;
  }

  // FFT data
  fftw_execute(plan_forward);
  fftw_destroy_plan(plan_forward);

  // Normalize to dBFS
  for (i = 0; i < samples; ++i) {
    abs_fft[i] = cabs(out[i] / samples);
    ampl[i] = 20 * log10(abs_fft[i] / max_input);
  }

  // Shift FFT
  fftshift(ampl_shifted, ampl, sizeof(double) * samples);

  // Find fundamental
  fund_indx = find_max(ampl_shifted, samples);

  // Traverse until we find peaks
  peak_indx = 0;
  uint16_t peaks_found = 0, max_peak_indx = 0;
  double max_peak, peak;

  while (peak_indx < (samples - 1)) {
    peak_indx = find_peak_from_left(ampl_shifted, peak_indx, samples - 1);

    if (fund_indx == peak_indx) continue;

    peak = ampl_shifted[peak_indx];

    if (peaks_found <= 0) {
      peaks_found++;
      max_peak = peak;
      max_peak_indx = peak_indx;
    } else {
      peaks_found++;
      if (peak > max_peak) {
        max_peak = peak;
        max_peak_indx = peak_indx;
      }
    }
  }

  sfdr_dBFS = ampl_shifted[fund_indx] - max_peak;

  // cleanup
  fftw_free(win);
  fftw_free(abs_fft);
  fftw_free(ampl);
  fftw_free(ampl_shifted);
  fftw_free(in_c);
  fftw_free(out);

  return sfdr_dBFS;
}
