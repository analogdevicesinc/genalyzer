#include "test_genalyzer.h"
#include <assert.h>
#include <cgenalyzer.h>
#include <stdbool.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
    // read test waveform
    const char* test_filename_ip = argv[1];
    const char* test_filename_op = argv[2];

    unsigned int err_code;
    meas_domain domain_wf = atoll(extract_token(test_filename_ip, "domain_wf", &err_code));
    waveform_type type_wf = atoll(extract_token(test_filename_ip, "type_wf", &err_code));
    size_t nfft = atoll(extract_token(test_filename_ip, "nfft", &err_code));
    size_t num_tones = atoll(extract_token(test_filename_ip, "num_tones", &err_code));
    int res = atoi(extract_token(test_filename_ip, "res", &err_code));
    int navg = atoi(extract_token(test_filename_ip, "navg", &err_code));
    double fs = atof(extract_token(test_filename_ip, "fs", &err_code));
    double fdata = fs, fshift = fs;
    double fsr = atof(extract_token(test_filename_ip, "fsr", &err_code));
    double* freq = (double*)calloc(num_tones, sizeof(double));
    double* scale = (double*)calloc(num_tones, sizeof(double));
    double* phase = (double*)calloc(num_tones, sizeof(double));

    char tmp_token[10];
    for (int n = 0; n < num_tones; n++) {
        sprintf(tmp_token, "freq%d", n);
        freq[n] = atof(extract_token(test_filename_ip, tmp_token, &err_code));
        sprintf(tmp_token, "scale%d", n);
        scale[n] = atof(extract_token(test_filename_ip, tmp_token, &err_code));
        sprintf(tmp_token, "phase%d", n);
        phase[n] = atof(extract_token(test_filename_ip, tmp_token, &err_code));
    }

    size_t npts = 2 * nfft * navg;
    int ref_qwf_ip[npts], ref_qwf_ip_re[npts / 2], ref_qwf_ip_im[npts / 2];
    double ref_fft_op[2 * nfft], ref_fft_op_re[nfft], ref_fft_op_im[nfft];
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
        false,
        false,
        false);

    // read quantized input waveform
    read_file_to_array(test_filename_ip, (void*)ref_qwf_ip, INT32);
    deinterleave(ref_qwf_ip, npts, ref_qwf_ip_re, ref_qwf_ip_im, INT32);

    // read fft output waveform
    read_file_to_array(test_filename_op, (void*)ref_fft_op, DOUBLE);
    deinterleave(ref_fft_op, npts, ref_fft_op_re, ref_fft_op_im, DOUBLE);

    // compute rfft
    double *fft_op_re, *fft_op_im;
    size_t fft_size;
    fft(c, ref_qwf_ip_re, ref_qwf_ip_im, &fft_op_re, &fft_op_im, &fft_size);

    // compare
    assert(float_arrays_almost_equal(ref_fft_op_re, fft_op_re, nfft, 8));
    assert(float_arrays_almost_equal(ref_fft_op_im, fft_op_im, nfft, 8));

    // free memory
    free(c);
    free(freq);
    free(scale);
    free(phase);

    return 0;
}