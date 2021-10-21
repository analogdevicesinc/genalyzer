#include "cgenalyzer.h"
#include "cgenalyzer_private.h"
#include <complex>
#include <dft.hpp>
#include <iostream>
#include <processes.hpp>

extern "C" {
void gn_rfft_mp(gn_config c, size_t* fft_m_size, size_t* fft_p_size, double* fft_m, double* fft_p, int32_t* qwf, size_t* qwf_size)
{
    an::int32_vector qwvf(qwf, qwf + *qwf_size);

    *fft_m_size = c->nfft / 2 + 1;
    *fft_p_size = c->nfft / 2 + 1;
    an::rfft_mp(qwvf.data(), qwvf.size(), fft_m, *fft_m_size, fft_p, *fft_p_size, c->res, c->navg, static_cast<an::diff_t>(c->nfft), c->win, c->cf);
}
}
