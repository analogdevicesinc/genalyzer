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
    
    double* awf;
    unsigned int err_code;
    meas_domain domain_wf;
    waveform_type type_wf;
    size_t nfft;
    int res, navg;
    double fs, fsr;
    err_code = read_param(test_filename, "domain_wf", (void*)(&domain_wf), UINT32);
    err_code = read_param(test_filename, "type_wf", (void*)(&type_wf), UINT32);
    err_code = read_param(test_filename, "nfft", (void*)(&nfft), UINT64);
    err_code = read_param(test_filename, "res", (void*)(&res), INT32);
    err_code = read_param(test_filename, "navg", (void*)(&navg), INT32);
    err_code = read_param(test_filename, "fs", (void*)(&fs), DOUBLE);
    err_code = read_param(test_filename, "fsr", (void*)(&fsr), DOUBLE);

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
      free(in);
    }
    else {
      int * in = (int*)malloc(2*nfft*navg*sizeof(int));
      read_file_to_array(test_filename, (void*)in, INT32);
      double *fft_op_re, *fft_op_im;
      size_t fft_len;
      sfdr_val = gn_metric(c, in, "SFDR", &fft_op_re, &fft_op_im, &fft_len, &err_code);      
      // assert(floats_almost_equal(sfdr_val, 9.53, 2));
      free(in);
    }

    // compare
    printf("SFDR - %f\n", sfdr_val);
    printf("err - %u\n", err_code);

    // free memory
    free(c);

    return 0;
}
