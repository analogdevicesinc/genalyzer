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

    int gn_config_fa_tone(gn_config c, double fixed_tone_freq)
    {
        int err_code;

        if (c->sample_rate <= 0)
            c->sample_rate = 1;

        c->all_results_computed = false;

        c->obj_key = (char *)calloc(3, sizeof(char));
        strcpy(c->obj_key, "fa");
        c->comp_key = (char *)calloc(2, sizeof(char));
        strcpy(c->comp_key, "A");

        c->ssb_fund = 0;
        c->ssb_rest = 0;
        c->max_harm_order = 3;

        // configure object key for Fourier analysis
        err_code = gn_fa_create(c->obj_key);

        // configure component key for Fourier analysis
        err_code += gn_fa_fixed_tone(c->obj_key, c->comp_key, GnFACompTagSignal, fixed_tone_freq, c->ssb_fund);

        // configure harmonic order for Fourier analysis
        err_code += gn_fa_hd(c->obj_key, c->max_harm_order);

        // configure single-side bins for Fourier analysis
        err_code += gn_fa_ssb(c->obj_key, GnFASsbDefault, c->ssb_rest);
        err_code += gn_fa_ssb(c->obj_key, GnFASsbDC, -1);
        err_code += gn_fa_ssb(c->obj_key, GnFASsbSignal, -1);
        err_code += gn_fa_ssb(c->obj_key, GnFASsbWO, -1);

        // configure sample-rate, data-rate, shift frequency, and converter offset
        err_code += gn_fa_fsample(c->obj_key, c->sample_rate);
        err_code += gn_fa_fdata(c->obj_key, c->sample_rate);
        err_code += gn_fa_fshift(c->obj_key, 0.0);
        err_code += gn_fa_conv_offset(c->obj_key, false);

        return (err_code);
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

        for (size_t i = 0; i < c->num_tones; i++) 
        {
            double *tmp = (double *)calloc(c->npts, sizeof(double));
            if (c->wf_type == REAL_COSINE)
                err_code = gn_cos(tmp, c->npts, c->sample_rate, c->scale[i], c->freq[i], c->phase[i], 0, 0);
            else if (c->wf_type == REAL_SINE)
                err_code = gn_sin(tmp, c->npts, c->sample_rate, c->scale[i], c->freq[i], c->phase[i], 0, 0);
            if (!err_code) 
            {
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
            if (!err_code) 
            {
                for (size_t j = 0; j < c->npts; j++)
                    awfi[j] = awfi[j] + tmp[j];                        
            }
            tmp = (double *)calloc(c->npts, sizeof(double));
            err_code = gn_sin(tmp, c->npts, c->sample_rate, c->scale[i], c->freq[i], c->phase[i], 0, 0);
            if (!err_code) 
            {
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

    int gn_fa_get_single_result(double *rvalue, const char *metric_name, double *fft_ilv, gn_config c)
    {
        int err_code;
        size_t i;

        // compute all results 
        err_code = gn_fa_get_all_results(NULL, NULL, NULL, fft_ilv, c);
        for (i = 0; i < c->results_size; i++) 
        {
            if (!strcmp(metric_name, c->rkeys[i]))
                break;
            else
            {
                printf("ERROR: Invalid selection of metric\n");
                return EXIT_FAILURE;
            }
        }
        *rvalue = c->rvalues[i];

        return err_code;
    }

    int gn_fa_get_all_results(char ***rkeys, double **rvalues, size_t *results_size, double *fft_ilv, gn_config c)
    {
        int err_code = 0;

        if (!(c->all_results_computed))
        {
            // get results size
            err_code = gn_fft_analysis_results_size(&(c->results_size), c->obj_key, 2*c->nfft, c->nfft);
            
            // allocate memory for result keys and values
            c->rkeys = (char **)calloc((c->results_size), sizeof(char*));
            c->rvalues = (double *)calloc((c->results_size), sizeof(double));
            
            // get result key sizes
            c->rkey_sizes = (size_t *)calloc((c->results_size), sizeof(size_t));
            err_code += gn_fft_analysis_results_key_sizes(c->rkey_sizes, c->results_size, c->obj_key, 2*c->nfft, c->nfft);
            
            // allocate memory for each result key
            for (size_t i = 0; i < c->results_size; ++i)
                c->rkeys[i] = (char *)calloc(c->rkey_sizes[i], sizeof(char));
            
            // execute analysis
            err_code += gn_fft_analysis(c->rkeys, c->results_size, c->rvalues, c->results_size, c->obj_key, fft_ilv, 2*c->nfft, c->nfft, GnFreqAxisTypeDcCenter);

            c->all_results_computed = true;
        }

        if (rkeys && rvalues && results_size) 
        {
            // copy keys
            *rkeys = (char **)calloc((c->results_size), sizeof(char*));
            for (size_t i = 0; i < c->results_size; ++i)
                (*rkeys)[i] = (char *)calloc(c->rkey_sizes[i], sizeof(char));

            // copy values
            *rvalues = (double *)calloc((c->results_size), sizeof(double));
            for (size_t i = 0; i < c->results_size; ++i) 
            {
                strcpy((*rkeys)[i], c->rkeys[i]);
                (*rvalues)[i] = c->rvalues[i];
            }
            *results_size = c->results_size;
        }
        
        return (err_code);
    }
}