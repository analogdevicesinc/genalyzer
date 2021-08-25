#include "cgenalyzer.h"
#include "cgenalyzer_private.h"
#include <dft.hpp>

extern "C" {
void fft(config c, int32_t* qwf_i, int32_t* qwf_q, double** fft_cplx_re, double** fft_cplx_im, size_t* len)
{
    an::int32_vector qwvf_i(qwf_i, qwf_i + c->nfft);
    an::int32_vector qwvf_q(qwf_q, qwf_q + c->nfft);
    an::cplx_vector fft_cplx(c->nfft);
    double* fft_cplx_re_tmp = (double*)calloc(c->nfft, sizeof(double));
    double* fft_cplx_im_tmp = (double*)calloc(c->nfft, sizeof(double));

    an::fft(qwvf_i.data(), qwvf_i.size(), qwvf_q.data(), qwvf_q.size(), 1, fft_cplx.data(), fft_cplx.size(), c->res, c->navg, static_cast<an::diff_t>(c->nfft), c->win, c->cf);

    for (int i = 0; i < fft_cplx.size(); i++) {
        fft_cplx_re_tmp[i] = fft_cplx[i].real();
        fft_cplx_im_tmp[i] = fft_cplx[i].imag();
    }
    *fft_cplx_re = fft_cplx_re_tmp;
    *fft_cplx_im = fft_cplx_im_tmp;
    *len = fft_cplx.size();
}
}
