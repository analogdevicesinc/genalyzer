#include "cgenalyzer.h"
#include "cgenalyzer_private.h"

extern "C" {
void gn_compute_inl(gn_config c, int* qwf, int32_t** bins, uint64_t** hits, double** dnl_data, double** inl_data)
{
    *bins = new int32_t[c->num_bins];
    *hits = new uint64_t[c->num_bins];
    *dnl_data = new double[c->num_hits];
    *inl_data = new double[c->num_hits];

    an::hist(*hits, c->num_hits, qwf, c->npts, c->res, CodeFormat::TwosComplement, 1);
    if ((c->wf_type == REAL_COSINE) || (c->wf_type == REAL_SINE))
        an::dnl(*dnl_data, c->num_hits, *hits, c->num_hits, DnlSignal::Tone);
    else
        an::dnl(*dnl_data, c->num_hits, *hits, c->num_hits, DnlSignal::Ramp);
    an::inl(*inl_data, c->num_hits, *dnl_data, c->num_hits, InlLineFit::BestFit);
}
}
