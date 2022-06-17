#include "cgenalyzer.h"
#include "cgenalyzer_private.h"

extern "C" {
    int gn_config_tone(gn_config_tone_struct* c, waveform_type wf_type, size_t npts, double sample_rate, size_t num_tones, double *tone_freq, double *tone_ampl, double *tone_phase)
    {
        if (!((wf_type == REAL_COSINE) || (wf_type == REAL_SINE) || (wf_type == COMPLEX_EXP)))
        {
            printf("ERROR: Invalid selection of wf_type for tone generation\n");
            return EXIT_FAILURE;
        }
        
        // assign arguments to configuration struct
        gn_config c_p;
        c_p = (gn_config)calloc(1, sizeof(*c_p));
        
        c_p->wf_type = wf_type;
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

    int gn_config_fft(gn_config_fft_struct* c, size_t npts, int qres, size_t navg, size_t nfft, GnWindow win)
    {
        if (npts != (navg*nfft))
        {
            printf("ERROR: Number of samples points in the waveform has to equal FFT order times number of FFT averages\n");
            return EXIT_FAILURE;
        }

        // assign arguments to configuration struct
        gn_config c_p;
        c_p = (gn_config)calloc(1, sizeof(*c_p));

        // assign arguments to configuration struct
        c_p->npts = npts;
        c_p->qres = qres;
        c_p->navg = navg;
        c_p->nfft = nfft;
        c_p->win = win;
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

    int gn_fftz(double **out, const int32_t *in_i, const int32_t *in_q, gn_config c)
    {
        int err_code;
        double *fft_of_in = (double *)calloc(2*c->nfft, sizeof(double));

        if (!in_q)
            err_code = gn_fft32(fft_of_in, 2*c->nfft, in_i, c->npts, NULL, 0, c->qres, c->navg, c->nfft, c->win, c->code_format);
        else
            err_code = gn_fft32(fft_of_in, 2*c->nfft, in_i, c->npts, in_q, c->npts, c->qres, c->navg, c->nfft, c->win, c->code_format);

        *out = fft_of_in;

        return err_code;
    }
}