#include "cgenalyzer.h"
#include "cgenalyzer_private.h"
#include <dft.hpp>
#include <iostream>
#include <processes.hpp>

extern "C" {
void quantize64(gn_config c, double* awf, int64_t** result)
{
    an::int64_vector qwvf(c->npts);
    an::quantize(awf, c->npts, qwvf.data(), qwvf.size(), c->fsr, c->res, c->irnoise, c->cf, false, 1, {}, {});
    *result = new int64_t[qwvf.size()];
    memcpy(*result, &qwvf.front(), qwvf.size() * sizeof(int64_t));
}
}
