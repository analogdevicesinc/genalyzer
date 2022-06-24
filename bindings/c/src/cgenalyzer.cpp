#include "cgenalyzer.h"
#include "cgenalyzer_private.h"

extern "C" {
    // configuration
    int gn_config_create(gn_config_tone_struct* c)
    {
        gn_config c_p;
        c_p = (gn_config)calloc(1, sizeof(*c_p));
        
        *c = c_p;
        return gn_success;
    }

    int gn_config_tone(gn_config_tone_struct* c, waveform_type wf_type, size_t npts, gn::real_t sample_rate, size_t num_tones, gn::real_t *tone_freq, gn::real_t *tone_ampl, gn::real_t *tone_phase)
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

    int gn_config_quantize(gn_config_quantize_struct* c, size_t npts, gn::real_t fsr, int qres, gn::real_t qnoise)
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

    int gn_config_fa(gn_config c, gn::real_t fixed_tone_freq)
    {
        int err_code;

        if (c->sample_rate <= 0)
            c->sample_rate = 1;

        c->_all_fa_results_computed = false;

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

    // waveform generation
    int gn_gen_real_tone(gn::real_t **out, gn_config c)
    {
        int err_code;
        gn::real_t *awf = (gn::real_t *)calloc(c->npts, sizeof(gn::real_t));

        for (size_t i = 0; i < c->num_tones; i++) 
        {
            gn::real_t *tmp = (gn::real_t *)calloc(c->npts, sizeof(gn::real_t));
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

    int gn_gen_complex_tone(gn::real_t **outi, gn::real_t **outq, gn_config c)
    {
        int err_code;
        gn::real_t *awfi = (gn::real_t *)calloc(c->npts, sizeof(gn::real_t));
        gn::real_t *awfq = (gn::real_t *)calloc(c->npts, sizeof(gn::real_t));

        for (size_t i = 0; i < c->num_tones; i++) {
            gn::real_t *tmp = (gn::real_t *)calloc(c->npts, sizeof(gn::real_t));
            err_code = gn_cos(tmp, c->npts, c->sample_rate, c->scale[i], c->freq[i], c->phase[i], 0, 0);
            if (!err_code) 
            {
                for (size_t j = 0; j < c->npts; j++)
                    awfi[j] = awfi[j] + tmp[j];                        
            }
            tmp = (gn::real_t *)calloc(c->npts, sizeof(gn::real_t));
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

    // processing
    int gn_quantize(int32_t **out, const gn::real_t *in, gn_config c)
    {
        int err_code;
        int32_t *qwf = (int32_t *)calloc(c->npts, sizeof(int32_t));

        err_code = gn_quantize32(qwf, c->npts, in, c->npts, c->fsr, c->qres, c->qnoise, c->code_format);
        *out = qwf;

        return err_code;
    }

    int gn_fftz(gn::real_t **out, const int32_t *in_i, const int32_t *in_q, gn_config c)
    {
        int err_code;
        gn::real_t *fft_of_in = (gn::real_t *)calloc(2*c->nfft, sizeof(gn::real_t));

        err_code = gn_fft32(fft_of_in, 2*c->nfft, in_i, c->npts, in_q, c->npts, c->qres, c->navg, c->nfft, c->win, c->code_format);
        *out = fft_of_in;

        return err_code;
    }

    int gn_histz(uint64_t **hist, size_t *hist_len, const int32_t *qwf, gn_config c)
    {
        int err_code;
        uint64_t *out = NULL;        

        err_code = gn_code_density_size(&(c->code_density_size), c->qres, c->code_format);

        out = (uint64_t *)calloc(c->code_density_size, sizeof(uint64_t));
        err_code += gn_hist32(out, c->code_density_size, qwf, c->npts, c->qres, c->code_format, false);
        *hist = out;
        *hist_len = c->code_density_size;        

        return err_code;
    }

    int gn_dnlz(double **dnl, size_t *dnl_len, const uint64_t *hist, gn_config c)
    {
        int err_code;        
        double *out = (double *)calloc(c->code_density_size, sizeof(double));

        err_code = gn_dnl(out, c->code_density_size, hist, c->code_density_size, c->dnla_signal_type);
        *dnl = out;
        *dnl_len = c->code_density_size;        

        return err_code;
    }

    int gn_inlz(double **inl, size_t *inl_len, const double *dnl, gn_config c)
    {
        int err_code;        
        double *out = (double *)calloc(c->code_density_size, sizeof(double));

        err_code = gn_inl(out, c->code_density_size, dnl, c->code_density_size, c->inla_fit);
        *inl = out;
        *inl_len = c->code_density_size;        

        return err_code;
    }

    // Waveform/Histogram/DNL/INL/Fourier Analysis
    int gn_get_wfa_results(char ***rkeys, gn::real_t **rvalues, size_t *results_size, const int32_t *qwf, gn_config c)
    {
        int err_code = 0;

        // get results size
        err_code = gn_analysis_results_size(&(c->_wfa_results_size), GnAnalysisTypeWaveform);
        
        // allocate memory for result keys and values
        c->_wfa_result_keys = (char **)calloc((c->_wfa_results_size), sizeof(char*));
        c->_wfa_result_values = (gn::real_t *)calloc((c->_wfa_results_size), sizeof(gn::real_t));
        
        // get result key sizes
        c->_wfa_result_key_sizes = (size_t *)calloc((c->_wfa_results_size), sizeof(size_t));
        err_code += gn_analysis_results_key_sizes(c->_wfa_result_key_sizes, c->_wfa_results_size, GnAnalysisTypeWaveform);
        
        // allocate memory for each result key
        for (size_t i = 0; i < c->_wfa_results_size; ++i)
            c->_wfa_result_keys[i] = (char *)calloc(c->_wfa_result_key_sizes[i], sizeof(char));
        
        // execute analysis
        err_code += gn_wf_analysis32(c->_wfa_result_keys, c->_wfa_results_size, c->_wfa_result_values, c->_wfa_results_size, qwf, c->npts);

        // copy keys
        *rkeys = (char **)calloc((c->_wfa_results_size), sizeof(char*));
        for (size_t i = 0; i < c->_wfa_results_size; ++i)
            (*rkeys)[i] = (char *)calloc(c->_wfa_result_key_sizes[i], sizeof(char));

        // copy values
        *rvalues = (gn::real_t *)calloc((c->_wfa_results_size), sizeof(gn::real_t));
        for (size_t i = 0; i < c->_wfa_results_size; ++i) 
        {
            strcpy((*rkeys)[i], c->_wfa_result_keys[i]);
            (*rvalues)[i] = c->_wfa_result_values[i];
        }
        *results_size = c->_wfa_results_size;

        return(err_code);
    }

    int gn_get_ha_results(char ***rkeys, gn::real_t **rvalues, size_t *results_size, const uint64_t *hist, gn_config c)
    {
        int err_code = 0;

        // get results size
        err_code = gn_analysis_results_size(&(c->_hist_results_size), GnAnalysisTypeHistogram);
        
        // allocate memory for result keys and values
        c->_hist_result_keys = (char **)calloc((c->_hist_results_size), sizeof(char*));
        c->_hist_result_values = (gn::real_t *)calloc((c->_hist_results_size), sizeof(gn::real_t));
        
        // get result key sizes
        c->_hist_result_key_sizes = (size_t *)calloc((c->_hist_results_size), sizeof(size_t));
        err_code += gn_analysis_results_key_sizes(c->_hist_result_key_sizes, c->_hist_results_size, GnAnalysisTypeHistogram);
        
        // allocate memory for each result key
        for (size_t i = 0; i < c->_hist_results_size; ++i)
            c->_hist_result_keys[i] = (char *)calloc(c->_hist_result_key_sizes[i], sizeof(char));
        
        // execute analysis
        err_code += gn_hist_analysis(c->_hist_result_keys, c->_hist_results_size, c->_hist_result_values, c->_hist_results_size, hist, c->code_density_size);

        // copy keys
        *rkeys = (char **)calloc((c->_hist_results_size), sizeof(char*));
        for (size_t i = 0; i < c->_hist_results_size; ++i)
            (*rkeys)[i] = (char *)calloc(c->_hist_result_key_sizes[i], sizeof(char));

        // copy values
        *rvalues = (gn::real_t *)calloc((c->_hist_results_size), sizeof(gn::real_t));
        for (size_t i = 0; i < c->_hist_results_size; ++i) 
        {
            strcpy((*rkeys)[i], c->_hist_result_keys[i]);
            (*rvalues)[i] = c->_hist_result_values[i];
        }
        *results_size = c->_hist_results_size;

        return(err_code);
    }

    int gn_dnl_analysis_results(char ***rkeys, gn::real_t **rvalues, size_t *results_size, const gn::real_t *dnl, gn_config c)
    {
        int err_code;

        // get results size
        err_code = gn_analysis_results_size(&(c->_dnl_results_size), GnAnalysisTypeDNL);
        
        // allocate memory for result keys and values
        c->_dnl_result_keys = (char **)calloc((c->_dnl_results_size), sizeof(char*));
        c->_dnl_result_values = (gn::real_t *)calloc((c->_dnl_results_size), sizeof(gn::real_t));
        
        // get result key sizes
        c->_dnl_result_key_sizes = (size_t *)calloc((c->_dnl_results_size), sizeof(size_t));
        err_code += gn_analysis_results_key_sizes(c->_dnl_result_key_sizes, c->_dnl_results_size, GnAnalysisTypeDNL);
        
        // allocate memory for each result key
        for (size_t i = 0; i < c->_dnl_results_size; ++i)
            c->_dnl_result_keys[i] = (char *)calloc(c->_dnl_result_key_sizes[i], sizeof(char));
        
        // execute analysis
        err_code += gn_dnl_analysis(c->_dnl_result_keys, c->_dnl_results_size, c->_dnl_result_values, c->_dnl_results_size, dnl, c->code_density_size);

        // copy keys
        *rkeys = (char **)calloc((c->_dnl_results_size), sizeof(char*));
        for (size_t i = 0; i < c->_dnl_results_size; ++i)
            (*rkeys)[i] = (char *)calloc(c->_dnl_result_key_sizes[i], sizeof(char));

        // copy values
        *rvalues = (gn::real_t *)calloc((c->_dnl_results_size), sizeof(gn::real_t));
        for (size_t i = 0; i < c->_dnl_results_size; ++i) 
        {
            strcpy((*rkeys)[i], c->_dnl_result_keys[i]);
            (*rvalues)[i] = c->_dnl_result_values[i];
        }
        *results_size = c->_dnl_results_size;

        return(err_code);
    }

    int gn_inl_analysis_results(char ***rkeys, gn::real_t **rvalues, size_t *results_size, const gn::real_t *inl, gn_config c)
    {
        int err_code;

        // get results size
        err_code = gn_analysis_results_size(&(c->_inl_results_size), GnAnalysisTypeINL);
        
        // allocate memory for result keys and values
        c->_inl_result_keys = (char **)calloc((c->_inl_results_size), sizeof(char*));
        c->_inl_result_values = (gn::real_t *)calloc((c->_inl_results_size), sizeof(gn::real_t));
        
        // get result key sizes
        c->_inl_result_key_sizes = (size_t *)calloc((c->_inl_results_size), sizeof(size_t));
        err_code += gn_analysis_results_key_sizes(c->_inl_result_key_sizes, c->_inl_results_size, GnAnalysisTypeINL);
        
        // allocate memory for each result key
        for (size_t i = 0; i < c->_inl_results_size; ++i)
            c->_inl_result_keys[i] = (char *)calloc(c->_inl_result_key_sizes[i], sizeof(char));
        
        // execute analysis
        err_code += gn_inl_analysis(c->_inl_result_keys, c->_inl_results_size, c->_inl_result_values, c->_inl_results_size, inl, c->code_density_size);

        // copy keys
        *rkeys = (char **)calloc((c->_inl_results_size), sizeof(char*));
        for (size_t i = 0; i < c->_inl_results_size; ++i)
            (*rkeys)[i] = (char *)calloc(c->_inl_result_key_sizes[i], sizeof(char));

        // copy values
        *rvalues = (gn::real_t *)calloc((c->_inl_results_size), sizeof(gn::real_t));
        for (size_t i = 0; i < c->_inl_results_size; ++i) 
        {
            strcpy((*rkeys)[i], c->_inl_result_keys[i]);
            (*rvalues)[i] = c->_inl_result_values[i];
        }
        *results_size = c->_inl_results_size;

        return(err_code);
    }
    
    int gn_get_fa_single_result(gn::real_t *rvalue, const char *metric_name, gn::real_t *fft_ilv, gn_config c)
    {
        int err_code;
        size_t i;
        bool metric_found = false;

        // compute all results 
        err_code = gn_get_fa_results(NULL, NULL, NULL, fft_ilv, c);
        for (i = 0; i < c->_fa_results_size; i++) 
        {
            if (!strcmp(metric_name, c->_fa_result_keys[i])) 
            {
                metric_found = true;
                break;
            }            
        }
        if (!metric_found)
        {
            printf("ERROR: Invalid selection of metric\n");
            return EXIT_FAILURE;
        }
        *rvalue = c->_fa_result_values[i];

        return err_code;
    }

    int gn_get_fa_results(char ***rkeys, gn::real_t **rvalues, size_t *results_size, gn::real_t *fft_ilv, gn_config c)
    {
        int err_code = 0;

        if (!(c->_all_fa_results_computed))
        {
            // get results size
            err_code = gn_fft_analysis_results_size(&(c->_fa_results_size), c->obj_key, 2*c->nfft, c->nfft);
            
            // allocate memory for result keys and values
            c->_fa_result_keys = (char **)calloc((c->_fa_results_size), sizeof(char*));
            c->_fa_result_values = (gn::real_t *)calloc((c->_fa_results_size), sizeof(gn::real_t));
            
            // get result key sizes
            c->_fa_result_key_sizes = (size_t *)calloc((c->_fa_results_size), sizeof(size_t));
            err_code += gn_fft_analysis_results_key_sizes(c->_fa_result_key_sizes, c->_fa_results_size, c->obj_key, 2*c->nfft, c->nfft);
            
            // allocate memory for each result key
            for (size_t i = 0; i < c->_fa_results_size; ++i)
                c->_fa_result_keys[i] = (char *)calloc(c->_fa_result_key_sizes[i], sizeof(char));
            
            // execute analysis
            err_code += gn_fft_analysis(c->_fa_result_keys, c->_fa_results_size, c->_fa_result_values, c->_fa_results_size, c->obj_key, fft_ilv, 2*c->nfft, c->nfft, GnFreqAxisTypeDcCenter);

            if (err_code == 0)
                c->_all_fa_results_computed = true;
        }

        if (rkeys && rvalues && results_size) 
        {
            // copy keys
            *rkeys = (char **)calloc((c->_fa_results_size), sizeof(char*));
            for (size_t i = 0; i < c->_fa_results_size; ++i)
                (*rkeys)[i] = (char *)calloc(c->_fa_result_key_sizes[i], sizeof(char));

            // copy values
            *rvalues = (gn::real_t *)calloc((c->_fa_results_size), sizeof(gn::real_t));
            for (size_t i = 0; i < c->_fa_results_size; ++i) 
            {
                strcpy((*rkeys)[i], c->_fa_result_keys[i]);
                (*rvalues)[i] = c->_fa_result_values[i];
            }
            *results_size = c->_fa_results_size;
        }
        
        return (err_code);
    }
}