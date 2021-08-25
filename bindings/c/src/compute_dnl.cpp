#include "cgenalyzer.h"
#include "cgenalyzer_private.h"

extern "C" {
void compute_dnl(config c, int* qwf, int32_t** bins, int64_t** hits, double** dnl_data)
{
    *bins = new int32_t[c->num_bins];
    *hits = new int64_t[c->num_bins];
    *dnl_data = new double[c->num_bins];
    // double *dnl_result = (double *)malloc(hits_len*sizeof(double));

    an::histogram(qwf, c->npts, *bins, c->num_bins, *hits, c->num_hits, c->min_code, c->max_code);
    if ((c->wf_type == REAL_COSINE) || (c->wf_type == REAL_SINE))
        an::dnl(*hits, c->num_hits, *dnl_data, c->num_hits, PmfType::Tone);
    else
        an::dnl(*hits, c->num_hits, *dnl_data, c->num_hits, PmfType::Ramp);

    // memcpy(*dnl_data, dnl_result, hits_len*sizeof(double));
}
}