#include "cgenalyzer.h"
#include "cgenalyzer_private.h"

extern "C" {
    int gn_gen_tone(gn_config c, double **result, size_t *len)
    {
        if (c->wf_type == REAL_COSINE) {
            double *awf = (double *)malloc(c->npts*sizeof(double));
            gn_cos(awf, c->npts, c->fs, c->scale, c->freq, c->phase, 0, 0);
        }
        else if (c->wf_type == REAL_SINE) {
            double *awf = (double *)malloc(c->npts*sizeof(double));
            gn_sin(awf, c->npts, c->fs, c->scale, c->freq, c->phase, 0, 0);
        }
        else if (c->wf_type == COMPLEX_EXP) {
            double *awfi = (double *)malloc(c->npts*sizeof(double));
            double *awfq = (double *)malloc(c->npts*sizeof(double));
            gn_cos(awfi, c->npts, c->fs, c->scale, c->freq, c->phase, 0, 0);
            gn_sin(awfq, c->npts, c->fs, c->scale, c->freq, c->phase, 0, 0);
        }
    }
/*
    int gn_fftq(gn_config c, double* out, size_t out_size,
        const int16_t* i, size_t i_size, const int16_t* q, size_t q_size,
        int n, size_t navg, size_t nfft, GnWindow window)
    {
        return gn_fft16(out, out_size, i, i_size, q, q_size, n, navg, nfft, window, GnCodeFormatTwosComplement);
    }*/
}