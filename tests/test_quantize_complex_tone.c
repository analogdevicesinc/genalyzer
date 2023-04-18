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
    double *ref_awfi, *ref_awfq;
    int32_t *qwfi, *qwfq, *ref_qwfi, *ref_qwfq;

    // read parameters
    double fsr, qnoise;
    int qres;
    unsigned long long npts;
    err_code = read_scalar_from_json_file(test_filename, "npts", (void*)(&npts), UINT64);
    if (err_code != 0)return err_code;
    err_code = read_scalar_from_json_file(test_filename, "fsr", (void*)(&fsr), DOUBLE);
    if (err_code != 0)return err_code;
    err_code = read_scalar_from_json_file(test_filename, "qnoise", (void*)(&qnoise), DOUBLE);
    if (err_code != 0)return err_code;
    err_code = read_scalar_from_json_file(test_filename, "qres", (void*)(&qres), INT32);
    if (err_code != 0)return err_code;
    
    // configuration
    gn_config c = NULL;
    err_code = gn_config_quantize(npts, fsr, qres, qnoise, &c);
    if (err_code != 0)return err_code;

    // read reference waveforms
    ref_awfi = (double*)malloc(npts*sizeof(double));
    err_code = read_array_from_json_file(test_filename, "test_vec_i", ref_awfi, DOUBLE, npts);
    if (err_code != 0)return err_code;
    ref_awfq = (double*)malloc(npts*sizeof(double));
    err_code = read_array_from_json_file(test_filename, "test_vec_q", ref_awfq, DOUBLE, npts);
    if (err_code != 0)return err_code;
    ref_qwfi = (int32_t*)malloc(npts*sizeof(int32_t));
    err_code = read_array_from_json_file(test_filename, "test_vecq_i", ref_qwfi, INT32, npts);
    if (err_code != 0)return err_code;
    ref_qwfq = (int32_t*)malloc(npts*sizeof(int32_t));
    err_code = read_array_from_json_file(test_filename, "test_vecq_q", ref_qwfq, INT32, npts);
    if (err_code != 0)return err_code;
    
    // quantize waveform
    err_code = gn_quantize(&qwfi, ref_awfi, &c);
    if (err_code != 0)return err_code;
    err_code = gn_quantize(&qwfq, ref_awfq, &c);
    if (err_code != 0)return err_code;

    // read reference waveform
    ref_awfi = (double*)malloc(npts*sizeof(double));
    err_code = read_array_from_json_file(test_filename, "test_vec_i", ref_awfi, DOUBLE, npts);
    if (err_code != 0)return err_code;
    ref_awfq = (double*)malloc(npts*sizeof(double));
    err_code = read_array_from_json_file(test_filename, "test_vec_q", ref_awfq, DOUBLE, npts);
    if (err_code != 0)return err_code;

    // compare
    assert(int_arrays_almost_equal(ref_qwfi, qwfi, 0, npts, INT32));
    assert(int_arrays_almost_equal(ref_qwfq, qwfq, 0, npts, INT32));

    // free memory
    free(qwfi);
    free(qwfq);
    free(ref_qwfi);
    free(ref_qwfq);
    free(ref_awfi);
    free(ref_awfq);
    gn_config_free(&c);

    return 0;
}
