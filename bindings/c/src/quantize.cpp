#include "cgenalyzer.h"
#include "cgenalyzer_private.h"
#include <dft.hpp>
#include <processes.hpp>

extern "C" {
void quantize(config c, double* awf, int32_t** result)
{
    an::int32_vector qwvf(c->npts);
    an::quantize(awf, c->npts, qwvf.data(), qwvf.size(), c->fsr, c->res, c->irnoise, c->cf, false, 1, {}, {});
    *result = new int32_t[qwvf.size()];
    memcpy(*result, &qwvf.front(), qwvf.size() * sizeof(int32_t));
}
}

/*
extern "C" {
  void quantize(config c, double *awf, int32_t **result) {
    if (c->i_repr == I32) {
      an::int32_vector qwvf(c->npts*c->navg);
      an::quantize(awf, c->npts*c->navg, qwvf.data(), qwvf.size(), c->fsr, c->res, c->irnoise, c->cf, false, 1, {}, {});
      *result = new int32_t[qwvf.size()];
      memcpy(*result, &qwvf.front(), qwvf.size()*sizeof(int32_t));
    }
    else if (c->i_repr == I64) {
      an::int64_vector qwvf(c->npts*c->navg);
      an::quantize(awf, c->npts*c->navg, qwvf.data(), qwvf.size(), c->fsr, c->res, c->irnoise, c->cf, false, 1, {}, {});
      *result = new int64_t[qwvf.size()];
      memcpy(*result, &qwvf.front(), qwvf.size()*sizeof(int64_t));
    }
  }
}
*/
