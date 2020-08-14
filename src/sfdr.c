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

  double sfdr_dBFS;
  int i;
  uint16_t fund_indx, peak_indx;

  double *log_spectrum;
  log_spectrum = (double *)fftw_malloc(sizeof(double) * samples);
  spectrum_cdouble(real, imag, max_input, samples, log_spectrum);

  // Find fundamental
  fund_indx = find_max(log_spectrum, samples);

  // Traverse until we find peaks
  peak_indx = 0;
  uint16_t peaks_found = 0, max_peak_indx = 0;
  double max_peak, peak;

  while (peak_indx < (samples - 1)) {
    peak_indx = find_peak_from_left(log_spectrum, peak_indx, samples - 1);

    if (fund_indx == peak_indx) continue;

    peak = log_spectrum[peak_indx];

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

  sfdr_dBFS = log_spectrum[fund_indx] - max_peak;

  // cleanup
  fftw_free(log_spectrum);

  return sfdr_dBFS;
}
