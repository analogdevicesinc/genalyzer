#include "cgenalyzer.h"
#include "cgenalyzer_private.h"
#include <cmath>

extern "C" {
void gn_config_tone_nl_meas(gn_config* c, waveform_type wf_type, size_t npts, double sample_rate, double full_scale_range, int resolution, double* tone_freq, double* tone_ampl, double* tone_phase, size_t num_tones)
{
    gn_config c_private;
    c_private = (gn_config)calloc(1, sizeof(*c_private));

    c_private->wf_type = wf_type;
    c_private->i_repr = I64;
    c_private->npts = npts;
    c_private->fs = sample_rate;
    c_private->fsr = full_scale_range;
    c_private->res = resolution;
    c_private->min_code = -(1 << (c_private->res - 1));
    c_private->max_code = (1 << (c_private->res - 1)) - 1;
    c_private->freq = tone_freq;
    c_private->scale = tone_ampl;
    c_private->phase = tone_phase;
    c_private->n_tones = num_tones;
    c_private->dnl_type = an::DnlSignal::Tone;

    *c = c_private;
}
}
