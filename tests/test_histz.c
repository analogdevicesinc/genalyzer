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
    int32_t *ref_qwf;
    uint64_t *hist;
    size_t hist_len;
    
    // read parameters
    double fsr, qnoise;
    int qres;
    unsigned int npts;
    err_code = read_scalar_from_json_file(test_filename, "npts", (void*)(&npts), UINT64);
    err_code = read_scalar_from_json_file(test_filename, "fsr", (void*)(&fsr), DOUBLE);
    err_code = read_scalar_from_json_file(test_filename, "qnoise", (void*)(&qnoise), DOUBLE);
    err_code = read_scalar_from_json_file(test_filename, "qres", (void*)(&qres), INT32);
    
    // configuration
    gn_config c = NULL;
    err_code = gn_config_quantize(npts, fsr, qres, qnoise, &c);
    
    // read reference output waveform
    ref_qwf = (int32_t*)malloc(npts*sizeof(int32_t));
    err_code = read_array_from_json_file(test_filename, "test_vecq", ref_qwf, INT32, npts);
    
    // Do waveform analysis
    err_code = gn_histz(&hist, &hist_len, ref_qwf, &c);
    
    printf("Histogram length: %lu\n", hist_len);
    size_t tmp_sum = 0;
    for (size_t i = 0; i < hist_len; i++)
        tmp_sum += hist[i];
    printf("tmp_sum: %lu\n", tmp_sum);
    for (size_t i = 0; i < 20; i++)
        printf("%4zu\t%4zu\n", i, hist[i]);

    // free memory
    free(ref_qwf);
    free(hist);
    gn_config_free(&c);
    
    return 0;
}
