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
    double *fft_out, *ref_fft_out;

    // read parameters
    tone_type ttype;
    int qres;
    unsigned long long npts, fft_navg, nfft, tmp_win;
    GnWindow win;
    err_code = read_scalar_from_json_file(test_filename, "wf_type", (void*)(&ttype), UINT64);
    err_code = read_scalar_from_json_file(test_filename, "qres", (void*)(&qres), INT32);
    err_code = read_scalar_from_json_file(test_filename, "npts", (void*)(&npts), UINT64);    
    err_code = read_scalar_from_json_file(test_filename, "navg", (void*)(&fft_navg), UINT64);
    err_code = read_scalar_from_json_file(test_filename, "nfft", (void*)(&nfft), UINT64);
    err_code = read_scalar_from_json_file(test_filename, "win", (void*)(&tmp_win), UINT64);
    if (tmp_win==1)
        win = GnWindowBlackmanHarris;
    else if (tmp_win==2)
        win = GnWindowHann; 
    else if (tmp_win==3)
        win = GnWindowNoWindow;

    // configuration
    gn_config c = NULL;
    err_code = gn_config_fftz(npts, qres, fft_navg, nfft, win, &c);

    // read reference waveforms    
    ref_qwfi = (int32_t*)malloc(npts*sizeof(int32_t));
    err_code = read_array_from_json_file(test_filename, "test_vecq_i", ref_qwfi, INT32, npts);
    ref_qwfq = (int32_t*)malloc(npts*sizeof(int32_t));
    err_code = read_array_from_json_file(test_filename, "test_vecq_q", ref_qwfq, INT32, npts);

    // FFT of waveform
    err_code = gn_fftz(&fft_out, ref_qwfi, ref_qwfq, &c);

    // read reference FFT
    ref_fft_out = (double*)malloc(2*nfft*sizeof(double));
    err_code = read_array_from_json_file(test_filename, "fft_test_vec", ref_fft_out, DOUBLE, 2*nfft);

    // compare
    assert(float_arrays_almost_equal(ref_fft_out, fft_out, 2*nfft, 6));    
    
    // free memory
    free(ref_qwfi);
    free(ref_qwfq);
    free(fft_out);
    free(ref_fft_out);
    gn_config_free(&c);
    
    return 0;
}
