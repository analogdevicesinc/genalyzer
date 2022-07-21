#include "cgenalyzer.h"
#include <stdlib.h>
#include <math.h>

int main(int argc, const char* argv[])
{
    // parameters
    double fsr = 3.0, qnoise = pow(10.0, -60.0 / 20.0), ramp_start = 0, ramp_stop = 2;
    int qres = 12;
    unsigned long long npts = 8192;
    
    // waveforms
    double *awf;
    int32_t *qwf;

    // results
    uint64_t *hist;
    size_t results_size, hist_len;
    char **rkeys;
    double *rvalues;

    // configuration
    int err_code;
    gn_config c = NULL;
    err_code = gn_config_gen_ramp(npts, ramp_start, ramp_stop, &c);
    err_code = gn_config_quantize(npts, fsr, qres, qnoise, &c);

    // generate waveform
    err_code = gn_gen_ramp(&awf, &c);
    
    // quantize waveform
    err_code = gn_quantize(&qwf, awf, &c);
    
    // compute histogram
    err_code = gn_histz(&hist, &hist_len, qwf, &c);
    
    // do waveform analysis
    err_code = gn_get_ha_results(&rkeys, &rvalues, &results_size, hist, &c);
    
    // print results
    printf("All Waveform Analysis Results:\n");
    for (size_t i = 0; i < results_size; i++)
        printf("%4zu%20s%20.6f\n", i, rkeys[i], rvalues[i]);

    // free memory
    free(qwf);
    free(awf);
    for (size_t i = 0; i < results_size; i++)
        free(rkeys[i]);
    free(rkeys);
    gn_config_free(&c);

    return 0;
}