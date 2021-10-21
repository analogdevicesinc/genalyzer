#include "cgenalyzer.h"
#include "cgenalyzer_private.h"
#include <complex>
#include <dft.hpp>
#include <processes.hpp>

extern "C" {
void gn_rfft(gn_config c, int32_t* qwf, double** fft_cplx_re, double** fft_cplx_im, size_t* len)
{
    an::int32_vector qwvf(qwf, qwf + c->npts);
    an::cplx_vector fft_cplx(c->nfft / 2 + 1);
    double* fft_cplx_re_tmp = (double*)calloc(c->nfft / 2, sizeof(double));
    double* fft_cplx_im_tmp = (double*)calloc(c->nfft / 2, sizeof(double));

    an::rfft(qwvf.data(), qwvf.size(), fft_cplx.data(), fft_cplx.size(), c->res, c->navg, static_cast<an::diff_t>(c->nfft), c->win, c->cf);

    for (int i = 0; i < fft_cplx.size(); i++) {
        fft_cplx_re_tmp[i] = fft_cplx[i].real();
        fft_cplx_im_tmp[i] = fft_cplx[i].imag();
    }
    *fft_cplx_re = fft_cplx_re_tmp;
    *fft_cplx_im = fft_cplx_im_tmp;
    *len = fft_cplx.size();
}
}
