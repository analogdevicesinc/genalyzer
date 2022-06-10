/*
 * cgenalyzer - genalyzer API header file
 *
 * Copyright (C) 2021 Analog Devices, Inc.
 * Author: Srikanth Pagadarai
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 * */

#ifndef CGENALYZER_H
#define CGENALYZER_H
#include "cgenalyzer_advanced2.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef _WIN32
#ifdef GENALYZER_EXPORTS
#define __api __declspec(dllexport)
#else
#define __api __declspec(dllimport)
#endif
#elif __GNUC__ >= 4 && !defined(MATLAB_MEX_FILE)                              \
    && !defined(MATLAB_LOADLIBRARY)
#define __api __attribute__ ((visibility ("default")))
#else
#define __api
#endif

  typedef enum waveform_type
  {
    REAL_COSINE,
    REAL_SINE,
    COMPLEX_EXP,
    REAL_NOISE,
    COMPLEX_NOISE,
    RAMP
  } waveform_type;
  typedef enum meas_domain
  {
    TIME,
    FREQ
  } meas_domain;
  typedef enum int_repr
  {
    I32,
    I64
  } int_repr;
  typedef enum win_func
  {
    BlackmanHarris,
    Hann,
    Rect
  } win_func;

  // opaque pointer
  typedef struct gn_config_private *gn_config;
  typedef gn_config gn_tone_config;

  /**
   * @brief Configure test based on real sinusoidal or complex exponential
   * tones
   * @return 0 on success, non-zero otherwise
   * @param c Configuration structure of test and waveform to generate
   * @param wf_type ENUM value to indicate input waveform type.
   * Options: REAL_COSINE, REAL_SINE, COMPLEX_EXP    
   * @param npts Number of sample points in the generated waveform
   * @param sample_rate Input Sample rate of the data converter
   * @param tone_freq Input array of tone frequencies to generate
   * @param tone_ampl Input array of tone scales to generate
   * @param tone_phase Input array of tone phases to generate   
   */
  __api int gn_config_tone (gn_tone_config *c, waveform_type wf_type, size_t npts,
                              double sample_rate, double tone_freq, double tone_ampl,
                              double tone_phase);
  /**
   * @brief Configure test based on real sinusoidal or complex exponential
   * tones
   * @param c Configuration structure of test and waveform to generate
   * @param m_domain Input ENUM value to indicate time/frequency domain of
   * input waveform
   * @param wf_type Input ENUM value to indicate input waveform type as real or
   * complex
   * @param fft_order Input FFT order desired for the waveform generated or
   * contained in the provided FFT data
   * @param num_avgs Input number of FFT averages desired for the waveform
   * generated or contained in the provided FFT data
   * @param sample_rate Input Sample rate of the data converter
   * @param full_scale_range Input full-scale-range of the data converter
   * @param resolution Input resolution of the data converter
   * @param window Window function to apply for computing FFT
   * @param fsample_update Input Boolean value to update fsample
   * @param fdata_update Input Boolean value to update fdata
   * @param fshift_update Input Boolean value to update fshift
   */
  /*
  __api void gn_config_tone_meas (gn_config_tone *c, meas_domain m_domain,
                                  waveform_type wf_type, size_t fft_order,
                                  int num_avgs, double sample_rate,
                                  double full_scale_range, int resolution, 
                                  win_func window, 
                                  bool fsample_update, bool fdata_update,
                                  bool fshift_update);
                                  */
  /**
   * @brief Configure test based on noise waveform
   * @param c Configuration structure of test and waveform to generate
   * @param wf_type Input ENUM value to indicate input waveform type as real or
   * complex
   * @param fft_order Input FFT order desired for the waveform generated or
   * contained in the provided FFT data
   * @param num_avgs Input number of FFT averages desired for the waveform
   * generated or contained in the provided FFT data
   * @param sample_rate Input Sample rate of the data converter
   * @param full_scale_range Input full-scale-range of the data converter
   * @param resolution Input resolution of the data converter
   * @param noise_pwr_lvl Input noise power of the data converter
   * @param fsample_update Input Boolean value to update fsample
   * @param fdata_update Input Boolean value to update fdata
   * @param fshift_update Input Boolean value to update fshift
   */
  __api void gn_config_noise_meas (gn_config *c, waveform_type wf_type,
                                   size_t fft_order, int num_avgs,
                                   double sample_rate, double full_scale_range,
                                   int resolution, double noise_pwr_lvl,
                                   bool fsample_update, bool fdata_update,
                                   bool fshift_update);
  /**
   * @brief Configure non-linearity test based on real sinusoidal tone
   * @param c Configuration structure of test and waveform to generate
   * @param wf_type Input ENUM value to indicate input waveform type as cosine
   * real or sine real
   * @param npts Input number desired for the waveform generated or contained
   * in the provided data
   * @param sample_rate Input Sample rate of the data converter
   * @param full_scale_range Input full-scale-range of the data converter
   * @param resolution Input resolution of the data converter
   * @param tone_freq Input array of tone frequencies to generate
   * @param tone_ampl Input array of tone scales to generate
   * @param tone_phase Input array of tone phases to generate
   * @param num_tones Input number of tones to generate
   */
  __api void gn_config_tone_nl_meas (gn_config *c, waveform_type wf_type,
                                     size_t npts, double sample_rate,
                                     double full_scale_range, int resolution,
                                     double *tone_freq, double *tone_ampl,
                                     double *tone_phase, size_t num_tones);
  /**
   * @brief Configure non-linearity test based on ramp waveform
   * @param c Configuration structure of test and waveform to generate
   * @param npts Input number desired for the waveform generated or contained
   * in the provided data
   * @param sample_rate Input Sample rate of the data converter
   * @param full_scale_range Input full-scale-range of the data converter
   * @param resolution Input resolution of the data converter
   * @param start Input start of the ramp waveform
   * @param stop Input stop of the ramp waveform
   * @param irnoise Input IR noise
   */
  __api void gn_config_ramp_nl_meas (gn_config *c, size_t npts,
                                     double sample_rate,
                                     double full_scale_range, int resolution,
                                     double start, double stop,
                                     double irnoise);

  /**
   * @brief Generate sinusoidal tone based on supplied configuration.
   * @param c Configuration structure of test and waveform to generate
   * @param result Output array of tone generated
   * @param len Output length of the generated tone in samples
   */
  __api int gn_gen_tone (gn_config c, double **result, size_t *len);
  /**
   * @brief Generate noise based on supplied configuration.
   * @param c Configuration structure of test and waveform to generate
   * @param result Output array of noise generated
   */
  __api void gn_gen_noise (gn_config c, double **result);
  /**
   * @brief Generate ramp based on supplied configuration.
   * @param c Configuration structure of test and waveform to generate
   * @param result Output array of ramp generated
   * @param len Output length of the generated tone in samples
   */
  __api void gn_gen_ramp (gn_config c, double **result, size_t *len);
  /**
   * @brief Quantize waveform based on supplied configuration.
   * @param c Configuration structure of test
   * @param awf Input waveform to quantize
   * @param result Output array of waveform quantized
   */
  __api void gn_quantize (gn_config c, const double *awf, int32_t **result);  
  /**
 * @brief Compute complex FFT of real waveform
 * @return 0 on success, non-zero otherwise
 * @param out: [out] Interleaved Re/Im output array pointer
 * @param out_size out_size: [in] Output array size
 * @param in in: [in] Input array pointer
 * @param in_size in_size: [in] Input array size
 * @param navg navg: [in] FFT averaging number
 * @param nfft nfft: [in] FFT size
 * @param window window: [in] Window
 * @param scale scale: [in] Scaling mode
 */
  __api int gn_rfft (double* out, size_t out_size, const double* in,
                     size_t in_size, size_t navg, size_t nfft,
                     GnWindow window, GnRfftScale scale);
  /**
 * @brief gn_fft
 * @return 0 on success, non-zero otherwise
 * @param out: [out] Interleaved Re/Im output array pointer
 * @param out_size: [in] Output array size
 * @param i: [in] In-phase input array pointer
 * @param i_size: [in] In-phase input array size
 * @param q: [in] Quadrature input array pointer
 * @param q_size: [in] Quadrature input array size
 * @param n: [in] Resolution
 * @param navg: [in] FFT averaging number
 * @param nfft: [in] FFT size
 * @param window: [in] Window
 */
/*
  __api int gn_fftq(double* out, size_t out_size, const int16_t* i,
                 size_t i_size, const int16_t* q, size_t q_size, int n, 
                 size_t navg, size_t nfft, GnWindow window);*/
  /**
   * @brief Compute desired data-converter performance metric
   * @param c Configuration structure of test
   * @param input Input quantized real waveform
   * @param m_name Input name of the performance metric
   * @param fft_re Output real part of FFT computed.
   * @param fft_im Output imaginary part of FFT computed.
   * @param len Length of the FFT generated
   * @param error_code Output error code. If 0, no error occurred.
   */
  __api double gn_metric (gn_config c, const void *input, const char *m_name,
                          double** fft_re, double** fft_im, size_t *len, unsigned int *error_code);
  /**
   * @brief Compute DNL of the data-converter
   * @param c Configuration structure of test
   * @param qwf Input quantized real waveform
   * @param bins Output bins of histogram of codes
   * @param hits Output frequency of bins
   * @param dnl_data Output DNL computed
   */
  __api void gn_compute_dnl (gn_config c, int *qwf, int32_t **bins,
                             uint64_t **hits, double **dnl_data);
  /**
   * @brief Compute INL of the data-converter
   * @param c Configuration structure of test
   * @param qwf Input quantized real waveform
   * @param bins Output bins of histogram of codes
   * @param hits Output frequency of bins
   * @param dnl_data Output DNL computed
   * @param inl_data Output INL computed
   */
  __api void gn_compute_inl (gn_config c, int *qwf, int32_t **bins,
                             uint64_t **hits, double **dnl_data,
                             double **inl_data);

#ifdef __cplusplus
}
#endif

#endif
