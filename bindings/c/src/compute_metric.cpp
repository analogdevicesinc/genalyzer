#include "cgenalyzer.h"
#include "cgenalyzer_private.h"

extern "C" {
double gn_compute_metric(gn_config c, fft_analysis_wrapper* obj, const double* fft_data, const char* result_name)
{
    an::fourier_analysis* fftobj;
    fftobj = reinterpret_cast<an::fourier_analysis*>(obj);

    size_t fft_size;
    if (c->wf_type == COMPLEX_EXP)
        fft_size = c->nfft;
    else
        fft_size = c->nfft / 2;
    /*
    an::cplx_vector fft_cplx(fft_size);
    std::complex<double> I = std::complex<double>(0, 1);
    for (unsigned int i = 0; i < fft_size; ++i)
        fft_cplx[i] = fft_data_re[i] + I * fft_data_im[i];
    */

    an::str_t fs_str = an::to_string(c->fs);
    fftobj->set_fsample(fs_str);

    an::fourier_analysis_results results;
    if ((c->wf_type == REAL_COSINE) || (c->wf_type == REAL_SINE))
        results = fftobj->analyze(fft_data, fft_size, c->nfft, FreqAxisType::DcCenter);
    else if (c->wf_type == COMPLEX_EXP)
        results = fftobj->analyze(fft_data, fft_size, c->nfft, FreqAxisType::DcCenter);

    double r = results->as_map("Metrics").as_real(result_name);
    return r;
}
}
