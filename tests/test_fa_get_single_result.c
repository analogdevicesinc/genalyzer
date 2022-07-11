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
    int32_t *ref_qwfi, *ref_qwfq;
    double *fft_out;
    double sfdr;

    // read parameters
    tone_type ttype;
    int qres;
    unsigned int npts, fft_navg, nfft, tmp_win, num_tones;
    double *freq;
    GnWindow win;    
    err_code = read_scalar_from_json_file(test_filename, "wf_type", (void*)(&ttype), UINT64);
    err_code = read_scalar_from_json_file(test_filename, "qres", (void*)(&qres), INT32);
    err_code = read_scalar_from_json_file(test_filename, "npts", (void*)(&npts), UINT64);    
    err_code = read_scalar_from_json_file(test_filename, "navg", (void*)(&fft_navg), UINT64);
    err_code = read_scalar_from_json_file(test_filename, "nfft", (void*)(&nfft), UINT64);
    err_code = read_scalar_from_json_file(test_filename, "num_tones", (void*)(&num_tones), UINT64);
    freq = (double*)calloc(num_tones, sizeof(double));
    if (num_tones > 1)
        err_code = read_array_from_json_file(test_filename, "freq", freq, DOUBLE, num_tones);
    else
        err_code = read_scalar_from_json_file(test_filename, "freq", (void*)(freq), DOUBLE);
    err_code = read_scalar_from_json_file(test_filename, "win", (void*)(&tmp_win), UINT64);
    if (tmp_win==1)
        win = GnWindowBlackmanHarris;
    else if (tmp_win==2)
        win = GnWindowHann; 
    else if (tmp_win==3)
        win = GnWindowNoWindow;

    // read reference waveforms    
    ref_qwfi = (int32_t*)malloc(npts*sizeof(int32_t));
    err_code = read_array_from_json_file(test_filename, "test_vecq_i", ref_qwfi, INT32, npts);
    ref_qwfq = (int32_t*)malloc(npts*sizeof(int32_t));
    err_code = read_array_from_json_file(test_filename, "test_vecq_q", ref_qwfq, INT32, npts);

    // configuration
    gn_config c = NULL;
    err_code = gn_config_calloc(&c);
    err_code = gn_config_fftz(npts, qres, fft_navg, nfft, win, c);

    // FFT of waveform
    err_code = gn_fftz(&fft_out, ref_qwfi, ref_qwfq, c);

    // Configure Fourier analysis
    err_code = gn_config_fa(freq[0], c);
    err_code = gn_get_fa_single_result(&sfdr, "sfdr", fft_out, c);
    
    printf("SFDR - %20.6f\n", sfdr);
    
    // free memory
    free(ref_qwfi);
    free(ref_qwfq);
    free(fft_out);
    gn_config_free(c);
    
    return 0;
}
