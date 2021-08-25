#include "cgenalyzer.h"
#include "cgenalyzer_private.h"
#include <complex>
#include <dft.hpp>
#include <iostream>
#include <processes.hpp>

extern "C" {
void rfft_norm(config c, size_t* fft_size, double* fft_cplx_re, double* fft_cplx_im, double* awf, size_t* awf_size)
{
    an::cplx_vector fft_cplx(c->nfft / 2 + 1);
    an::rfft_norm(awf, *awf_size, fft_cplx.data(), fft_cplx.size(), c->navg, static_cast<an::diff_t>(c->nfft), c->win);

    for (int i = 0; i < fft_cplx.size(); i++) {
        fft_cplx_re[i] = fft_cplx[i].real();
        fft_cplx_im[i] = fft_cplx[i].imag();
    }
    *fft_size = c->nfft / 2 + 1;
}
}
