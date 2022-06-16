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
    double *ref_awf;
    int32_t *qwf, *ref_qwf;

    // read parameters
    double fsr, qnoise;
    int qres;
    unsigned int npts;
    err_code = read_scalar_from_json_file(test_filename, "npts", (void*)(&npts), UINT64);
    err_code = read_scalar_from_json_file(test_filename, "fsr", (void*)(&fsr), DOUBLE);
    err_code = read_scalar_from_json_file(test_filename, "qnoise", (void*)(&qnoise), DOUBLE);
    err_code = read_scalar_from_json_file(test_filename, "qres", (void*)(&qres), INT32);
    
    // configuration
    gn_config_quantize_struct c = NULL;
    err_code = gn_config_quantize(&c, npts, fsr, qres, qnoise);    

    // read reference waveforms
    ref_awf = (double*)malloc(npts*sizeof(double));
    err_code = read_array_from_json_file(test_filename, "test_vec", ref_awf, DOUBLE, npts);
    ref_qwf = (int32_t*)malloc(npts*sizeof(int32_t));
    err_code = read_array_from_json_file(test_filename, "test_vecq", ref_qwf, INT32, npts);
    
    // quantize waveform
    err_code = gn_quantize(&qwf, ref_awf, c);
    
    // read reference waveform
    ref_awf = (double*)malloc(npts*sizeof(double));
    err_code = read_array_from_json_file(test_filename, "test_vec", ref_awf, DOUBLE, npts);
    
    // compare
    assert(int_arrays_almost_equal(ref_qwf, qwf, 0, npts, INT32));
    for (int ii = 0; ii < 10; ii++)
        printf("%d\t%d\n", ref_qwf[ii], qwf[ii]);

    // free memory
    free(qwf);
    free(ref_qwf);
    free(ref_awf);
    
    return 0;
}
