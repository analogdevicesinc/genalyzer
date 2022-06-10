#include "cgenalyzer.h"
#include "test_genalyzer.h"
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

int main(int argc, const char* argv[])
{
    // read test waveform filename
    const char* test_filename = argv[1];
    
    char *tmp_token_name;
    //double* awf;
    unsigned int err_code;
    meas_domain domain_wf;
    waveform_type type_wf;
    size_t nfft, num_tones;
    int navg;
    double fs, fsr;
    double freq, scale, phase;// double *freq, *scale, *phase;
    err_code = read_param(test_filename, "wf_type", (void*)(&domain_wf), UINT64);
    /*
    err_code = read_param(test_filename, "type_wf", (void*)(&type_wf), UINT64);
    err_code = read_param(test_filename, "nfft", (void*)(&nfft), UINT64);
    err_code = read_param(test_filename, "num_tones", (void*)(&num_tones), UINT64);
    err_code = read_param(test_filename, "navg", (void*)(&navg), INT32);
    err_code = read_param(test_filename, "fs", (void*)(&fs), DOUBLE);
    err_code = read_param(test_filename, "fsr", (void*)(&fsr), DOUBLE);*/
    /*
    freq = (double*)calloc(num_tones, sizeof(double));
    scale = (double*)calloc(num_tones, sizeof(double));
    phase = (double*)calloc(num_tones, sizeof(double));
    tmp_token_name = (char*)malloc(10*sizeof(char));
    for (int n = 0; n < num_tones; n++) {
        sprintf(tmp_token_name, "freq%d", n);
        err_code = read_param(test_filename, tmp_token_name, (void*)(freq+n), DOUBLE);
        sprintf(tmp_token_name, "scale%d", n);
        err_code = read_param(test_filename, tmp_token_name, (void*)(scale+n), DOUBLE);
        sprintf(tmp_token_name, "phase%d", n);
        err_code = read_param(test_filename, tmp_token_name, (void*)(phase+n), DOUBLE);
    }
    *//*
   err_code = read_param(test_filename, "freq0", (void*)(&freq), DOUBLE);
   err_code = read_param(test_filename, "scale0", (void*)(&scale), DOUBLE);
   err_code = read_param(test_filename, "phase0", (void*)(&phase), DOUBLE);

    // configuration
    gn_config c = NULL;
    gn_config_tone_gen(&c,
        domain_wf,
        type_wf,
        nfft, // FFT order
        navg, // # of FFTs averaged
        fs, // sample rate
        fsr, // full-scale range
        0, // ADC resolution: unused configuration setting
        freq, // tone frequency, # of array elements = num_tones
        scale, // tone scale, # of array elements = num_tones
        phase, // tone phase, # of array elements = num_tones
        num_tones, // # of tones
        0, 
        false,
        false,
        false);

    // waveform generation
    size_t npts;
    gn_gen_tone(c, &awf, &npts);
    
    // read reference waveform
    double * ref_awf = (double*)malloc(npts*sizeof(double));
    read_file_to_array(test_filename, (void*)ref_awf, DOUBLE);
    for (int i=0; i<10; i++)
        printf("%d\t%f\t%f\n",i,ref_awf[i], awf[i]);
printf("hello\n");
    // compare
    assert(float_arrays_almost_equal(ref_awf, awf, npts, 6));

    // free memory*/
    //free(awf);
    /*
    free(freq);
    free(scale);
    free(phase);
    */
    //free(ref_awf);
    //free(tmp_token_name);

    return 0;
}
