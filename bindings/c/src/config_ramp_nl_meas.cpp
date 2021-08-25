#include "cgenalyzer.h"
#include "cgenalyzer_private.h"

extern "C" {
void config_ramp_nl_meas(config* c, size_t npts, double sample_rate, double full_scale_range, int resolution, double start, double stop, double irnoise)
{
    config c_private;
    c_private = (config)calloc(1, sizeof(*c_private));

    c_private->wf_type = RAMP;
    c_private->i_repr = I32;
    c_private->npts = npts;
    c_private->fs = sample_rate;
    c_private->fsr = full_scale_range;
    c_private->res = resolution;
    c_private->min_code = -(1 << (c_private->res - 1));
    c_private->max_code = -c_private->min_code - 1;
    c_private->ramp_start = start;
    c_private->ramp_stop = stop;
    c_private->cf = an::CodeFormat::TwosComplement;
    c_private->irnoise = irnoise;

    c_private->num_bins = (1 << c_private->res);
    c_private->num_hits = c_private->num_bins;

    *c = c_private;
}
}
