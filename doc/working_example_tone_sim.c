#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include "cgenalyzer.h"

int main(int argc, char *argv[]) {
  // read configuration settings from a file
  const char *test_filename = argv[1];
  printf("%s\n", test_filename);

  // variables to hold configuration settings
  double *awf;
  int *qwf;
  size_t nfft = atoll(extract_token(test_filename, "nfft"));
  size_t num_tones = atoll(extract_token(test_filename, "num_tones"));
  int navg = atoi(extract_token(test_filename, "navg"));
  double fs = atof(extract_token(test_filename, "fs"));
  double fdata = fs, fshift = fs;
  double fsr = atof(extract_token(test_filename, "fsr"));
  double *freq = (double *)calloc(num_tones, sizeof(double));
  double *scale = (double *)calloc(num_tones, sizeof(double));
  double *phase = (double *)calloc(num_tones, sizeof(double));

  char tmp_token[10];
  for (int n = 0; n < num_tones; n++) {
    sprintf(tmp_token, "freq%d", n);
    freq[n] = atof(extract_token(test_filename, tmp_token));
    sprintf(tmp_token, "scale%d", n);
    scale[n] = atof(extract_token(test_filename, tmp_token));
    sprintf(tmp_token, "phase%d", n);
    phase[n] = atof(extract_token(test_filename, tmp_token));
  }

  // opaque config struct that will contain config settings
  config c = NULL;

  // configuration
  config_tone_meas(&c,
                   FREQ,
                   COMPLEX_EXP,
                   nfft,
                   navg,
                   fs,
                   fsr,
                   res,
                   &freq,
                   &scale,
                   &phase,
                   1,
                   update_fsample,
                   update_fdata,
                   update_fshift);

  // waveform generation
  gen_tone(c, &awf);
  quantize(c, awf, &qwf);

  // compute metrics
  fsnr_val = metric(c, qwf, "FSNR");
  sfdr_val = metric(c, qwf, "SFDR");
  sinad_val = metric(c, qwf, "SINAD");

  free(awf);
  free(qwf);
  return 0;
}