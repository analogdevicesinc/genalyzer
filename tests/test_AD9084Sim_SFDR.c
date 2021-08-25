#include "../tests/test_genalyzer.h"
#include "../wrapper_include/genalyzer_cwrapper.h"
#include <stdbool.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
    // read test waveform
    const char* test_filename_ip = argv[1];
    printf("%s\n", test_filename_ip);

    meas_domain domain_wf = atoll(extract_token(test_filename_ip, "domain_wf"));
    waveform_type type_wf = atoll(extract_token(test_filename_ip, "type_wf"));
    size_t nfft = atoll(extract_token(test_filename_ip, "nfft"));
    size_t num_tones = atoll(extract_token(test_filename_ip, "num_tones"));
    int res = atoi(extract_token(test_filename_ip, "res"));
    int navg = atoi(extract_token(test_filename_ip, "navg"));
    double fs = atof(extract_token(test_filename_ip, "fs"));
    printf("%f\n", fs);
    double fsr = atof(extract_token(test_filename_ip, "fsr"));
    double* freq = (double*)calloc(num_tones, sizeof(double));
    double* scale = (double*)calloc(num_tones, sizeof(double));
    double* phase = (double*)calloc(num_tones, sizeof(double));
    printf("%u\t%d\t%d\t%lf\n", nfft, res, navg, fs);

    char tmp_token[10];
    for (int n = 0; n < num_tones; n++) {
        sprintf(tmp_token, "freq%d", n);
        freq[n] = atof(extract_token(test_filename_ip, tmp_token));
        sprintf(tmp_token, "scale%d", n);
        scale[n] = atof(extract_token(test_filename_ip, tmp_token));
        sprintf(tmp_token, "phase%d", n);
        phase[n] = atof(extract_token(test_filename_ip, tmp_token));
    }

    size_t npts;
    if (type_wf == 2)
        npts = 2 * nfft * navg;
    else
        npts = nfft * navg;

    int ref_qwf_ip[npts];
    double rfft_op_re[nfft / 2 + 1], rfft_op_im[nfft / 2 + 1];
    config c = NULL;

    // configuration
    config_tone_meas(&c,
        domain_wf,
        type_wf,
        nfft, // FFT order
        navg, // # of FFTs averaged
        fs, // sample rate
        fsr, // full-scale range
        res, // ADC resolution: unused configuration setting
        freq, // tone frequency, # of array elements = num_tones
        scale, // tone scale, # of array elements = num_tones
        phase, // tone phase, # of array elements = num_tones
        num_tones, // # of tones
        false,
        false,
        false);

    // read quantized input waveform
    read_file_to_array(test_filename_ip, (void*)ref_qwf_ip, INT32);

    // compute rfft
    // rfft(c, rfft_op_re, rfft_op_im, ref_qwf_ip);
    printf("1. gadhg\n");
    // load FFT data
    double* fft_data = calloc(nfft, sizeof(double));
    /*
  for (int i = 0; i < nfft/2+1; i++) {
    fft_data[2*i] = rfft_op_re[i];
    fft_data[2*i+1] = rfft_op_im[i];
    }*/
    printf("2. skdgs\n");
    // compute metrics and print results
    double sfdr_val = metric(c, fft_data, "SFDR");
    printf("SFDR - %f\n", sfdr_val);

    // free memory
    free(fft_data);

    return 0;
}
