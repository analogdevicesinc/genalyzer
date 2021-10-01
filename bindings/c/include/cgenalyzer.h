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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * */


#ifndef GENALYZER_CWRAPPER_H
#define GENALYZER_CWRAPPER_H
//#include <complex.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif
typedef struct fft_analysis_wrapper fft_analysis_wrapper;
typedef enum waveform_type { REAL_COSINE,
    REAL_SINE,
    COMPLEX_EXP,
    REAL_NOISE,
    COMPLEX_NOISE,
    RAMP } waveform_type;
typedef enum meas_domain { TIME,
    FREQ } meas_domain;
typedef enum int_repr { I32,
    I64 } int_repr;

// opaque pointer
typedef struct config_s* config;
typedef config config_tone;

/**
   * @brief Configure test based on real sinusoidal or complex exponential tones
   * @param c Configuration structure of test and waveform to generate
   * @param meas_domain Input ENUM value to indicate time/frequency domain of input waveform
   * @param wf_type Input ENUM value to indicate input waveform type as real or complex
   * @param fft_order Input FFT order desired for the waveform generated or contained in the provided FFT data
   * @param num_avgs Input number of FFT averages desired for the waveform generated or contained in the provided FFT data 
   * @param sample_rate Input Sample rate of the data converter
   * @param full_scale_range Input full-scale-range of the data converter
   * @param resolution Input resolution of the data converter
   * @param tone_freq Input array of tone frequencies to generate
   * @param tone_ampl Input array of tone scales to generate
   * @param tone_phase Input array of tone phases to generate
   * @param num_tones Input number of tones to generate
   * @param fsample_update Input Boolean value to update fsample
   * @param fdata_update Input Boolean value to update fdata
   * @param fshift_update Input Boolean value to update fshift
   */
void config_tone_gen(config_tone* c, meas_domain m_domain, waveform_type wf_type, size_t fft_order, int num_avgs, double sample_rate, double full_scale_range, int resolution, double* tone_freq, double* tone_ampl, double* tone_phase, size_t num_tones, bool fsample_update, bool fdata_update, bool fshift_update);
/**
   * @brief Configure test based on real sinusoidal or complex exponential tones
   * @param c Configuration structure of test and waveform to generate
   * @param meas_domain Input ENUM value to indicate time/frequency domain of input waveform
   * @param wf_type Input ENUM value to indicate input waveform type as real or complex
   * @param fft_order Input FFT order desired for the waveform generated or contained in the provided FFT data
   * @param num_avgs Input number of FFT averages desired for the waveform generated or contained in the provided FFT data 
   * @param sample_rate Input Sample rate of the data converter
   * @param full_scale_range Input full-scale-range of the data converter
   * @param resolution Input resolution of the data converter
   * @param fsample_update Input Boolean value to update fsample
   * @param fdata_update Input Boolean value to update fdata
   * @param fshift_update Input Boolean value to update fshift
   */
void config_tone_meas(config_tone* c, meas_domain m_domain, waveform_type wf_type, size_t fft_order, int num_avgs, double sample_rate, double full_scale_range, int resolution, bool fsample_update, bool fdata_update, bool fshift_update);
/**
   * @brief Configure test based on noise waveform
   * @param c Configuration structure of test and waveform to generate
   * @param wf_type Input ENUM value to indicate input waveform type as real or complex
   * @param fft_order Input FFT order desired for the waveform generated or contained in the provided FFT data
   * @param num_avgs Input number of FFT averages desired for the waveform generated or contained in the provided FFT data 
   * @param sample_rate Input Sample rate of the data converter
   * @param full_scale_range Input full-scale-range of the data converter
   * @param resolution Input resolution of the data converter
   * @param noise_pwr_lvl Input noise power of the data converter
   * @param fsample_update Input Boolean value to update fsample
   * @param fdata_update Input Boolean value to update fdata
   * @param fshift_update Input Boolean value to update fshift
   */
void config_noise_meas(config* c, waveform_type wf_type, size_t fft_order, int num_avgs, double sample_rate, double full_scale_range, int resolution, double noise_pwr_lvl, bool fsample_update, bool fdata_update, bool fshift_update);
/**
   * @brief Configure non-linearity test based on real sinusoidal tone
   * @param c Configuration structure of test and waveform to generate
   * @param wf_type Input ENUM value to indicate input waveform type as cosine real or sine real
   * @param npts Input number desired for the waveform generated or contained in the provided data
   * @param sample_rate Input Sample rate of the data converter
   * @param full_scale_range Input full-scale-range of the data converter
   * @param resolution Input resolution of the data converter
   * @param tone_freq Input array of tone frequencies to generate
   * @param tone_ampl Input array of tone scales to generate
   * @param tone_phase Input array of tone phases to generate
   * @param num_tones Input number of tones to generate
   */
void config_tone_nl_meas(config* c, waveform_type wf_type, size_t npts, double sample_rate, double full_scale_range, int resolution, double* tone_freq, double* tone_ampl, double* tone_phase, size_t num_tones);
/**
   * @brief Configure non-linearity test based on ramp waveform
   * @param c Configuration structure of test and waveform to generate
   * @param npts Input number desired for the waveform generated or contained in the provided data
   * @param sample_rate Input Sample rate of the data converter
   * @param full_scale_range Input full-scale-range of the data converter
   * @param resolution Input resolution of the data converter
   * @param start Input start of the ramp waveform
   * @param stop Input stop of the ramp waveform
   * @param irnoise Input IR noise
   */
void config_ramp_nl_meas(config* c, size_t npts, double sample_rate, double full_scale_range, int resolution, double start, double stop, double irnoise);

/**
   * @brief Generate sinusoidal tone based on supplied configuration.
   * @param c Configuration structure of test and waveform to generate
   * @param result Output array of tone generated
   */
void gen_tone(config c, double** result, size_t* len);
/**
   * @brief Generate noise based on supplied configuration.
   * @param c Configuration structure of test and waveform to generate
   * @param result Output array of noise generated
   */
void gen_noise(config c, double** result);
/**
   * @brief Generate ramp based on supplied configuration.
   * @param c Configuration structure of test and waveform to generate
   * @param result Output array of ramp generated
   */
void gen_ramp(config c, double** result, size_t* len);
/**
   * @brief Quantize waveform based on supplied configuration.
   * @param c Configuration structure of test 
   * @param awf Input waveform to quantize
   * @param result Output array of waveform quantized
   */
void quantize(config c, const double* awf, int32_t** result);
void quantize64(config c, double* awf, int64_t** result);
/**
   * @brief Compute complex FFT of real waveform based on supplied configuration.
   * @param c Configuration structure of test 
   * @param fft_cplx_re Output In-phase array of FFT of Input quantized real waveform
   * @param fft_cplx_re Output Quadrature-phase array of FFT of Input quantized real waveform
   * @param qwf Input real quantized waveform
   */
void rfft(config c, int32_t* qwf, double** fft_cplx_re, double** fft_cplx_im, size_t* len);

/**
   * @brief Compute complex FFT of real waveform based on supplied configuration.
   * @param c Configuration structure of test 
   * @param fft_cplx_re Output In-phase array of FFT of quantized real waveform
   * @param fft_cplx_re Output Quadrature-phase array of FFT of quantized real waveform
   * @param qwf_i Input real part of quantized waveform
   * @param qwf_q Input imaginary part of quantized waveform
   */
void fft(config c, int32_t* qwf_i, int32_t* qwf_q, double** fft_cplx_re, double** fft_cplx_im, size_t* len);

/**
   * @brief Create wrapper to fft_analysis2 class constructor
   */
fft_analysis_wrapper* create_fft_analysis_wrapper();

/**
   * @brief Destroy wrapper to fft_analysis2 class constructor
   * @param cfftobj Input wrapper to ftt_analysis2 class
   */
void destroy_fft_analysis_wrapper(fft_analysis_wrapper* cfftobj);

// wrappers to fft_analysis2 methods
fft_analysis_wrapper* add_band(fft_analysis_wrapper* cfftobj);
fft_analysis_wrapper* add_fixed_tone(fft_analysis_wrapper* cfftobj);
fft_analysis_wrapper* add_max_tone(fft_analysis_wrapper* cfftobj, const char* tone_label);
fft_analysis_wrapper* set_analysis_band(fft_analysis_wrapper* cfftobj);
fft_analysis_wrapper* set_fdata(fft_analysis_wrapper* cfftobj, double fdata);
fft_analysis_wrapper* set_fsample(fft_analysis_wrapper* cfftobj, double fs);
fft_analysis_wrapper* set_fshift(fft_analysis_wrapper* cfftobj, double fshift);

/**
   * @brief Compute desired data-converter performance metric
   * @param c Configuration structure of test 
   * @param obj Input wrapper to ftt_analysis2 class
   * @param fft_data_re Input In-phase array of FFT of quantized real waveform
   * @param fft_data_re Input Quadrature-phase array of FFT of quantized real waveform
   * @param result_name Input name of the performance metric
   */
double compute_metric(config c, fft_analysis_wrapper* obj, const double* fft_data_re, const double* fft_data_im, const char* result_name);
/**
   * @brief Compute desired data-converter performance metric
   * @param c Configuration structure of test 
   * @param input Input quantized real waveform
   * @param m_name Input name of the performance metric
   */
double metric(config c, const void* input, const char* m_name, unsigned int* error_code);
/**
   * @brief Compute DNL of the data-converter
   * @param c Configuration structure of test 
   * @param qwf Input quantized real waveform
   * @param bins Output bins of histogram of codes
   * @param hits Output frequency of bins
   * @param dnl_data Output DNL computed
   */
void compute_dnl(config c, int* qwf, int32_t** bins, int64_t** hits, double** dnl_data);
/**
   * @brief Compute INL of the data-converter
   * @param c Configuration structure of test 
   * @param qwf Input quantized real waveform
   * @param bins Output bins of histogram of codes
   * @param hits Output frequency of bins
   * @param dnl_data Output DNL computed
   * @param inl_data Output INL computed
   */
void compute_inl(config c, int* qwf, int32_t** bins, int64_t** hits, double** dnl_data, double** inl_data);

#ifdef __cplusplus
}
#endif

#endif
