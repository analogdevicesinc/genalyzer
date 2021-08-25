#include "test_genalyzer.h"
#include <assert.h>
#include <cgenalyzer.h>
#include <stdbool.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
    // read test waveform
    const char* test_filename = argv[1];

    double* awf;
    int* qwf;
    unsigned int err_code;
    size_t npts = atoll(extract_token(test_filename, "npts", &err_code));
    double fs = atof(extract_token(test_filename, "fs", &err_code));
    double fsr = atof(extract_token(test_filename, "fsr", &err_code));
    int res = atoi(extract_token(test_filename, "res", &err_code));
    double start = atof(extract_token(test_filename, "start", &err_code));
    double stop = atof(extract_token(test_filename, "stop", &err_code));

    int ref_qwf[npts];
    config c = NULL;

    // configuration
    config_ramp_nl_meas(&c,
        npts, // # of data points
        fs, // sample rate
        fsr, // full-scale range
        res, // ADC resolution: unused configuration setting
        start,
        stop,
        -200.0);

    // waveform generation
    size_t len;
    gen_ramp(c, &awf, &len);
    quantize(c, awf, &qwf);

    // read reference waveform
    read_file_to_array(test_filename, (void*)ref_qwf, INT32);

    // compare
    assert(int_arrays_almost_equal(qwf, ref_qwf, 1, len, INT32));

    // free memory
    free(c);
    free(awf);
    free(qwf);

    return 0;
}
