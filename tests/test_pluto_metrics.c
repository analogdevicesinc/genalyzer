#include "test_genalyzer.h"
#include <assert.h>
#include <cgenalyzer.h>
#include <stdbool.h>
#include <stdlib.h>

int main(int argc, const char* argv[])
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
    
    gn_config c = NULL;

    // configuration
    gn_config_tone_meas(&c,
        domain_wf,
        type_wf,
        nfft, // FFT order
        navg, // # of FFTs averaged
        fs, // sample rate
        fsr, // full-scale range
        res, // ADC resolution: unused configuration setting
        0, 
        true,
        true,
        true);

    // read quantized input waveform
    if (domain_wf) {
      double * in = (double*)malloc(2*nfft*sizeof(double));
      read_file_to_array(test_filename, (void*)in, DOUBLE);
      double *fft_op_re, *fft_op_im;
      size_t fft_len;
      sfdr_val = gn_metric(c, in, "SFDR", &fft_op_re, &fft_op_im, &fft_len, &err_code);      
      // assert(floats_almost_equal(sfdr_val, 9.53, 2));
    }
    else {
      int * in = (int*)malloc(2*nfft*navg*sizeof(int));
      read_file_to_array(test_filename, (void*)in, INT32);
      double *fft_op_re, *fft_op_im;
      size_t fft_len;
      sfdr_val = gn_metric(c, in, "SFDR", &fft_op_re, &fft_op_im, &fft_len, &err_code);      
      // assert(floats_almost_equal(sfdr_val, 9.53, 2));
      }

    // compare
    printf("SFDR - %f\n", sfdr_val);
    printf("err - %u\n", err_code);

    // free memory
    free(c);

    return 0;
}
