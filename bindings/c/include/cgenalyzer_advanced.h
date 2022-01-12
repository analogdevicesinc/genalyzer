
#ifndef GENALYZER_ADVANCED_H
#define GENALYZER_ADVANCED_H
#include "cgenalyzer.h"
// #include <code_density.hpp>
#include <fft_analysis2.hpp>
// #include <real_to_string.hpp>

#ifdef __cplusplus
extern "C" {
#endif

  typedef struct fft_analysis_wrapper fft_analysis_wrapper;

  /**
   * @brief Create wrapper to fft_analysis2 class constructor
   */
  __api fft_analysis_wrapper *gn_create_fft_analysis_wrapper ();

  /**
   * @brief Destroy wrapper to fft_analysis2 class constructor
   * @param cfftobj Input wrapper to ftt_analysis2 class
   */
  __api void gn_destroy_fft_analysis_wrapper (fft_analysis_wrapper *cfftobj);

  // wrappers to fft_analysis2 methods
  __api fft_analysis_wrapper *add_band (fft_analysis_wrapper *cfftobj);
  __api fft_analysis_wrapper *add_fixed_tone (fft_analysis_wrapper *cfftobj);
  __api fft_analysis_wrapper *add_max_tone (fft_analysis_wrapper *cfftobj,
                                      const char *tone_label);
  __api fft_analysis_wrapper *set_analysis_band (fft_analysis_wrapper *cfftobj);
  __api fft_analysis_wrapper *set_fdata (fft_analysis_wrapper *cfftobj,
                                   double fdata);
  __api fft_analysis_wrapper *set_fsample (fft_analysis_wrapper *cfftobj, double fs);
  __api fft_analysis_wrapper *set_fshift (fft_analysis_wrapper *cfftobj,
                                    double fshift);

  /**
   * @brief Compute desired data-converter performance metric
   * @param c Configuration structure of test
   * @param obj Input wrapper to ftt_analysis2 class
   * @param fft_data_re Input In-phase array of FFT of quantized real waveform
   * @param fft_data_im Input Quadrature-phase array of FFT of quantized real
   * waveform
   * @param result_name Input name of the performance metric
   */
  __api double gn_compute_metric (gn_config c, fft_analysis_wrapper *obj,
                                  const double *fft_data_re,
                                  const double *fft_data_im,
                                  const char *result_name);

#ifdef __cplusplus
}
#endif

#endif
