#include "test_genalyzer.h"
#include <assert.h>
#include <cgenalyzer.h>
#include <stdbool.h>
#include <stdlib.h>

int main(int argc, const char* argv[])
{
    // read test waveform
    const char* test_filename_ip = argv[1];
    const char* test_filename_hits_op0 = argv[2];
    const char* test_filename_hits_op1 = argv[3];

    char *tmp_token_name;
    double* awf;
    unsigned int err_code;
    size_t npts;
    int res;
    double fs, fsr, start, stop;
    err_code = read_param(test_filename_ip, "npts", (void*)(&npts), UINT64);
    err_code = read_param(test_filename_ip, "res", (void*)(&res), INT32);
    err_code = read_param(test_filename_ip, "fs", (void*)(&fs), DOUBLE);
    err_code = read_param(test_filename_ip, "fsr", (void*)(&fsr), DOUBLE);
    err_code = read_param(test_filename_ip, "start", (void*)(&start), DOUBLE);
    err_code = read_param(test_filename_ip, "stop", (void*)(&stop), DOUBLE);

    int *ip_qwf = (int*)calloc(npts, sizeof(int));
    size_t num_bins = (1 << res);
    int* bins;
    double* dnl;
    long int* hits;
    long int *ref_hits = (long int*)calloc(num_bins, sizeof(long int));
    double *ref_dnl = (double*)calloc(num_bins, sizeof(double));
    gn_config c = NULL;

    // configuration
    gn_config_ramp_nl_meas(&c,
        npts, // # of data points
        fs, // sample rate
        fsr, // full-scale range
        res, // ADC resolution: unused configuration setting
        start,
        stop,
        -200.0);

    // read quantized ramp input waveform
    read_file_to_array(test_filename_ip, (void*)ip_qwf, INT32);

    // read histogram frequency output waveform
    read_file_to_array(test_filename_hits_op0, (void*)ref_hits, INT64);

    // read reference dnl data output waveform
    read_file_to_array(test_filename_hits_op1, (void*)ref_dnl, DOUBLE);

    // compute dnl
    gn_compute_dnl(c, ip_qwf, &bins, &hits, &dnl);
    // compute_inl(c, qwf, &bins, &hits, &dnl_data, &inl_data);

    // reference bins array
    int *ref_bins = (int*)calloc(num_bins, sizeof(int));
    for (int i = 0; i < num_bins; i++)
        ref_bins[i] = i - num_bins / 2;

    // compare
    assert(int_arrays_almost_equal(bins, ref_bins, num_bins, 0, INT32));
    assert(int_arrays_almost_equal(hits, ref_hits, num_bins, 1, INT64));
    float_arrays_almost_equal(dnl, ref_dnl, num_bins, 5);

    // free memory
    free(c);
    free(hits);
    free(bins);
    free(dnl);
    free(ip_qwf);
    free(ref_hits);
    free(ref_dnl);
    free(ref_bins);
    free(tmp_token_name);

    return 0;
}
