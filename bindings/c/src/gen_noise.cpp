#include "cgenalyzer.h"
#include "cgenalyzer_private.h"
#include <dft.hpp>
#include <iostream>
#include <waveforms.hpp>

extern "C" {
void gn_gen_noise(gn_config c, double** result)
{
    std::vector<an::real_t> awvf(c->npts);
    // an::noise(awvf.data(), awvf.size(), c->fsr, c->noise_pwr_db, 0.0);

    *result = new double[awvf.size()];
    memcpy(*result, &awvf.front(), awvf.size() * sizeof(double));
}
}
