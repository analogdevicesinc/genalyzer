#include "cgenalyzer.h"
#include "cgenalyzer_private.h"

extern "C" {
double compute_metric(config c, fft_analysis_wrapper* obj, const double* fft_data_re, const double* fft_data_im, const char* result_name)
{
    an::fft_analysis2* fftobj;
    fftobj = reinterpret_cast<an::fft_analysis2*>(obj);

    size_t fft_size;
    if (c->wf_type == COMPLEX_EXP)
        fft_size = c->nfft;
    else
        fft_size = c->nfft / 2;
    an::cplx_vector fft_cplx(fft_size);
    std::complex<double> I = std::complex<double>(0, 1);
    for (unsigned int i = 0; i < fft_size; ++i)
        fft_cplx[i] = fft_data_re[i] + I * fft_data_im[i];

    an::str_t fs_str = an::real_to_string(c->fs);
    fftobj->set_fsample(fs_str);
    an::var_map::unique_ptr results = fftobj->analyze_rfft(fft_cplx);
    double r = results->as_map("Metrics").as_real(result_name);
    return r;
}
}
