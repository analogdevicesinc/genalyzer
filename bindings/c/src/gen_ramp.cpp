#include "cgenalyzer.h"
#include "cgenalyzer_private.h"
#include <dft.hpp>
#include <iostream>
#include <waveforms.hpp>

extern "C" {
void gn_gen_ramp(gn_config c, double** result, size_t* len)
{
    std::vector<an::real_t> awvf(c->npts);
    an::ramp(awvf.data(), awvf.size(), c->ramp_start, c->ramp_stop, 0.0);

    *result = new double[awvf.size()];
    memcpy(*result, &awvf.front(), awvf.size() * sizeof(double));
    *len = c->npts;
}
}
