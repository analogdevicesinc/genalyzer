#include "test_genalyzer.h"
#include <assert.h>
#include <cgenalyzer.h>
#include <stdbool.h>
#include <stdlib.h>

int main(int argc, const char* argv[])
{
    // read test waveform
    const char* test_filename = argv[1];

    char *tmp_token_name;
    double* awf;
    int* qwf;
    unsigned int err_code;
    meas_domain domain_wf;
    waveform_type type_wf;
    size_t nfft, num_tones;
    int res, navg;
    double fs, fsr;
    double *freq, *scale, *phase;
    err_code = read_param(test_filename, "domain_wf", (void*)(&domain_wf), UINT64);
    err_code = read_param(test_filename, "type_wf", (void*)(&type_wf), UINT64);
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

    // configuration
    gn_config c = NULL;
    gn_config_tone_gen(&c,
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
        0, 
        false,
        false,
        false);

    // waveform generation
    size_t npts;
    gn_gen_tone(c, &awf, &npts);
    gn_quantize(c, awf, &qwf);

    // read reference waveform
    int* ref_qwf = (int*)calloc(npts, sizeof(int));
    read_file_to_array(test_filename, (void*)ref_qwf, INT32);

    // compare
    assert(int_arrays_almost_equal(qwf, ref_qwf, 1, npts, INT32));

    // free memory
    free(c);
    free(awf);
    free(qwf);
    free(ref_qwf);
    free(freq);
    free(scale);
    free(phase);
    free(tmp_token_name);

    return 0;
}
