#include "cgenalyzer.h"
#include "cgenalyzer_private.h"
#include <dft.hpp>
#include <processes.hpp>

extern "C" {
void gn_quantize(gn_config c, const double* awf, int32_t** result)
{
    an::int32_vector qwvf(c->npts);
    an::quantize(awf, c->npts, qwvf.data(), qwvf.size(), c->fsr, c->res, c->irnoise, c->cf, false, 1, {}, {});
    *result = new int32_t[qwvf.size()];
    memcpy(*result, &qwvf.front(), qwvf.size() * sizeof(int32_t));
}
}
