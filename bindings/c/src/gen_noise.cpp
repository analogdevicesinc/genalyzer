#include "cgenalyzer.h"
#include "cgenalyzer_private.h"
#include <dft.hpp>
#include <iostream>
#include <waveforms.hpp>

extern "C" {
void gen_noise(config c, double** result)
{
    an::real_vector awvf(c->npts);
    an::noise(awvf.data(), awvf.size(), c->fsr, c->noise_pwr_db, 0.0);

    *result = new double[awvf.size()];
    memcpy(*result, &awvf.front(), awvf.size() * sizeof(double));
}
}
