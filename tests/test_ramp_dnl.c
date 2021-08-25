#include "test_genalyzer.h"
#include <assert.h>
#include <cgenalyzer.h>
#include <stdbool.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
    // read test waveform
    const char* test_filename_ip = argv[1];
    const char* test_filename_hits_op0 = argv[2];
    const char* test_filename_hits_op1 = argv[3];

    size_t npts = atoll(extract_token(test_filename_ip, "npts"));
    double fs = atof(extract_token(test_filename_ip, "fs"));
    double fsr = atof(extract_token(test_filename_ip, "fsr"));
    int res = atoi(extract_token(test_filename_ip, "res"));
    double start = atof(extract_token(test_filename_ip, "start"));
    double stop = atof(extract_token(test_filename_ip, "stop"));

    int ip_qwf[npts];
    size_t num_bins = (1 << res);
    size_t num_hits = num_bins;
    int* bins;
    double* dnl;
    long int* hits;
    long int ref_hits[num_bins];
    double ref_dnl[num_bins];
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

    // read quantized ramp input waveform
    read_file_to_array(test_filename_ip, (void*)ip_qwf, INT32);

    // read histogram frequency output waveform
    read_file_to_array(test_filename_hits_op0, (void*)ref_hits, INT64);

    // read reference dnl data output waveform
    read_file_to_array(test_filename_hits_op1, (void*)ref_dnl, DOUBLE);

    // compute dnl
    compute_dnl(c, ip_qwf, &bins, &hits, &dnl);
    // compute_inl(c, qwf, &bins, &hits, &dnl_data, &inl_data);

    // reference bins array
    int ref_bins[num_bins];
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

    return 0;
}
