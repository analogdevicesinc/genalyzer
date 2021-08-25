#include "test_genalyzer.h"
#include <assert.h>
#include <cgenalyzer.h>
#include <stdbool.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
    // read test waveform
    const char* test_filename = argv[1];
    double sfdr_val;

    unsigned int err_code;
    meas_domain domain_wf = atoll(extract_token(test_filename, "domain_wf", &err_code));
    waveform_type type_wf = atoll(extract_token(test_filename, "type_wf", &err_code));
    size_t nfft = atoll(extract_token(test_filename, "nfft", &err_code));
    int res = atoi(extract_token(test_filename, "res", &err_code));
    int navg = atoi(extract_token(test_filename, "navg", &err_code));
    double fs = atof(extract_token(test_filename, "fs", &err_code));
    double fsr = atof(extract_token(test_filename, "fsr", &err_code));
    size_t npts = nfft;

    double qwf[2 * npts];
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
        true,
        true,
        true);

    // read quantized input waveform
    read_file_to_array(test_filename, (void*)qwf, INT32);
    int qwf_re[npts], qwf_im[npts];
    deinterleave(qwf, 2 * npts, qwf_re, qwf_im, INT32);
    /*
  for (int i = 0; i < npts; i++)
printf("%d\t%d\n", qwf_re[i], qwf_im[i]);
  
  // compute rfft
  double *fft_op_re, *fft_op_im;  
  size_t fft_size;
  fft(c, qwf_re, qwf_im, &fft_op_re, &fft_op_im, &fft_size);
  /*
  for (int i = 0; i < fft_size; i++)
  printf("%f\t%f\n", fft_op_re[i], fft_op_im[i]);*/

    // compute metric
    sfdr_val = metric(c, qwf, "SFDR", &err_code);

    // compare
    printf("SFDR - %f\n", sfdr_val);
    printf("err - %u\n", err_code);

    // free memory
    free(c);

    return 0;
}
