
#ifndef GENALYZER_ADVANCED_H
#define GENALYZER_ADVANCED_H
#include "cgenalyzer.h"
#include <fourier_analysis.hpp>

#ifdef __cplusplus
extern "C" {
#endif

  typedef struct fft_analysis_wrapper fft_analysis_wrapper;

  /**
   * @brief Create wrapper to fourier_analysis class constructor
   */
  __api fft_analysis_wrapper *gn_create_fft_analysis_wrapper ();

  /**
   * @brief Destroy wrapper to fourier_analysis class constructor
   * @param cfftobj Input wrapper to ftt_analysis2 class
   */
  __api void gn_destroy_fft_analysis_wrapper (fft_analysis_wrapper *cfftobj);

  // wrappers to fourier_analysis methods
  __api void add_fixed_tone (fft_analysis_wrapper *cfftobj);
  __api void add_max_tone (fft_analysis_wrapper *cfftobj,
                                      const char *tone_label);
  __api void set_analysis_band (fft_analysis_wrapper *cfftobj);
  __api void set_fdata (fft_analysis_wrapper *cfftobj,
                                   double fdata);
  __api void set_fsample (fft_analysis_wrapper *cfftobj, double fs);
  __api void set_fshift (fft_analysis_wrapper *cfftobj,
                                    double fshift);

  /**
   * @brief Compute desired data-converter performance metric
   * @param c Configuration structure of test
   * @param obj Input wrapper to ftt_analysis2 class
   * @param fft_data Input array of FFT of quantized real waveform
   * @param result_name Input name of the performance metric
   */
  __api double gn_compute_metric (gn_config c, fft_analysis_wrapper *obj,
                                  const double *fft_data,
                                  const char *result_name);

#ifdef __cplusplus
}
#endif

#endif
