#include "test_genalyzer.h"
#include <assert.h>
#include <cgenalyzer.h>
#include <stdbool.h>
#include <stdlib.h>

int main(int argc, const char* argv[])
{
    // read test waveform
    const char* test_filename_ip = argv[1];
    const char* test_filename_op = argv[2];

    char *tmp_token_name;
    double* awf;
    unsigned int err_code;
    meas_domain domain_wf;
    waveform_type type_wf;
    size_t nfft, num_tones, res;
    int navg;
    double fs, fsr;
    double *freq, *scale, *phase;
    err_code = read_param(test_filename_ip, "domain_wf", (void*)(&domain_wf), UINT64);
    err_code = read_param(test_filename_ip, "type_wf", (void*)(&type_wf), UINT64);
    err_code = read_param(test_filename_ip, "nfft", (void*)(&nfft), UINT64);
    err_code = read_param(test_filename_ip, "num_tones", (void*)(&num_tones), UINT64);
    err_code = read_param(test_filename_ip, "res", (void*)(&res), INT32);
    err_code = read_param(test_filename_ip, "navg", (void*)(&navg), INT32);
    err_code = read_param(test_filename_ip, "fs", (void*)(&fs), DOUBLE);
    err_code = read_param(test_filename_ip, "fsr", (void*)(&fsr), DOUBLE);    
    freq = (double*)calloc(num_tones, sizeof(double));
    scale = (double*)calloc(num_tones, sizeof(double));
    phase = (double*)calloc(num_tones, sizeof(double));
    tmp_token_name = (char*)malloc(10*sizeof(char));
    for (int n = 0; n < num_tones; n++) {
        sprintf(tmp_token_name, "freq%d", n);
        err_code = read_param(test_filename_ip, tmp_token_name, (void*)(freq+n), DOUBLE);
        sprintf(tmp_token_name, "scale%d", n);
        err_code = read_param(test_filename_ip, tmp_token_name, (void*)(scale+n), DOUBLE);
        sprintf(tmp_token_name, "phase%d", n);
        err_code = read_param(test_filename_ip, tmp_token_name, (void*)(phase+n), DOUBLE);
    }

    size_t npts;
    if (type_wf == 2)
        npts = 2 * nfft * navg;
    else
        npts = nfft * navg;

    int * ref_qwf_ip = (int*)malloc(npts*sizeof(int));
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
        2, // rectangular window
        false,
        false,
        false);

    // read quantized input waveform
    read_file_to_array(test_filename_ip, (void*)ref_qwf_ip, INT32);

    // compute rfft
    double *rfft_op_re, *rfft_op_im;
    size_t fft_size;
    gn_rfft(c, ref_qwf_ip, &rfft_op_re, &rfft_op_im, &fft_size);

    // read reference output waveform and deinterleave
    double * ref_rfft_op = (double*)malloc(((fft_size - 1) * 2)*sizeof(double));
    double * ref_rfft_op_re = (double*)malloc(fft_size*sizeof(double));
    double * ref_rfft_op_im = (double*)malloc(fft_size*sizeof(double));
    read_file_to_array(test_filename_op, (void*)ref_rfft_op, DOUBLE);
    deinterleave(ref_rfft_op, nfft, ref_rfft_op_re, ref_rfft_op_im, DOUBLE);

    // compare
    assert(float_arrays_almost_equal(ref_rfft_op_re, rfft_op_re, nfft / 2, 8));
    assert(float_arrays_almost_equal(ref_rfft_op_im, rfft_op_im, nfft / 2, 8));

    // free memory
    free(c);
    free(freq);
    free(scale);
    free(phase);
    free(ref_qwf_ip);
    free(ref_rfft_op);
    free(ref_rfft_op_re);
    free(ref_rfft_op_im);
    free(rfft_op_re);
    free(rfft_op_im);
    free(tmp_token_name);

    return 0;
}
