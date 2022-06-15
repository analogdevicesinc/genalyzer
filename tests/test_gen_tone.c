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
    waveform_type wf_type;
    unsigned int npts, num_tones;
    double fs;
    double *freq, *scale, *phase;
    err_code = read_scalar_from_json_file(test_filename, "wf_type", (void*)(&wf_type), UINT64);
    err_code = read_scalar_from_json_file(test_filename, "npts", (void*)(&npts), UINT64);
    err_code = read_scalar_from_json_file(test_filename, "fs", (void*)(&fs), DOUBLE);
    err_code = read_scalar_from_json_file(test_filename, "num_tones", (void*)(&num_tones), UINT64);
    
    freq = (double*)calloc(num_tones, sizeof(double));
    scale = (double*)calloc(num_tones, sizeof(double));
    phase = (double*)calloc(num_tones, sizeof(double));
    if (num_tones > 1) {
        err_code = read_array_from_json_file(test_filename, "freq", freq, DOUBLE, num_tones);
        err_code = read_array_from_json_file(test_filename, "scale", scale, DOUBLE, num_tones);
        err_code = read_array_from_json_file(test_filename, "phase", phase, DOUBLE, num_tones);
    }
    else {
        err_code = read_scalar_from_json_file(test_filename, "freq", (void*)(freq), DOUBLE);
        err_code = read_scalar_from_json_file(test_filename, "scale", (void*)(scale), DOUBLE);
        err_code = read_scalar_from_json_file(test_filename, "phase", (void*)(phase), DOUBLE);
    }

    // configuration
    gn_config_tone_struct c = NULL;
    err_code = gn_config_tone(&c, wf_type, npts, fs, num_tones, freq, scale, phase);

    // waveform generation
    gn_gen_tone(c, &awf);

    // read reference waveform
    ref_awf = (double*)malloc(npts*sizeof(double));
    err_code = read_array_from_json_file(test_filename, "test_vec", ref_awf, DOUBLE, npts);
    for (int i = 0; i < 10; i++)
        printf("%d\t%f\t%f\n",i,ref_awf[i],awf[i]);

    // compare
    assert(float_arrays_almost_equal(ref_awf, awf, npts, 6));    

    // free memory
    free(awf);
    free(freq);
    free(scale);
    free(phase);
    free(ref_awf);

    return 0;
}
