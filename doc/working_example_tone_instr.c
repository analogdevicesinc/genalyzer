#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include "test_genalyzer.h"
#include "cgenalyzer.h"

int main(int argc, char *argv[]) {
  // read test waveform
  const char *test_filename_ip = argv[1];
  printf("%s\n", test_filename_ip);

  meas_domain domain_wf = atoll(extract_token(test_filename_ip, "domain_wf"));
  waveform_type type_wf = atoll(extract_token(test_filename_ip, "type_wf"));
  size_t nfft = atoll(extract_token(test_filename_ip, "nfft"));
  size_t num_tones = atoll(extract_token(test_filename_ip, "num_tones"));
  int res = atoi(extract_token(test_filename_ip, "res"));
  int navg = atoi(extract_token(test_filename_ip, "navg"));
  double fs = atof(extract_token(test_filename_ip, "fs"));
  double fdata = fs, fshift = fs;
  double fsr = atof(extract_token(test_filename_ip, "fsr"));
  double *freq = (double *)calloc(num_tones, sizeof(double));
  double *scale = (double *)calloc(num_tones, sizeof(double));
  double *phase = (double *)calloc(num_tones, sizeof(double));

  char tmp_token[10];
  for (int n = 0; n < num_tones; n++) {
    sprintf(tmp_token, "freq%d", n);
    freq[n] = atof(extract_token(test_filename_ip, tmp_token));
    sprintf(tmp_token, "scale%d", n);
    scale[n] = atof(extract_token(test_filename_ip, tmp_token));
    sprintf(tmp_token, "phase%d", n);
    phase[n] = atof(extract_token(test_filename_ip, tmp_token));
  }

  size_t npts = 2*nfft*navg;
  int qwf[npts];
  config c = NULL;

  // configuration
  config_tone_meas(&c,
                   domain_wf,
                   type_wf,
                   nfft, // FFT order
                   navg, // # of FFTs averaged
                   fs, // sample rate
                   fsr, // full-scale range
                   res, // ADC resolution: unused configuration setting
                   freq, // tone frequency, # of array elements = num_tones
                   scale, // tone scale, # of array elements = num_tones
                   phase, // tone phase, # of array elements = num_tones
                   num_tones, // # of tones
                   false,
                   false,
                   false
                   );

  // read quantized input waveform
  read_file_to_array(test_filename_ip, (void *) qwf, INT32);

  // compute metrics
  fsnr_val = metric(c, qwf, "FSNR");
  sfdr_val = metric(c, qwf, "SFDR");
  sinad_val = metric(c, qwf, "SINAD");

  free(awf);
  free(qwf);
  return 0;
}