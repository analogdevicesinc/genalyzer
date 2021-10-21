#include "cgenalyzer.h"
#include "cgenalyzer_private.h"
#include <dft.hpp>
#include <iostream>
#include <waveforms.hpp>

extern "C" {
void gn_gen_tone(gn_config c, double** result, size_t* len)
{
    std::vector<an::real_t> c_freq(c->freq, c->freq + c->n_tones);
    c_freq = an::coherent_freq(c->nfft, c->fs, c_freq);
    std::vector<an::real_t> c_ampl(c->scale, c->scale + c->n_tones);
    std::vector<an::real_t> c_phase(c->phase, c->phase + c->n_tones);
    *result = new double[c->npts];

    if (c->wf_type == REAL_COSINE) {
        an::real_vector awvf(c->npts);
        an::cos(awvf.data(), awvf.size(), c->fs, c->fsr, c_ampl, c_freq, c_phase, 0.0, 0.0, 0.0);

        memcpy(*result, &awvf.front(), awvf.size() * sizeof(double));
    } else if (c->wf_type == REAL_SINE) {
        an::real_vector awvf(c->npts);
        an::sin(awvf.data(), awvf.size(), c->fs, c->fsr, c_ampl, c_freq, c_phase, 0.0, 0.0, 0.0);

        memcpy(*result, &awvf.front(), awvf.size() * sizeof(double));
    } else if (c->wf_type == COMPLEX_EXP) {
        size_t awvf_size_iq = c->npts >> 1;
        an::real_vector awvf_i(awvf_size_iq);
        an::real_vector awvf_q(awvf_size_iq);
        an::cos(awvf_i.data(), awvf_size_iq, c->fs, c->fsr, c_ampl, c_freq, c_phase, 0.0, 0.0, 0.0);
        an::sin(awvf_q.data(), awvf_size_iq, c->fs, c->fsr, c_ampl, c_freq, c_phase, 0.0, 0.0, 0.0);

        auto i = awvf_i.begin();
        auto j = awvf_q.begin();
        int k = 0;
        while (i != awvf_i.end() && j != awvf_q.end()) {
            (*result)[k] = *i;
            (*result)[k + 1] = *j;
            i++;
            j++;
            k += 2;
        }
    }
    *len = c->npts;
}
}
