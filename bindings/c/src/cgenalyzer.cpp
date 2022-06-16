#include "cgenalyzer.h"
#include "cgenalyzer_private.h"

extern "C" {
    int gn_config_tone(gn_config_tone_struct* c, waveform_type wf_type, size_t npts, double sample_rate, size_t num_tones, double *tone_freq, double *tone_ampl, double *tone_phase)
    {
        gn_config c_p;
        c_p = (gn_config)calloc(1, sizeof(*c_p));
        
        // assign arguments to configuration struct
        if ((wf_type == REAL_COSINE) || (wf_type == REAL_SINE) || (wf_type == COMPLEX_EXP))
            c_p->wf_type = wf_type;
        else {
            printf("ERROR: Invalid selection of wf_type for tone generation\n");
            return EXIT_FAILURE;
        }
        c_p->npts = npts;
        c_p->sample_rate = sample_rate;
        c_p->num_tones = num_tones;
        c_p->freq = tone_freq;
        c_p->scale = tone_ampl;
        c_p->phase = tone_phase;

        *c = c_p;
        return gn_success;
    }

    int gn_config_quantize(gn_config_quantize_struct* c, size_t npts, double fsr, int qres, double qnoise)
    {
        gn_config c_p;
        c_p = (gn_config)calloc(1, sizeof(*c_p));

        // assign arguments to configuration struct
        c_p->npts = npts;
        c_p->fsr = fsr;
        c_p->qres = qres;
        c_p->qnoise = qnoise;
        c_p->code_format = GnCodeFormatTwosComplement;

        *c = c_p;
        return gn_success;
    }

    int gn_gen_real_tone(double **out, gn_config c)
    {
        int err_code;
        double *awf = (double *)calloc(c->npts, sizeof(double));

        for (size_t i = 0; i < c->num_tones; i++) {
            double *tmp = (double *)calloc(c->npts, sizeof(double));
            if (c->wf_type == REAL_COSINE)
                err_code = gn_cos(tmp, c->npts, c->sample_rate, c->scale[i], c->freq[i], c->phase[i], 0, 0);
            else if (c->wf_type == REAL_SINE)
                err_code = gn_sin(tmp, c->npts, c->sample_rate, c->scale[i], c->freq[i], c->phase[i], 0, 0);
            if (!err_code) {
                for (size_t j = 0; j < c->npts; j++)
                    awf[j] = awf[j] + tmp[j];                        
            }
        }
        *out = awf;
        
        return err_code;
    }

    int gn_gen_complex_tone(double **outi, double **outq, gn_config c)
    {
        int err_code;
        double *awfi = (double *)calloc(c->npts, sizeof(double));
        double *awfq = (double *)calloc(c->npts, sizeof(double));

        for (size_t i = 0; i < c->num_tones; i++) {
            double *tmp = (double *)calloc(c->npts, sizeof(double));
            err_code = gn_cos(tmp, c->npts, c->sample_rate, c->scale[i], c->freq[i], c->phase[i], 0, 0);
            if (!err_code) {
                for (size_t j = 0; j < c->npts; j++)
                    awfi[j] = awfi[j] + tmp[j];                        
            }
            tmp = (double *)calloc(c->npts, sizeof(double));
            err_code = gn_sin(tmp, c->npts, c->sample_rate, c->scale[i], c->freq[i], c->phase[i], 0, 0);
            if (!err_code) {
                for (size_t j = 0; j < c->npts; j++)
                    awfq[j] = awfq[j] + tmp[j];                        
            }
        }
        *outi = awfi;
        *outq = awfq;
    
        return err_code;
    }

    int gn_quantize(int32_t **out, const double *in, gn_config c)
    {
        int err_code;
        int32_t *qwf = (int32_t *)calloc(c->npts, sizeof(int32_t));
        
        err_code = gn_quantize32(qwf, c->npts, in, c->npts, c->fsr, c->qres, c->qnoise, c->code_format);
        *out = qwf;

        return err_code;
    }
/*
    int gn_fftq(gn_config c, double* out, size_t out_size,
        const int16_t* i, size_t i_size, const int16_t* q, size_t q_size,
        int n, size_t navg, size_t nfft, GnWindow window)
    {
        return gn_fft16(out, out_size, i, i_size, q, q_size, n, navg, nfft, window, GnCodeFormatTwosComplement);
    }*/
}