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
    size_t results_size;
    char **rkeys;
    double *rvalues;

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

    // read reference input waveform
    ref_awf = (double*)malloc(npts*sizeof(double));
    err_code = read_array_from_json_file(test_filename, "test_vec", ref_awf, DOUBLE, npts);
    if (err_code != 0)return err_code;    
    
    // quantize waveform
    err_code = gn_quantize(&qwf, ref_awf, &c);
    if (err_code != 0)return err_code;
    
    // read reference output waveform
    ref_qwf = (int32_t*)malloc(npts*sizeof(int32_t));
    err_code = read_array_from_json_file(test_filename, "test_vecq", ref_qwf, INT32, npts);
    if (err_code != 0)return err_code;
    
    // Do waveform analysis
    err_code = gn_get_wfa_results(&rkeys, &rvalues, &results_size, ref_qwf, &c);
    if (err_code != 0)return err_code;
    
    printf("\nAll Waveform Analysis Results:\n");
    for (size_t i = 0; i < results_size; i++)
        printf("%4zu%20s%20.6f\n", i, rkeys[i], rvalues[i]);

    // free memory
    free(qwf);
    free(ref_qwf);
    free(ref_awf);
    gn_config_free(&c);
    
    return 0;
}
