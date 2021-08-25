#include "cgenalyzer.h"
#include "test_genalyzer.h"
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
    // read test waveform
    const char* test_filename = argv[1];

    double* awf;
    unsigned int err_code;
    meas_domain domain_wf = atoll(extract_token(test_filename, "domain_wf", &err_code));
    waveform_type type_wf = atoll(extract_token(test_filename, "type_wf", &err_code));
    size_t nfft = atoll(extract_token(test_filename, "nfft", &err_code));
    size_t num_tones = atoll(extract_token(test_filename, "num_tones", &err_code));
    int navg = atoi(extract_token(test_filename, "navg", &err_code));
    double fs = atof(extract_token(test_filename, "fs", &err_code));
    double fdata = fs, fshift = fs;
    double fsr = atof(extract_token(test_filename, "fsr", &err_code));
    double* freq = (double*)calloc(num_tones, sizeof(double));
    double* scale = (double*)calloc(num_tones, sizeof(double));
    double* phase = (double*)calloc(num_tones, sizeof(double));

    char tmp_token[10];
    for (int n = 0; n < num_tones; n++) {
        sprintf(tmp_token, "freq%d", n);
        freq[n] = atof(extract_token(test_filename, tmp_token, &err_code));
        sprintf(tmp_token, "scale%d", n);
        scale[n] = atof(extract_token(test_filename, tmp_token, &err_code));
        sprintf(tmp_token, "phase%d", n);
        phase[n] = atof(extract_token(test_filename, tmp_token, &err_code));
    }

    // configuration
    config c = NULL;
    config_tone_gen(&c,
        domain_wf,
        type_wf,
        nfft, // FFT order
        navg, // # of FFTs averaged
        fs, // sample rate
        fsr, // full-scale range
        0, // ADC resolution: unused configuration setting
        freq, // tone frequency, # of array elements = num_tones
        scale, // tone scale, # of array elements = num_tones
        phase, // tone phase, # of array elements = num_tones
        num_tones, // # of tones
        false,
        false,
        false);

    // waveform generation
    size_t npts;
    gen_tone(c, &awf, &npts);

    // read reference waveform
    double ref_awf[npts];
    read_file_to_array(test_filename, (void*)ref_awf, DOUBLE);

    // compare
    assert(float_arrays_almost_equal(ref_awf, awf, npts, 6));

    // free memory
    free(awf);
    free(freq);
    free(scale);
    free(phase);

    return 0;
}
