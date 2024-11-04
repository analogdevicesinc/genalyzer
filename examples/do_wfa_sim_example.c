#include "cgenalyzer_simplified_beta.h"
#include <stdlib.h>
#include <math.h>

int main(int argc, const char* argv[])
{
    // parameters
    tone_type ttype = REAL_COSINE;
    double fsr = 3.0, qnoise = pow(10.0, -60.0 / 20.0);
    int qres = 12;
    double fs = 5000000.0;
    unsigned long long npts = 8192, num_tones = 1;
    double freq[] = {50000.0}, scale[] = {0.5}, phase[] = {0.2};
    
    // waveforms
    double *awf;
    int32_t *qwf;

    // results
    size_t results_size;
    char **rkeys;
    double *rvalues;

    // configuration
    int err_code;
    gn_config c = NULL;
    err_code = gn_config_gen_tone(ttype, npts, fs, num_tones, freq, scale, phase, &c);
    err_code = gn_config_quantize(npts, fsr, qres, qnoise, &c);

    // generate waveform
    err_code = gn_gen_real_tone(&awf, &c);
    
    // quantize waveform
    err_code = gn_quantize(&qwf, awf, &c);
    
    // do waveform analysis
    err_code = gn_get_wfa_results(&rkeys, &rvalues, &results_size, qwf, &c);
    
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
