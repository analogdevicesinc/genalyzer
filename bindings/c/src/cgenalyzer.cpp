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
        c_p->wf_type_set = true;

        c_p->npts = npts;
        c_p->npts_set = true;
        
        c_p->sample_rate = sample_rate;
        c_p->sample_rate_set = true;
        
        c_p->num_tones = num_tones;
        c_p->num_tones_set = true;

        c_p->freq = tone_freq;
        c_p->freq_set = true;
        
        c_p->scale = tone_ampl;
        c_p->scale_set = true;
        
        c_p->phase = tone_phase;
        c_p->phase_set = true;

        *c = c_p;
        return gn_success;
    }

    int gn_gen_tone(gn_config c, double **result)
    {
        int err_code;
        if (!c->npts_set) {
            printf("ERROR: size needed for tone waveform generation\n");
            return gn_failure;
        }
        if (!c->sample_rate_set) {
            printf("ERROR: sample rate needed for tone waveform generation\n");
            return gn_failure;
        }
        if (!c->num_tones_set) {
            printf("ERROR: num. tones needed for tone waveform generation\n");
            return gn_failure;
        }
        if (!c->scale_set) {
            printf("ERROR: scale needed for tone waveform generation\n");
            return gn_failure;
        }
        if (!c->freq_set) {
            printf("ERROR: freq needed for tone waveform generation\n");
            return gn_failure;
        }
        if (!c->phase_set) {
            printf("ERROR: phase needed for tone waveform generation\n");
            return gn_failure;
        }

        if (c->wf_type == REAL_COSINE) {
            double *awf = (double *)calloc(c->npts, sizeof(double));
            for (size_t i = 0; i < c->num_tones; i++) {
                double *tmp = (double *)calloc(c->npts, sizeof(double));printf("%f\t%f\t%f\n",c->scale[i], c->freq[i], c->phase[i]);
                err_code = gn_cos(tmp, c->npts, c->sample_rate, c->scale[i], c->freq[i], c->phase[i], 0, 0);
                if (!err_code) {
                    for (size_t j = 0; j < c->npts; j++)
                        awf[j] = awf[j] + tmp[j];                        
                }
            }
            *result = awf;
        }
        else if (c->wf_type == REAL_SINE) {
            double *awf = (double *)calloc(c->npts, sizeof(double));
            for (size_t i = 0; i < c->num_tones; i++) {
                double *tmp = (double *)calloc(c->npts, sizeof(double));
                err_code = gn_sin(tmp, c->npts, c->sample_rate, c->scale[i], c->freq[i], c->phase[i], 0, 0);
                if (!err_code) {
                    for (size_t j = 0; j < c->npts; j++)
                        awf[j] = awf[j] + tmp[j];                        
                }
            }
            *result = awf;
        }
        else if (c->wf_type == COMPLEX_EXP) {
            double *awf = (double *)calloc(2*c->npts, sizeof(double));
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
            for (size_t j = 0; j < c->npts; j++) {
                awf[2*j] = awfi[j];
                awf[2*j+1] = awfq[j];
            }
            *result = awf;
        }
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