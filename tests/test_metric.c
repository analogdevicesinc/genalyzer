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
    size_t num_tones = atoll(extract_token(test_filename, "num_tones", &err_code));
    int res = atoi(extract_token(test_filename, "res", &err_code));
    int navg = atoi(extract_token(test_filename, "navg", &err_code));
    double fs = atof(extract_token(test_filename, "fs", &err_code));
    double fdata = fs, fshift = fs;
    double fsr = atof(extract_token(test_filename, "fsr", &err_code));
    double* freq = (double*)calloc(num_tones, sizeof(double));
    double* scale = (double*)calloc(num_tones, sizeof(double));
    double* phase = (double*)calloc(num_tones, sizeof(double));

    char * tmp_token = (char*)malloc(10*sizeof(char));
    for (int n = 0; n < num_tones; n++) {
        sprintf(tmp_token, "freq%d", n);
        freq[n] = 0.0; //atof(extract_token(test_filename, tmp_token, &err_code));
        sprintf(tmp_token, "scale%d", n);
        scale[n] = 0.0; //atof(extract_token(test_filename, tmp_token, &err_code));
        sprintf(tmp_token, "phase%d", n);
        phase[n] = 0.0; //atof(extract_token(test_filename, tmp_token, &err_code));
    }

    size_t npts = 2 * nfft * navg;
    double * ref_fft = (double*)malloc(2*nfft*sizeof(double));
    double * ref_fft_re = (double*)malloc(nfft*sizeof(double));
    double * ref_fft_im = (double*)malloc(nfft*sizeof(double));
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
        false,
        false,
        false);

    // read fft output waveform
    read_file_to_array(test_filename, (void*)ref_fft, DOUBLE);

    // compute metric
    sfdr_val = gn_metric(c, ref_fft, "SFDR", &err_code);

    // compare
    printf("SFDR - %f\n", sfdr_val);

    // free memory
    free(c);
    free(freq);
    free(scale);
    free(phase);
    free(ref_fft);
    free(ref_fft_im);
    free(ref_fft_re);

    return 0;
}
