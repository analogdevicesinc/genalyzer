#include "cgenalyzer.h"
#include "cgenalyzer_private.h"
#include <complex>
#include <dft.hpp>
#include <iostream>
#include <processes.hpp>

extern "C" {
void fft_mp(gn_config c, size_t* fft_m_size, size_t* fft_p_size, double* fft_m, double* fft_p, int32_t* qwf_i, size_t* qwf_i_size, int32_t* qwf_q, size_t* qwf_q_size)
{
    an::int32_vector qwvf_i(qwf_i, qwf_i + *qwf_i_size);
    an::int32_vector qwvf_q(qwf_q, qwf_q + *qwf_q_size);

    *fft_m_size = c->nfft / 2 + 1;
    *fft_p_size = c->nfft / 2 + 1;
    an::fft_mp(qwvf_i.data(), qwvf_i.size(), qwvf_q.data(), qwvf_q.size(), static_cast<an::diff_t>(c->in_stride), fft_m, *fft_m_size, fft_p, *fft_p_size, c->res, c->navg, static_cast<an::diff_t>(c->nfft), c->win, c->cf);
}
}
