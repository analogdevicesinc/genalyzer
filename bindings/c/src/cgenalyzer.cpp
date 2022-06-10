#include "cgenalyzer.h"
#include "cgenalyzer_private.h"

extern "C" {
    int gn_config_tone(gn_tone_config* c, waveform_type wf_type, size_t npts, double sample_rate, double tone_freq, double tone_ampl, double tone_phase)
    {
        gn_config c_p;
        c_p = (gn_config)calloc(1, sizeof(*c_p));

        // assign arguments to configuration struct
        if ((wf_type == REAL_COSINE) || (wf_type == REAL_SINE) || (wf_type == COMPLEX_EXP))
            c_p->wf_type = wf_type;
        else {
            printf("ERROR: Invalid selection of wf_type\n");
            return EXIT_FAILURE;
        }
        c_p->npts = npts;
        c_p->fs = sample_rate;
        c_p->freq = tone_freq;
        c_p->scale = tone_ampl;
        c_p->phase = tone_phase;        

        *c = c_p;
        return gn_success;
    }

    int gn_gen_tone(gn_config c, double **result, size_t *len)
    {
        if (c->wf_type == REAL_COSINE) {
            double *awf = (double *)malloc(c->npts*sizeof(double));
            gn_cos(awf, c->npts, c->fs, c->scale, c->freq, c->phase, 0, 0);
            *result = awf;
            for (int i=0; i<10; i++)
                printf("t%d\t%f\n",i,awf[i]);
            *len = c->npts;
        }
        else if (c->wf_type == REAL_SINE) {
            double *awf = (double *)malloc(c->npts*sizeof(double));
            gn_sin(awf, c->npts, c->fs, c->scale, c->freq, c->phase, 0, 0);
            *len = c->npts;
        }
        else if (c->wf_type == COMPLEX_EXP) {
            double *awfi = (double *)malloc(c->npts*sizeof(double));
            double *awfq = (double *)malloc(c->npts*sizeof(double));
            gn_cos(awfi, c->npts, c->fs, c->scale, c->freq, c->phase, 0, 0);
            gn_sin(awfq, c->npts, c->fs, c->scale, c->freq, c->phase, 0, 0);
            *len = c->npts;
        }
        // else // case to handle neither 
    }
/*
    int gn_fftq(gn_config c, double* out, size_t out_size,
        const int16_t* i, size_t i_size, const int16_t* q, size_t q_size,
        int n, size_t navg, size_t nfft, GnWindow window)
    {
        return gn_fft16(out, out_size, i, i_size, q, q_size, n, navg, nfft, window, GnCodeFormatTwosComplement);
    }*/
}