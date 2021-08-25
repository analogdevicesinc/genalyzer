#include "cgenalyzer.h"
#include "cgenalyzer_private.h"
#include <dft.hpp>
#include <iostream>
#include <waveforms.hpp>

extern "C" {
void gen_ramp(config c, double** result, size_t* len)
{
    an::real_vector awvf(c->npts);
    an::ramp(awvf.data(), awvf.size(), c->ramp_start, c->ramp_stop, 0.0);

    *result = new double[awvf.size()];
    memcpy(*result, &awvf.front(), awvf.size() * sizeof(double));
    *len = c->npts;
}
}
