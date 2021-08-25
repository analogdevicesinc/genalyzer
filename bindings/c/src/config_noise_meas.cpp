#include "cgenalyzer.h"
#include "cgenalyzer_private.h"

extern "C" {
void config_noise_meas(config* c, waveform_type wf_type, size_t fft_order, int num_avgs, double sample_rate, double full_scale_range, int resolution, double noise_pwr_lvl, bool fsample_update, bool fdata_update, bool fshift_update)
{
    config c_private;
    c_private = (config)calloc(1, sizeof(*c_private));

    c_private->wf_type = wf_type;
    c_private->nfft = fft_order;
    c_private->navg = num_avgs;
    c_private->npts = num_avgs * fft_order;
    if (c_private->wf_type == COMPLEX_NOISE)
        c_private->npts <<= 1;
    c_private->fs = sample_rate;
    c_private->fsr = full_scale_range;
    c_private->res = resolution;
    c_private->noise_pwr_db = noise_pwr_lvl;
    c_private->update_fsample = fsample_update;
    c_private->update_fdata = fdata_update;
    c_private->update_fshift = fshift_update;
    c_private->fdata = c_private->fs;
    c_private->fshift = c_private->fs;

    *c = c_private;
}
}
