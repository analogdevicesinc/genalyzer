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

    char *tmp_token_name;
    double* awf;
    unsigned int err_code, rvalue;
    size_t nfft, num_tones;
    int res, navg;
    double fs, fsr;
    double *freq, *scale, *phase;

    err_code = read_param(test_filename, "domain_wf", (void*)(&rvalue), UINT32);
    meas_domain domain_wf = rvalue;
    err_code = read_param(test_filename, "type_wf", (void*)(&rvalue), UINT32);
    waveform_type type_wf = rvalue;
    err_code = read_param(test_filename, "nfft", (void*)(&nfft), UINT64);
    err_code = read_param(test_filename, "num_tones", (void*)(&num_tones), UINT64);
    err_code = read_param(test_filename, "res", (void*)(&res), INT32);
    err_code = read_param(test_filename, "navg", (void*)(&navg), INT32);
    err_code = read_param(test_filename, "fs", (void*)(&fs), DOUBLE);
    err_code = read_param(test_filename, "fsr", (void*)(&fsr), DOUBLE);
    
    freq = (double*)calloc(num_tones, sizeof(double));
    scale = (double*)calloc(num_tones, sizeof(double));
    phase = (double*)calloc(num_tones, sizeof(double));
    tmp_token_name = (char*)malloc(10*sizeof(char));
    for (int n = 0; n < num_tones; n++) {
        sprintf(tmp_token_name, "freq%d", n);
        err_code = read_param(test_filename, tmp_token_name, (void*)(freq+n), DOUBLE);
        sprintf(tmp_token_name, "scale%d", n);
        err_code = read_param(test_filename, tmp_token_name, (void*)(scale+n), DOUBLE);
        sprintf(tmp_token_name, "phase%d", n);
        err_code = read_param(test_filename, tmp_token_name, (void*)(phase+n), DOUBLE);
    }

    double * ref_fft = (double*)malloc(2*nfft*sizeof(double));
    gn_config c = NULL;

    // configuration
    gn_config_tone_meas(&c,
        domain_wf,
        type_wf,
        nfft, // FFT order
        navg, // # of FFTs averaged
        fs, // sample rate
        fsr, // full-scale range
        res, // ADC resolution
        0,
        false,
        false,
        false);

    // read fft output waveform
    read_file_to_array(test_filename, (void*)ref_fft, DOUBLE);

    // compute metric
    double *fft_op_re, *fft_op_im;
    size_t fft_len;
    sfdr_val = gn_metric(c, ref_fft, "SFDR", &fft_op_re, &fft_op_im, &fft_len, &err_code);

    // compare
    printf("SFDR - %f\n", sfdr_val);
    // assert(floats_almost_equal(sfdr_val, sfdr_ref, 2));

    // free memory
    free(c);
    free(freq);
    free(scale);
    free(phase);
    free(ref_fft);
    free(tmp_token_name);
    
    return 0;
}
