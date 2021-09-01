#include "cgenalyzer.h"
#include "cgenalyzer_private.h"

extern "C" {
void config_tone_gen(config_tone* c, meas_domain m_domain, waveform_type wf_type, size_t fft_order, int num_avgs, double sample_rate, double full_scale_range, int resolution, double* tone_freq, double* tone_ampl, double* tone_phase, size_t num_tones, bool fsample_update, bool fdata_update, bool fshift_update)
{
    config c_p;
    c_p = (config)calloc(1, sizeof(*c_p));

    c_p->md = m_domain;
    c_p->wf_type = wf_type;
    c_p->i_repr = I32;
    c_p->nfft = fft_order;
    c_p->navg = num_avgs;
    c_p->npts = num_avgs * fft_order;
    if (c_p->wf_type == COMPLEX_EXP)
        c_p->npts <<= 1;
    c_p->fs = sample_rate;
    c_p->fsr = full_scale_range;
    c_p->res = resolution;
    c_p->freq = tone_freq;
    c_p->scale = tone_ampl;
    c_p->phase = tone_phase;
    c_p->n_tones = num_tones;
    c_p->update_fsample = fsample_update;
    c_p->update_fdata = fdata_update;
    c_p->update_fshift = fshift_update;
    c_p->fdata = c_p->fs;
    c_p->fshift = c_p->fs;

    // defaults
    c_p->irnoise = -200.0;
    c_p->cf = an::CodeFormat::TwosComplement;
    c_p->win = an::WindowType::Rect;

    *c = c_p;
}
}