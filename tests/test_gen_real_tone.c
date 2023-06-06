#include "cgenalyzer.h"
#include "test_genalyzer.h"
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

int main(int argc, const char* argv[])
{
    // read test waveform filename
    const char* test_filename = argv[1];
    
    int err_code;
    double *awf, *ref_awf;

    // read parameters
    tone_type wf_type;
    unsigned long long npts, num_tones;
    double fs;
    double *freq, *scale, *phase;
    err_code = read_scalar_from_json_file(test_filename, "wf_type", (void*)(&wf_type), UINT64);
    if (err_code != 0)return err_code;
    err_code = read_scalar_from_json_file(test_filename, "npts", (void*)(&npts), UINT64);
    if (err_code != 0)return err_code;
    err_code = read_scalar_from_json_file(test_filename, "fs", (void*)(&fs), DOUBLE);
    if (err_code != 0)return err_code;
    err_code = read_scalar_from_json_file(test_filename, "num_tones", (void*)(&num_tones), UINT64);
    if (err_code != 0)return err_code;
    
    freq = (double*)calloc(num_tones, sizeof(double));
    scale = (double*)calloc(num_tones, sizeof(double));
    phase = (double*)calloc(num_tones, sizeof(double));
    if (num_tones > 1) {
        err_code = read_array_from_json_file(test_filename, "freq", freq, DOUBLE, num_tones);
        if (err_code != 0)return err_code;
        err_code = read_array_from_json_file(test_filename, "scale", scale, DOUBLE, num_tones);
        if (err_code != 0)return err_code;
        err_code = read_array_from_json_file(test_filename, "phase", phase, DOUBLE, num_tones);
        if (err_code != 0)return err_code;
    }
    else {
        err_code = read_scalar_from_json_file(test_filename, "freq", (void*)(freq), DOUBLE);
        if (err_code != 0)return err_code;
        err_code = read_scalar_from_json_file(test_filename, "scale", (void*)(scale), DOUBLE);
        if (err_code != 0)return err_code;
        err_code = read_scalar_from_json_file(test_filename, "phase", (void*)(phase), DOUBLE);
        if (err_code != 0)return err_code;
    }

    // configuration
    gn_config c = NULL;
    err_code = gn_config_gen_tone(wf_type, npts, fs, num_tones, freq, scale, phase, &c);
    if (err_code != 0)return err_code;

    // waveform generation
    err_code = gn_gen_real_tone(&awf, &c);
    if (err_code != 0)return err_code;

    // read reference waveform
    ref_awf = (double*)malloc(npts*sizeof(double));
    err_code = read_array_from_json_file(test_filename, "test_vec", ref_awf, DOUBLE, npts);
    if (err_code != 0)return err_code;

    // compare
    assert(float_arrays_almost_equal(ref_awf, awf, npts, 6));    

    // free memory
    free(awf);
    free(freq);
    free(scale);
    free(phase);
    free(ref_awf);
    gn_config_free(&c);

    return 0;
}
