/*
 * cgenalyzer - genalyzer API header file
 *
 * Copyright (C) 2022 Analog Devices, Inc.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * */


#ifndef CGENALYZER_H
#define CGENALYZER_H
#include "cgenalyzer_advanced.h"
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
  
  // opaque pointer
  typedef struct gn_config_private *gn_config;

  typedef enum tone_type
  {
      REAL_COSINE,
      REAL_SINE,
      COMPLEX_EXP
  } tone_type;

  /**
   * @brief free memory for configuration struct
   * @return 0 on success, non-zero otherwise
   * @param c genalyzer Configuration struct
   */
  __api int gn_config_free(
            gn_config *c
        );

  /**
   * @brief set configuration struct member: tone_type
   * @return 0 on success, non-zero otherwise
   * @param ttype ENUM value to indicate input tone type. Options: REAL_COSINE, REAL_SINE, COMPLEX_EXP    
   * @param c genalyzer Configuration struct
   */
  __api int gn_config_set_ttype(
            tone_type ttype, 
            gn_config *c
        );

  /**
   * @brief set configuration struct member: npts
   * @return 0 on success, non-zero otherwise
   * @param npts Number of sample points in the generated waveform
   * @param c genalyzer Configuration struct
   */
  __api int gn_config_set_npts(
            size_t npts, 
            gn_config *c
        );

  /**
   * @brief get configuration struct member: npts
   * @return 0 on success, non-zero otherwise
   * @param npts Number of sample points in the generated waveform
   * @param c genalyzer Configuration struct
   */
  __api int gn_config_get_npts(
            size_t *npts, 
            gn_config *c
        );

  /**
   * @brief set configuration struct member: sample_rate
   * @return 0 on success, non-zero otherwise
   * @param sample_rate Input Sample rate of the data converter
   * @param c genalyzer Configuration struct
   */
  __api int gn_config_set_sample_rate(
            double sample_rate, 
            gn_config *c
        );

  /**
   * @brief get configuration struct member: sample_rate
   * @return 0 on success, non-zero otherwise
   * @param sample_rate Input Sample rate of the data converter
   * @param c genalyzer Configuration struct
   */
  __api int gn_config_get_sample_rate(
            double *sample_rate, 
            gn_config *c
        );

  /**
   * @brief set configuration struct member: data_rate
   * @return 0 on success, non-zero otherwise
   * @param data_rate Input data rate of the data converter
   * @param c genalyzer Configuration struct
   */
  __api int gn_config_set_data_rate(
            double data_rate, 
            gn_config *c
        );

  /**
   * @brief set configuration struct member: shift_freq
   * @return 0 on success, non-zero otherwise
   * @param shift_freq Shift frequency of the data converter
   * @param c genalyzer Configuration struct
   */
  __api int gn_config_set_shift_freq(
            double shift_freq, 
            gn_config *c
        );

  /**
   * @brief set configuration struct member: num_tones
   * @return 0 on success, non-zero otherwise
   * @param num_tones Number of tones to generate
   * @param c genalyzer Configuration struct
   */
  __api int gn_config_set_num_tones(
            size_t num_tones, 
            gn_config *c
        );

  /**
   * @brief set configuration struct member: tone_freq
   * @return 0 on success, non-zero otherwise
   * @param tone_freq Input array of tone frequencies to generate
   * @param c genalyzer Configuration struct
   */
  __api int gn_config_set_tone_freq(
            double *tone_freq, 
            gn_config *c
        );

  /**
   * @brief set configuration struct member: tone_ampl
   * @return 0 on success, non-zero otherwise
   * @param tone_ampl Input array of tone scales to generate
   * @param c genalyzer Configuration struct
   */
  __api int gn_config_set_tone_ampl(
            double *tone_ampl, 
            gn_config *c
        );

  /**
   * @brief set configuration struct member: tone_phase
   * @return 0 on success, non-zero otherwise
   * @param tone_phase Input array of tone phases to generate
   * @param c genalyzer Configuration struct
   */
  __api int gn_config_set_tone_phase(
            double *tone_phase, 
            gn_config *c
        );

  /**
   * @brief set configuration struct member: fsr
   * @return 0 on success, non-zero otherwise
   * @param fsr Full-scale range of the waveform
   * @param c genalyzer Configuration struct
   */
  __api int gn_config_set_fsr(
            double fsr, 
            gn_config *c
        );

  /**
   * @brief set configuration struct member: qres
   * @return 0 on success, non-zero otherwise
   * @param qres Quantization resolution
   * @param c genalyzer Configuration struct
   */
  __api int gn_config_set_qres(
            int qres, 
            gn_config *c
        );

  /**
   * @brief set configuration struct member: noise_rms
   * @return 0 on success, non-zero otherwise
   * @param qres Quantization noise
   * @param c genalyzer Configuration struct
   */
  __api int gn_config_set_noise_rms(
            double noise_rms, 
            gn_config *c
        );

  /**
   * @brief set configuration struct member: code_format
   * @return 0 on success, non-zero otherwise
   * @param code_format Code format of data
   * @param c genalyzer Configuration struct
   */
  __api int gn_config_set_code_format(
            GnCodeFormat code_format, 
            gn_config *c
        );

  /**
   * @brief set configuration struct member: nfft
   * @return 0 on success, non-zero otherwise
   * @param nfft FFT order
   * @param c genalyzer Configuration struct
   */
  __api int gn_config_set_nfft(
            size_t nfft, 
            gn_config *c
        );

  /**
   * @brief get configuration struct member: nfft
   * @return 0 on success, non-zero otherwise
   * @param nfft FFT order
   * @param c genalyzer Configuration struct
   */
  __api int gn_config_get_nfft(
            size_t *nfft, 
            gn_config *c
        );
    
  /**
   * @brief set configuration struct member: navg
   * @return 0 on success, non-zero otherwise
   * @param navg Num. of FFTs to average
   * @param c genalyzer Configuration struct
   */
  __api int gn_config_set_fft_navg(
            size_t fft_navg, 
            gn_config *c
        );
    
  /**
   * @brief set configuration struct member: win
   * @return 0 on success, non-zero otherwise
   * @param win Window function used
   * @param c genalyzer Configuration struct
   */
  __api int gn_config_set_win(
            GnWindow win, 
            gn_config *c
        );
    
  /**
   * @brief set configuration struct member: ssb_fund
   * @return 0 on success, non-zero otherwise
   * @param ssb_fund Single side bin fundamental
   * @param c genalyzer Configuration struct
   */
  __api int gn_config_set_ssb_fund(
            int ssb_fund, 
            gn_config *c
        );

  /**
   * @brief set configuration struct member: ssb_rest
   * @return 0 on success, non-zero otherwise
   * @param ssb_rest Single side bins rest
   * @param c genalyzer Configuration struct
   */
  __api int gn_config_set_ssb_rest(
            int ssb_rest, 
            gn_config *c
        );

  /**
   * @brief set configuration struct member: max_harm_order
   * @return 0 on success, non-zero otherwise
   * @param max_harm_order Max order of harmonic
   * @param c genalyzer Configuration struct
   */
  __api int gn_config_set_max_harm_order(
            int max_harm_order, 
            gn_config *c
        );

  /**
   * @brief set configuration struct member: dnla_signal_type
   * @return 0 on success, non-zero otherwise
   * @param dnla_signal_type DNL analysis signal type
   * @param c genalyzer Configuration struct
   */
  __api int gn_config_set_dnla_signal_type(
            GnDnlSignal dnla_signal_type, 
            gn_config *c
        );

  /**
   * @brief set configuration struct member: inla_fit
   * @return 0 on success, non-zero otherwise
   * @param inla_fit INL analysis line fit
   * @param c genalyzer Configuration struct
   */
  __api int gn_config_set_inla_fit(
            GnInlLineFit inla_fit, 
            gn_config *c
        );

  /**
   * @brief set configuration struct member: ramp_start
   * @return 0 on success, non-zero otherwise
   * @param ramp_start start value of ramp
   * @param c genalyzer Configuration struct
   */
  __api int gn_config_set_ramp_start(
            double ramp_start, 
            gn_config *c
        );
    
  /**
   * @brief set configuration struct member: ramp_stop
   * @return 0 on success, non-zero otherwise
   * @param ramp_stop stop value of ramp
   * @param c genalyzer Configuration struct
   */
  __api int gn_config_set_ramp_stop(
            double ramp_stop, 
            gn_config *c
        );    
    
  /**
   * @brief get configuration struct member: _code_density_size
   * @return 0 on success, non-zero otherwise
   * @param ramp_stop stop value of ramp
   * @param c genalyzer Configuration struct
   */
  __api int gn_config_get_code_density_size(
            size_t *code_density_size, 
            gn_config *c
        );

  /**
   * @brief Configure tone parameters to be used in measurement
   * @return 0 on success, non-zero otherwise
   * @param ttype ENUM value to indicate input tone type. Options: REAL_COSINE, REAL_SINE, COMPLEX_EXP    
   * @param npts Number of sample points in the generated waveform
   * @param sample_rate Input Sample rate of the data converter
   * @param num_tones Number of tones to generate
   * @param tone_freq Input array of tone frequencies to generate
   * @param tone_ampl Input array of tone scales to generate
   * @param tone_phase Input array of tone phases to generate   
   * @param c Configuration struct containing tone parameters
   */
  __api int gn_config_gen_tone (
            tone_type ttype, 
            size_t npts,
            double sample_rate, 
            size_t num_tones, 
            double *tone_freq, 
            double *tone_ampl,
            double *tone_phase, 
            gn_config *c
        );

  /**
   * @brief Configure tone parameters to be used in measurement
   * @return 0 on success, non-zero otherwise
   * @param npts Number of sample points in the generated waveform
   * @param ramp_start Input start value of ramp
   * @param ramp_stop Input stop value of ramp
   * @param c Configuration struct containing ramp parameters
   */
  __api int gn_config_gen_ramp(
            size_t npts, 
            double ramp_start, 
            double ramp_stop, 
            gn_config *c
        );

  /**
   * @brief Configure quantization parameters to be used in measurement
   * @return 0 on success, non-zero otherwise
   * @param npts Number of sample points in the generated waveform
   * @param fsr Full-scale range of the waveform    
   * @param qres Quantization resolution
   * @param qnoise Quantization noise
   * @param c Configuration structure
   */
  __api int gn_config_quantize(
            size_t npts, 
            double fsr,
            int qres, 
            double qnoise, 
            gn_config *c
        );

  /**
   * @brief Configure parameters to compute histogram
   * @return 0 on success, non-zero otherwise
   * @param npts Number of sample points in the generated waveform
   * @param qres Quantization resolution
   * @param c Configuration structure
   */
  __api int gn_config_histz_nla(
            size_t npts, 
            int qres, 
            gn_config *c
        );

  /**
   * @brief Configure FFT parameters 
   * @return 0 on success, non-zero otherwise
   */
  __api int gn_config_fftz(
            size_t npts,                        ///< [npts] Number of sample points in the input waveform
            int qres,                           ///< [qres] Quantization resolution
            size_t navg,                        ///< [navg] Number of FFT averages
            size_t nfft,                        ///< [nfft] FFT order
            GnWindow win,                       ///< [win] Window function to apply, Options: GnWindowBlackmanHarris, GnWindowHann, GnWindowNoWindow
            gn_config *c                         ///< [c] Configuration structure containing test parameters
        );

  /**
   * @brief Generate sinusoidal tone based on supplied configuration.
   * @return 0 on success, non-zero otherwise 
   */
  __api int gn_config_fa(
            double fixed_tone_freq,    ///< [fixed_tone_freq] Fixed tone frequency
            gn_config *c                ///< [c] Configuration structure containing test parameters            
        );
  
  /**
   * @brief Generate ramp based on supplied configuration.
   * @param out Output array of ramp generated
   * @param c Configuration structure of test and waveform to generate
   */
  __api int gn_gen_ramp(
            double **out, 
            gn_config *c
        );
  
  /**
   * @brief Generate sinusoidal tone based on supplied configuration.
   * @return 0 on success, non-zero otherwise   
   * @param out Output array of generated tone
   * @param c Configuration structure containing test parameters
   */
  __api int gn_gen_real_tone(
            double **out,
            gn_config *c
        );

  /**
   * @brief Generate sinusoidal tone based on supplied configuration.
   * @return 0 on success, non-zero otherwise   
   * @param outi In-phase output array of generated tone
   * @param outq Quadrature output array of generated tone
   * @param c Configuration structure containing test parameters
   */
  __api int gn_gen_complex_tone(
            double **outi, 
            double **outq,
            gn_config *c
        );

  /**
   * @brief Quantize waveform based on supplied configuration.
   * @return 0 on success, non-zero otherwise
   */
  __api int gn_quantize(
            int32_t **out,            ///< [out] Quantized output waveform   
            const double *in,         ///< [in] Input waveform to be quantized 
            gn_config *c               ///< [c] Configuration structure containing test parameters
        );
  /**
 * @brief Compute FFT of quantized input waveform
 * @return 0 on success, non-zero otherwise
 */
  __api int gn_fftz(
            double **out,             ///< [out] Interleaved Re/Im FFT output
            const int32_t *in_i,      ///< [in_i] In-phase input
            const int32_t *in_q,      ///< [in_q] Quadrature input
            gn_config *c               ///< [c] Configuration structure containing test parameters
        );

  /**
   * @brief Compute histogram of quantized waveform
   * @return 0 on success, non-zero otherwise
   */
  __api int gn_histz(
            uint64_t **hist,                    ///< [hist] Output - Histogram of input quantized waveform
            size_t *hist_len,                   ///< [hist_len] Output - Histogram size
            const int32_t *qwf,                 ///< [qwf] Input - Quantized input waveform  
            gn_config *c                         ///< [c] Input - Configuration structure containing test parameters
        );

  /**
   * @brief Compute histogram of quantized waveform
   * @return 0 on success, non-zero otherwise
   */
  __api int gn_dnlz(
            double **dnl, 
            size_t *dnl_len, 
            const uint64_t *hist, 
            gn_config *c
        );

  /**
   * @brief Compute histogram of quantized waveform
   * @return 0 on success, non-zero otherwise
   */
  
  __api int gn_inlz(
            double **inl, 
            size_t *inl_len, 
            const double *dnl, 
            gn_config *c
        );
  
  /**
   * @brief Do waveform analysis and all get results
   * @return 0 on success, non-zero otherwise
   */
  __api int gn_get_wfa_results(
            char ***rkeys, 
            double **rvalues, 
            size_t *results_size,               ///< [results_size] size of results 
            const int32_t *qwf,                 ///< [qwf] Input - Quantized input array pointer 
            gn_config *c                         ///< [c] Input - Configuration structure containing test parameters
        );

    /**
   * @brief Do histogram analysis and get results
   * @return 0 on success, non-zero otherwise
   */
  __api int gn_get_ha_results(
            char ***rkeys,                      ///< [rkeys] Output - Result keys 
            double **rvalues,                   ///< [rvalues] Output - Result values
            size_t *results_size,               ///< [results_size] Output - Size of results  
            const uint64_t *hist,               ///< [hist] Input - Histogram input to be analyzed 
            gn_config *c                         ///< [c] Input - Configuration structure containing test parameters
        );

  /**
   * @brief Do DNL analysis and get results
   * @return 0 on success, non-zero otherwise
   */
  __api int gn_get_dnla_results(
            char ***rkeys,                      ///< [rkeys] Output - Result keys  
            double **rvalues,                   ///< [rvalues] Output - Result values 
            size_t *results_size,               ///< [results_size] Output - Size of results   
            const double *dnl,                  ///< [dnl] Input - DNL input to be analyzed  
            gn_config *c                         ///< [c] Input - Configuration structure containing test parameters
        );

  /**
   * @brief Do INL analysis and get results
   * @return 0 on success, non-zero otherwise
   */
  __api int gn_get_inla_results(
          char ***rkeys,                      ///< [rkeys] Output - Result keys  
          double **rvalues,                   ///< [rvalues] Output - Result values 
          size_t *results_size,               ///< [results_size] Output - Size of results   
          const double *inl,                  ///< [dnl] Input - INL input to be analyzed  
          gn_config *c                         ///< [c] Input - Configuration structure containing test parameters
      );

  /**
   * @brief Do Fourier analysis and get a single result
   * @return 0 on success, non-zero otherwise
   */
  __api int gn_get_fa_single_result(
            double *rvalue,
            const char* metric_name,  
            double *fft_ilv,                    ///< [fft_ilv] Input - Interleaved Re/Im array pointer 
            gn_config *c                         ///< [c] Input - Configuration structure containing test parameters
        );

  /**
   * @brief Do Fourier analysis and all get results
   * @return 0 on success, non-zero otherwise
   */
  __api int gn_get_fa_results(
            char ***rkeys,
            double **rvalues,  
            size_t *results_size,               ///< [results_size] size of results
            double *fft_ilv,                    ///< [fft_ilv] Input - Interleaved Re/Im array pointer
            gn_config *c                         ///< [c] Input - Configuration structure containing test parameters
        );
  
#ifdef __cplusplus
}
#endif

#endif
