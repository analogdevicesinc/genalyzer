#include "cgenalyzer.h"
#include "cgenalyzer_private.h"

extern "C" {
    int gn_config_tone(gn_config_tone_struct* c, waveform_type wf_type, size_t npts, double sample_rate, size_t num_tones, double *tone_freq, double *tone_ampl, double *tone_phase)
    {
        bool fa_config_success;

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

        // configure object keys for Fourier analysis
        c_p->obj_key = (char **)calloc(1, sizeof(char *));
        c_p->obj_key[0] = (char *)calloc(6, sizeof(char));
        strcpy(c_p->obj_key[0], "fa0");
        c_p->num_obj_keys = c_p->num_obj_keys + 1;
        fa_config_success = gn_fa_create(c_p->obj_key[0]);

        // configure component keys for Fourier analysis
        c_p->comp_key = (char **)calloc(1, sizeof(char *));
        c_p->comp_key[0] = (char *)calloc(2, sizeof(char));
        strcpy(c_p->comp_key[0], "A");
        c_p->num_comp_keys = c_p->num_comp_keys + 1;
        fa_config_success |= gn_fa_fixed_tone(c_p->obj_key[0], c_p->comp_key[0], GnFACompTagSignal, c_p->freq[0], c_p->ssb_fund);

        // configure harmonic order for Fourier analysis
        fa_config_success |= gn_fa_hd(c_p->obj_key[0], c_p->max_harm_order);

        // configure single-side bins for Fourier analysis
        fa_config_success |= gn_fa_ssb(c_p->obj_key[0], GnFASsbDefault, c_p->ssb_rest);
        fa_config_success |= gn_fa_ssb(c_p->obj_key[0], GnFASsbDC, -1);
        fa_config_success |= gn_fa_ssb(c_p->obj_key[0], GnFASsbSignal, -1);
        fa_config_success |= gn_fa_ssb(c_p->obj_key[0], GnFASsbWO, -1);

        // configure sample-rate, data-rate, shift frequency, and converter offset
        fa_config_success |= gn_fa_fsample(c_p->obj_key[0], c_p->sample_rate);
        fa_config_success |= gn_fa_fdata(c_p->obj_key[0], c_p->sample_rate);
        fa_config_success |= gn_fa_fshift(c_p->obj_key[0], 0.0);
        fa_config_success |= gn_fa_conv_offset(c_p->obj_key[0], false);

        *c = c_p;
        return (gn_success || fa_config_success);
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

        err_code = gn_fft32(fft_of_in, 2*c->nfft, in_i, c->npts, in_q, c->npts, c->qres, c->navg, c->nfft, c->win, c->code_format);
        *out = fft_of_in;

        return err_code;
    }

    int gn_fa_get_all_results(size_t **size, size_t obj_key_index, gn_config c)
    {
        bool success;

        // get results size
        success = gn_fft_analysis_results_size(*size, c->obj_key[obj_key_index], 2*c->nfft, c->nfft);
        // allocate memory for result keys and values
        c->rkeys = malloc(*size * sizeof(char*));
        c->rvalues = malloc(*size * sizeof(double));
        // get result key sizes
        c->rkey_sizes = malloc(*size * sizeof(size_t));
        success |= gn_fft_analysis_results_key_sizes(c->rkey_sizes, *size, c->obj_key[obj_key_index], 2*c->nfft, c->nfft);
        // allocate memory for each result key
        for (size_t i = 0; i < fft_results_size; ++i)
            fft_rkeys[i] = malloc(fft_rkey_sizes[i]);
        // execute analysis
        success |= gn_fft_analysis(c->rkeys, **size, c->rvalues, **size, c->obj_key[obj_key_index], fft_cplx, 2*c->nfft, c->nfft, axis_type);
    }
}