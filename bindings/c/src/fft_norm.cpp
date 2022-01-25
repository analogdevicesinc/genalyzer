#include "cgenalyzer.h"
#include "cgenalyzer_private.h"
#include <complex>
#include <dft.hpp>
#include <iostream>
#include <processes.hpp>

extern "C" {
void fft_norm(gn_config c, size_t* fft_size, double* fft_cplx_re, double* fft_cplx_im, double* awf_i, size_t* awf_i_size, double* awf_q)
{
    cplx_t * in = new cplx_t[*awf_i_size];
    for (size_t i = 0; i < *awf_i_size; i++)
        in[i] = std::complex<real_t>(awf_i[i], awf_q[i]);

    an::cplx_vector fft_cplx(c->nfft / 2 + 1);
    an::fft_norm(in, *awf_i_size, fft_cplx.data(), fft_cplx.size(), c->navg, static_cast<an::diff_t>(c->nfft), c->win);

    for (size_t i = 0; i < fft_cplx.size(); i++) {
        fft_cplx_re[i] = fft_cplx[i].real();
        fft_cplx_im[i] = fft_cplx[i].imag();
    }
    *fft_size = c->nfft / 2 + 1;
    delete in;
}
}
