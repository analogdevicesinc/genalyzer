#include "test_genalyzer.h"
#include <assert.h>
#include <cgenalyzer.h>
#include <stdbool.h>
#include <stdlib.h>

int main(int argc, const char* argv[])
{
    // read test waveform
    const char* test_filename = argv[1];

    double* awf;
    unsigned int err_code;
    size_t npts;
    int res;
    double fs, fsr, start, stop;
    err_code = read_param(test_filename, "npts", (void*)(&npts), UINT64);
    err_code = read_param(test_filename, "res", (void*)(&res), INT32);
    err_code = read_param(test_filename, "fs", (void*)(&fs), DOUBLE);
    err_code = read_param(test_filename, "fsr", (void*)(&fsr), DOUBLE);
    err_code = read_param(test_filename, "start", (void*)(&start), DOUBLE);
    err_code = read_param(test_filename, "stop", (void*)(&stop), DOUBLE);

    double * ref_awf = (double*)malloc(npts*sizeof(double));
    gn_config c = NULL;

    // configuration
    gn_config_ramp_nl_meas(&c,
        npts, // # of data points
        fs, // sample rate
        fsr, // full-scale range
        res, // ADC resolution: unused configuration setting
        start,
        stop,
        0.0);

    // waveform generation
    size_t len;
    gn_gen_ramp(c, &awf, &len);

    // read reference waveform
    read_file_to_array(test_filename, (void*)ref_awf, DOUBLE);

    // compare
    assert(float_arrays_almost_equal(ref_awf, awf, len, 6));

    // free memory
    free(c);
    free(awf);
    free(ref_awf);

    return 0;
}
