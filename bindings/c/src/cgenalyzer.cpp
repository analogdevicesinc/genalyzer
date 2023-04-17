#include "cgenalyzer.h"
#include "cgenalyzer_private.h"

extern "C" {
    int gn_config_free(gn_config *c)
    {
        if (!((*c)->obj_key))
            free((*c)->obj_key);
        if (!((*c)->comp_key))
            free((*c)->comp_key);
        if (!((*c)->tone_freq))
            free((*c)->tone_freq);
        if (!((*c)->tone_ampl))
            free((*c)->tone_ampl);
        if (!((*c)->tone_phase))
            free((*c)->tone_phase);
        if (((*c)->_fa_results_size) > 0)
        {
            for (size_t i = 0; i < (*c)->_fa_results_size; i++)
                free((*c)->_fa_result_keys[i]);
            free((*c)->_fa_result_keys);
            free((*c)->_fa_result_key_sizes);
            free((*c)->_fa_result_values);
        }
        if (((*c)->_wfa_results_size) > 0)
        {
            for (size_t i = 0; i < (*c)->_wfa_results_size; i++)
                free((*c)->_wfa_result_keys[i]);
            free((*c)->_wfa_result_keys);
            free((*c)->_wfa_result_key_sizes);
            free((*c)->_wfa_result_values);
        }
        if (((*c)->_hist_results_size) > 0)
        {
            for (size_t i = 0; i < (*c)->_hist_results_size; i++)
                free((*c)->_hist_result_keys[i]);
            free((*c)->_hist_result_keys);
            free((*c)->_hist_result_key_sizes);
            free((*c)->_hist_result_values);
        }
        if (((*c)->_dnl_results_size) > 0)
        {
            for (size_t i = 0; i < (*c)->_dnl_results_size; i++)
                free((*c)->_dnl_result_keys[i]);
            free((*c)->_dnl_result_keys);
            free((*c)->_dnl_result_key_sizes);
            free((*c)->_dnl_result_values);
        }
        if (((*c)->_inl_results_size) > 0)
        {
            for (size_t i = 0; i < (*c)->_inl_results_size; i++)
                free((*c)->_inl_result_keys[i]);
            free((*c)->_inl_result_keys);
            free((*c)->_inl_result_key_sizes);
            free((*c)->_inl_result_values);
        }
        free(*c);

        return gn_success;
    }

    int gn_config_set_ttype(tone_type ttype, gn_config *c)
    {
        if (!((ttype == REAL_COSINE) || (ttype == REAL_SINE) || (ttype == COMPLEX_EXP)))
        {
            printf("ERROR: Invalid selection of ttype for tone generation\n");
            return gn_failure;
        }        
        
        if (!(*c)) 
        {
            gn_config c_p;
            c_p = (gn_config)calloc(1, sizeof(*c_p));
            if (!(c_p)) 
            {
                printf("insufficient memory\n");
                return ENOMEM;
            }
            else
                *c = c_p;
        }        
        (*c)->ttype = ttype;

        return gn_success;
    }

    int gn_config_set_npts(size_t npts, gn_config *c)
    {
        if (!(*c)) 
        {
            gn_config c_p;
            c_p = (gn_config)calloc(1, sizeof(*c_p));
            if (!(c_p)) 
            {
                printf("insufficient memory\n");
                return ENOMEM;
            }
            else
                *c = c_p;
        }
        (*c)->npts = npts;
        return gn_success;
    }

    int gn_config_get_npts(size_t *npts, gn_config *c)
    {
        if (!(*c)) 
        {
            printf("config struct is NULL\n");
            return gn_failure;
        }
        *npts = (*c)->npts;
        return gn_success;
    }

    int gn_config_set_sample_rate(gn::real_t sample_rate, gn_config *c)
    {
        if (!(*c)) 
        {
            gn_config c_p;
            c_p = (gn_config)calloc(1, sizeof(*c_p));
            if (!(c_p)) 
            {
                printf("insufficient memory\n");
                return ENOMEM;
            }
            else
                *c = c_p;
        }
        (*c)->sample_rate = sample_rate;
        return gn_success;
    }

    int gn_config_get_sample_rate(double *sample_rate, gn_config *c)
    {
        if (!(*c)) 
        {
            printf("config struct is NULL\n");
            return gn_failure;
        }
        *sample_rate = (*c)->sample_rate;
        return gn_success;
    }
    
    int gn_config_set_data_rate(gn::real_t data_rate, gn_config *c)
    {
        if (!(*c)) 
        {
            gn_config c_p;
            c_p = (gn_config)calloc(1, sizeof(*c_p));
            if (!(c_p)) 
            {
                printf("insufficient memory\n");
                return ENOMEM;
            }
            else
                *c = c_p;
        }
        (*c)->data_rate = data_rate;
        return gn_success;
    }

    int gn_config_set_shift_freq(gn::real_t shift_freq, gn_config *c)
    {
        if (!(*c)) 
        {
            gn_config c_p;
            c_p = (gn_config)calloc(1, sizeof(*c_p));
            if (!(c_p)) 
            {
                printf("insufficient memory\n");
                return ENOMEM;
            }
            else
                *c = c_p;
        }
        (*c)->shift_freq = shift_freq;
        return gn_success;
    }

    int gn_config_set_num_tones(size_t num_tones, gn_config *c)
    {
        if (!(*c)) 
        {
            gn_config c_p;
            c_p = (gn_config)calloc(1, sizeof(*c_p));
            if (!(c_p)) 
            {
                printf("insufficient memory\n");
                return ENOMEM;
            }
            else
                *c = c_p;
        }
        (*c)->num_tones = num_tones;
        return gn_success;
    }

    int gn_config_set_tone_freq(gn::real_t *tone_freq, gn_config *c)
    {
        if (!(*c)) 
        {
            gn_config c_p;
            c_p = (gn_config)calloc(1, sizeof(*c_p));
            if (!(c_p)) 
            {
                printf("insufficient memory\n");
                return ENOMEM;
            }
            else
                *c = c_p;
        }
        (*c)->tone_freq = tone_freq;
        return gn_success;
    }

    int gn_config_set_tone_ampl(gn::real_t *tone_ampl, gn_config *c)
    {
        if (!(*c)) 
        {
            gn_config c_p;
            c_p = (gn_config)calloc(1, sizeof(*c_p));
            if (!(c_p)) 
            {
                printf("insufficient memory\n");
                return ENOMEM;
            }
            else
                *c = c_p;
        }
        (*c)->tone_ampl = tone_ampl;
        return gn_success;
    }

    int gn_config_set_tone_phase(gn::real_t *tone_phase, gn_config *c)
    {
        if (!(*c)) 
        {
            gn_config c_p;
            c_p = (gn_config)calloc(1, sizeof(*c_p));
            if (!(c_p)) 
            {
                printf("insufficient memory\n");
                return ENOMEM;
            }
            else
                *c = c_p;
        }
        (*c)->tone_phase = tone_phase;
        return gn_success;
    }

    int gn_config_set_fsr(gn::real_t fsr, gn_config *c)
    {
        if (!(*c)) 
        {
            gn_config c_p;
            c_p = (gn_config)calloc(1, sizeof(*c_p));
            if (!(c_p)) 
            {
                printf("insufficient memory\n");
                return ENOMEM;
            }
            else
                *c = c_p;
        }
        (*c)->fsr = fsr;
        return gn_success;
    }

    int gn_config_set_qres(int qres, gn_config *c)
    {
        if (!(*c)) 
        {
            gn_config c_p;
            c_p = (gn_config)calloc(1, sizeof(*c_p));
            if (!(c_p)) 
            {
                printf("insufficient memory\n");
                return ENOMEM;
            }
            else
                *c = c_p;
        }
        (*c)->qres = qres;
        return gn_success;
    }

    int gn_config_set_noise_rms(gn::real_t noise_rms, gn_config *c)
    {
        if (!(*c)) 
        {
            gn_config c_p;
            c_p = (gn_config)calloc(1, sizeof(*c_p));
            if (!(c_p)) 
            {
                printf("insufficient memory\n");
                return ENOMEM;
            }
            else
                *c = c_p;
        }
        (*c)->noise_rms = noise_rms;
        return gn_success;
    }

    int gn_config_set_code_format(GnCodeFormat code_format, gn_config *c)
    {
        if (!((code_format == GnCodeFormatOffsetBinary) || (code_format == GnCodeFormatTwosComplement)))
        {
            printf("ERROR: Invalid selection of code format\n");
            return gn_failure;
        }
        
        if (!(*c)) 
        {
            gn_config c_p;
            c_p = (gn_config)calloc(1, sizeof(*c_p));
            if (!(c_p)) 
            {
                printf("insufficient memory\n");
                return ENOMEM;
            }
            else
                *c = c_p;
        }
        (*c)->code_format = code_format;
        return gn_success;
    }

    int gn_config_set_nfft(size_t nfft, gn_config *c)
    {
        if (((*c)->nfft) > ((*c)->npts))
        {
            printf("ERROR: FFT order cannot be greater than the number of sample points\n");
            return gn_failure;
        }

        double rem = 1.0*(((*c)->npts)%((*c)->nfft));
        if (rem > 0)
        {
            printf("ERROR: FFT order has to be a multiple of the number of sample points\n");
            return gn_failure;
        }
        
        if (!(*c)) 
        {
            gn_config c_p;
            c_p = (gn_config)calloc(1, sizeof(*c_p));
            if (!(c_p)) 
            {
                printf("insufficient memory\n");
                return ENOMEM;
            }
            else
                *c = c_p;
        }
        (*c)->nfft = nfft;
        (*c)->fft_navg = (*c)->npts/(*c)->nfft;
        return gn_success;
    }
    
    int gn_config_get_nfft(size_t *nfft, gn_config *c)
    {
        if (!(*c)) 
        {
            printf("here - config struct is NULL\n");
            return gn_failure;
        }
        *nfft = (*c)->nfft;
        return gn_success;
    }

    int gn_config_set_fft_navg(size_t fft_navg, gn_config *c)
    {
        if (((*c)->fft_navg) > ((*c)->npts))
        {
            printf("ERROR: Number of FFT averages cannot be greater than the number of sample points\n");
            return gn_failure;
        }

        double rem = 1.0*(((*c)->npts)%((*c)->fft_navg));
        if (rem > 0)
        {
            printf("ERROR: Number of FFT averages has to be a multiple of the number of sample points\n");
            return gn_failure;
        }
        
        if (!(*c)) 
        {
            gn_config c_p;
            c_p = (gn_config)calloc(1, sizeof(*c_p));
            if (!(c_p)) 
            {
                printf("insufficient memory\n");
                return ENOMEM;
            }
            else
                *c = c_p;
        }
        (*c)->fft_navg = fft_navg;
        (*c)->nfft = (*c)->npts/(*c)->fft_navg;
        return gn_success;
    }
    
    int gn_config_set_win(GnWindow win, gn_config *c)
    {
        if (!((win == GnWindowBlackmanHarris) || (win == GnWindowHann) || (win == GnWindowNoWindow)))
        {
            printf("ERROR: Invalid selection of window function\n");
            return gn_failure;
        }
        
        if (!(*c)) 
        {
            gn_config c_p;
            c_p = (gn_config)calloc(1, sizeof(*c_p));
            if (!(c_p)) 
            {
                printf("insufficient memory\n");
                return ENOMEM;
            }
            else
                *c = c_p;
        }
        (*c)->win = win;
        return gn_success;
    }
    
    int gn_config_set_ssb_fund(int ssb_fund, gn_config *c)
    {
        if (!(*c)) 
        {
            gn_config c_p;
            c_p = (gn_config)calloc(1, sizeof(*c_p));
            if (!(c_p)) 
            {
                printf("insufficient memory\n");
                return ENOMEM;
            }
            else
                *c = c_p;
        }
        (*c)->ssb_fund = ssb_fund;
        return gn_success;
    }

    int gn_config_set_ssb_rest(int ssb_rest, gn_config *c)
    {
        if (!(*c)) 
        {
            gn_config c_p;
            c_p = (gn_config)calloc(1, sizeof(*c_p));
            if (!(c_p)) 
            {
                printf("insufficient memory\n");
                return ENOMEM;
            }
            else
                *c = c_p;
        }
        (*c)->ssb_rest = ssb_rest;
        return gn_success;
    }

    int gn_config_set_max_harm_order(int max_harm_order, gn_config *c)
    {
        if (!(*c)) 
        {
            gn_config c_p;
            c_p = (gn_config)calloc(1, sizeof(*c_p));
            if (!(c_p)) 
            {
                printf("insufficient memory\n");
                return ENOMEM;
            }
            else
                *c = c_p;
        }
        (*c)->max_harm_order = max_harm_order;
        return gn_success;
    }

    int gn_config_set_dnla_signal_type(GnDnlSignal dnla_signal_type, gn_config *c)
    {
        if (!((dnla_signal_type == GnDnlSignalRamp) || (dnla_signal_type == GnDnlSignalTone)))
        {
            printf("ERROR: Invalid selection of DNL analysis signal type\n");
            return gn_failure;
        }
        
        if (!(*c)) 
        {
            gn_config c_p;
            c_p = (gn_config)calloc(1, sizeof(*c_p));
            if (!(c_p)) 
            {
                printf("insufficient memory\n");
                return ENOMEM;
            }
            else
                *c = c_p;
        }
        (*c)->dnla_signal_type = dnla_signal_type;
        return gn_success;
    }

    int gn_config_set_inla_fit(GnInlLineFit inla_fit, gn_config *c)
    {
        if (!((inla_fit == GnInlLineFitBestFit) || (inla_fit == GnInlLineFitEndFit) || (inla_fit == GnInlLineFitNoFit)))
        {
            printf("ERROR: Invalid selection of INL line fit\n");
            return gn_failure;
        }
        
        if (!(*c)) 
        {
            gn_config c_p;
            c_p = (gn_config)calloc(1, sizeof(*c_p));
            if (!(c_p)) 
            {
                printf("insufficient memory\n");
                return ENOMEM;
            }
            else
                *c = c_p;
        }
        (*c)->inla_fit = inla_fit;
        return gn_success;
    }

    int gn_config_set_ramp_start(double ramp_start, gn_config *c)
    {
        if (!(*c)) 
        {
            gn_config c_p;
            c_p = (gn_config)calloc(1, sizeof(*c_p));
            if (!(c_p)) 
            {
                printf("insufficient memory\n");
                return ENOMEM;
            }
            else
                *c = c_p;
        }
        (*c)->ramp_start = ramp_start;
        return gn_success;
    }

    int gn_config_set_ramp_stop(double ramp_stop, gn_config *c)
    {
        if (!(*c)) 
        {
            gn_config c_p;
            c_p = (gn_config)calloc(1, sizeof(*c_p));
            if (!(c_p)) 
            {
                printf("insufficient memory\n");
                return ENOMEM;
            }
            else
                *c = c_p;
        }
        (*c)->ramp_stop = ramp_stop;
        return gn_success;
    }

    int gn_config_get_code_density_size(size_t *code_density_size, gn_config *c)
    {
        if (!(*c)) 
        {
            printf("config struct is NULL\n");
            return gn_failure;
        }
        *code_density_size = (*c)->_code_density_size;
        return gn_success;
    }

    int gn_config_gen_tone(tone_type ttype, size_t npts, gn::real_t sample_rate, size_t num_tones, gn::real_t *tone_freq, gn::real_t *tone_ampl, gn::real_t *tone_phase, gn_config *c)
    {
        if (!((ttype == REAL_COSINE) || (ttype == REAL_SINE) || (ttype == COMPLEX_EXP)))
        {
            printf("ERROR: Invalid selection of waveform type for tone generation\n");
            return gn_failure;
        }

        if (!(*c)) 
        {
            gn_config c_p;
            c_p = (gn_config)calloc(1, sizeof(*c_p));
            if (!(c_p)) 
            {
                printf("insufficient memory\n");
                return ENOMEM;
            }
            else
                *c = c_p;
        }
        (*c)->ttype = ttype;
        (*c)->npts = npts;
        (*c)->sample_rate = sample_rate;
        (*c)->num_tones = num_tones;
        (*c)->tone_freq = tone_freq;
        (*c)->tone_ampl = tone_ampl;
        (*c)->tone_phase = tone_phase;

        return gn_success;
    }

    int gn_config_gen_ramp(size_t npts, double ramp_start, double ramp_stop, gn_config *c)
    {
        if (ramp_stop < ramp_start)
        {
            printf("ERROR: ramp stop value cannot be smaller than ramp start value for ramp generation\n");
            return gn_failure;
        }
        
        if (!(*c)) 
        {
            gn_config c_p;
            c_p = (gn_config)calloc(1, sizeof(*c_p));
            if (!(c_p)) 
            {
                printf("insufficient memory\n");
                return ENOMEM;
            }
            else
                *c = c_p;
        }
        (*c)->npts = npts;
        (*c)->ramp_start = ramp_start;
        (*c)->ramp_stop = ramp_stop;
        (*c)->noise_rms = 0.0;
        
        return gn_success;
    }
    
    int gn_config_quantize(size_t npts, gn::real_t fsr, int qres, gn::real_t noise_rms, gn_config *c)
    {
        if (!(*c)) 
        {
            gn_config c_p;
            c_p = (gn_config)calloc(1, sizeof(*c_p));
            if (!(c_p)) 
            {
                printf("insufficient memory\n");
                return ENOMEM;
            }
            else
                *c = c_p;
        }
        (*c)->npts = npts;
        (*c)->fsr = fsr;
        (*c)->qres = qres;
        (*c)->noise_rms = noise_rms;
        (*c)->code_format = GnCodeFormatTwosComplement;

        return gn_success;
    }

    int gn_config_histz_nla(size_t npts, int qres, gn_config *c)
    {
        if (!(*c)) 
        {
            gn_config c_p;
            c_p = (gn_config)calloc(1, sizeof(*c_p));
            if (!(c_p)) 
            {
                printf("insufficient memory\n");
                return ENOMEM;
            }
            else
                *c = c_p;
        }
        (*c)->npts = npts;
        (*c)->qres = qres;
        (*c)->code_format = GnCodeFormatTwosComplement;
        (*c)->inla_fit = GnInlLineFitBestFit;

        return gn_success;
    }

    int gn_config_fftz(size_t npts, int qres, size_t fft_navg, size_t nfft, GnWindow win, gn_config *c)
    {
        if (npts != (fft_navg*nfft))
        {
            printf("ERROR: Number of samples points in the waveform has to equal FFT order times number of FFT averages\n");
            return gn_failure;
        }

        if (!((win == GnWindowBlackmanHarris) || (win == GnWindowHann) || (win == GnWindowNoWindow)))
        {
            printf("ERROR: Invalid selection of window function\n");
            return gn_failure;
        }        
        
        if (!(*c)) 
        {
            gn_config c_p;
            c_p = (gn_config)calloc(1, sizeof(*c_p));
            if (!(c_p)) 
            {
                printf("insufficient memory\n");
                return ENOMEM;
            }
            else
                *c = c_p;
        }
        
        (*c)->npts = npts;
        (*c)->qres = qres;
        (*c)->fft_navg = fft_navg;
        (*c)->nfft = nfft;
        (*c)->win = win;
        (*c)->code_format = GnCodeFormatTwosComplement;        
        
        return gn_success;
    }

    int gn_config_fa_auto(uint8_t ssb_width, gn_config *c)
    {
        int err_code;

        if (!(*c)) 
        {
            gn_config c_p;
            c_p = (gn_config)calloc(1, sizeof(*c_p));
            if (!(c_p)) 
            {
                printf("insufficient memory\n");
                return ENOMEM;
            }
            else
                *c = c_p;
        }

        if ((*c)->sample_rate <= 0) {
            printf("ERROR: Sample rate must be set before configuring Fourier analysis\n");
            return gn_failure;
        }

        (*c)->_all_fa_results_computed = false;

        (*c)->obj_key = (char *)calloc(3, sizeof(char));
        strcpy((*c)->obj_key, "fa");
        (*c)->comp_key = (char *)calloc(2, sizeof(char));
        strcpy((*c)->comp_key, "A");

        (*c)->ssb_fund = ssb_width;
        (*c)->ssb_rest = 0;
        (*c)->max_harm_order = 3;
        (*c)->axis_type = GnFreqAxisTypeDcCenter;

        // configure object key for Fourier analysis
        err_code = gn_fa_create((*c)->obj_key);

        // configure component key for Fourier analysis
        err_code += gn_fa_max_tone((*c)->obj_key, (*c)->comp_key, GnFACompTagSignal, (*c)->ssb_fund);

        // configure harmonic order for Fourier analysis
        err_code += gn_fa_hd((*c)->obj_key, (*c)->max_harm_order);

        // configure single-side bins for Fourier analysis
        err_code += gn_fa_ssb((*c)->obj_key, GnFASsbDefault, (*c)->ssb_rest);
        err_code += gn_fa_ssb((*c)->obj_key, GnFASsbDC, -1);
        err_code += gn_fa_ssb((*c)->obj_key, GnFASsbSignal, -1);
        err_code += gn_fa_ssb((*c)->obj_key, GnFASsbWO, -1);

        // configure sample-rate, data-rate, shift frequency, and converter offset
        err_code += gn_fa_fsample((*c)->obj_key, (*c)->sample_rate);
        err_code += gn_fa_fdata((*c)->obj_key, (*c)->sample_rate);
        err_code += gn_fa_fshift((*c)->obj_key, 0.0);
        err_code += gn_fa_conv_offset((*c)->obj_key, false);

        return (err_code);
    }


    int gn_config_fa(gn::real_t fixed_tone_freq, gn_config *c)
    {
        int err_code;

        if (!(*c))
        {
            gn_config c_p;
            c_p = (gn_config)calloc(1, sizeof(*c_p));
            if (!(c_p))
            {
                printf("insufficient memory\n");
                return ENOMEM;
            }
            else
                *c = c_p;
        }

        if ((*c)->sample_rate <= 0) {
            printf("ERROR: Sample rate must be set before configuring Fourier analysis\n");
            return gn_failure;
        }

        (*c)->_all_fa_results_computed = false;

        (*c)->obj_key = (char *)calloc(3, sizeof(char));
        strcpy((*c)->obj_key, "fa");
        (*c)->comp_key = (char *)calloc(2, sizeof(char));
        strcpy((*c)->comp_key, "A");

        (*c)->ssb_fund = 120;
        (*c)->ssb_rest = 0;
        (*c)->max_harm_order = 3;
        (*c)->axis_type = GnFreqAxisTypeDcCenter;

        // configure object key for Fourier analysis
        err_code = gn_fa_create((*c)->obj_key);

        // configure component key for Fourier analysis
        err_code += gn_fa_fixed_tone((*c)->obj_key, (*c)->comp_key, GnFACompTagSignal, fixed_tone_freq, (*c)->ssb_fund);

        // configure harmonic order for Fourier analysis
        err_code += gn_fa_hd((*c)->obj_key, (*c)->max_harm_order);

        // configure single-side bins for Fourier analysis
        err_code += gn_fa_ssb((*c)->obj_key, GnFASsbDefault, (*c)->ssb_rest);
        err_code += gn_fa_ssb((*c)->obj_key, GnFASsbDC, -1);
        err_code += gn_fa_ssb((*c)->obj_key, GnFASsbSignal, -1);
        err_code += gn_fa_ssb((*c)->obj_key, GnFASsbWO, -1);

        // configure sample-rate, data-rate, shift frequency, and converter offset
        err_code += gn_fa_fsample((*c)->obj_key, (*c)->sample_rate);
        err_code += gn_fa_fdata((*c)->obj_key, (*c)->sample_rate);
        err_code += gn_fa_fshift((*c)->obj_key, 0.0);
        err_code += gn_fa_conv_offset((*c)->obj_key, false);

        return (err_code);
    }

    // waveform generation
    int gn_gen_ramp(gn::real_t **out, gn_config *c)
    {
        int err_code;
        gn::real_t *awf = (gn::real_t *)calloc((*c)->npts, sizeof(gn::real_t));
        err_code = gn_ramp(awf, (*c)->npts, (*c)->ramp_start, (*c)->ramp_stop, (*c)->noise_rms);
        *out = awf;
        
        return err_code;
    }

    int gn_gen_real_tone(gn::real_t **out, gn_config *c)
    {
        int err_code = 0;
        gn::real_t *awf = (gn::real_t *)calloc((*c)->npts, sizeof(gn::real_t));

        for (size_t i = 0; i < (*c)->num_tones; i++) 
        {
            gn::real_t *tmp = (gn::real_t *)calloc((*c)->npts, sizeof(gn::real_t));
            if ((*c)->ttype == REAL_COSINE)
                err_code = gn_cos(tmp, (*c)->npts, (*c)->sample_rate, (*c)->tone_ampl[i], (*c)->tone_freq[i], (*c)->tone_phase[i], 0, 0);
            else if ((*c)->ttype == REAL_SINE)
                err_code = gn_sin(tmp, (*c)->npts, (*c)->sample_rate, (*c)->tone_ampl[i], (*c)->tone_freq[i], (*c)->tone_phase[i], 0, 0);
            if (!err_code) 
            {
                for (size_t j = 0; j < (*c)->npts; j++)
                    awf[j] = awf[j] + tmp[j];                        
            }
        }
        *out = awf;
        
        return err_code;
    }

    int gn_gen_complex_tone(gn::real_t **outi, gn::real_t **outq, gn_config *c)
    {
        int err_code = 0;
        gn::real_t *awfi = (gn::real_t *)calloc((*c)->npts, sizeof(gn::real_t));
        gn::real_t *awfq = (gn::real_t *)calloc((*c)->npts, sizeof(gn::real_t));

        for (size_t i = 0; i < (*c)->num_tones; i++) {
            gn::real_t *tmp = (gn::real_t *)calloc((*c)->npts, sizeof(gn::real_t));
            err_code = gn_cos(tmp, (*c)->npts, (*c)->sample_rate, (*c)->tone_ampl[i], (*c)->tone_freq[i], (*c)->tone_phase[i], 0, 0);
            if (!err_code) 
            {
                for (size_t j = 0; j < (*c)->npts; j++)
                    awfi[j] = awfi[j] + tmp[j];                        
            }
            tmp = (gn::real_t *)calloc((*c)->npts, sizeof(gn::real_t));
            err_code = gn_sin(tmp, (*c)->npts, (*c)->sample_rate, (*c)->tone_ampl[i], (*c)->tone_freq[i], (*c)->tone_phase[i], 0, 0);
            if (!err_code) 
            {
                for (size_t j = 0; j < (*c)->npts; j++)
                    awfq[j] = awfq[j] + tmp[j];                        
            }
        }
        *outi = awfi;
        *outq = awfq;
    
        return err_code;
    }

    // processing
    int gn_quantize(int32_t **out, const gn::real_t *in, gn_config *c)
    {
        int err_code;
        int32_t *qwf = (int32_t *)calloc((*c)->npts, sizeof(int32_t));

        err_code = gn_quantize32(qwf, (*c)->npts, in, (*c)->npts, (*c)->fsr, (*c)->qres, (*c)->noise_rms, (*c)->code_format);
        *out = qwf;
        
        return err_code;
    }

    int gn_fftz(gn::real_t **out, const int32_t *in_i, const int32_t *in_q, gn_config *c)
    {
        int err_code;
        gn::real_t *fft_of_in = (gn::real_t *)calloc(2*(*c)->nfft, sizeof(gn::real_t));

        err_code = gn_fft32(fft_of_in, 2*(*c)->nfft, in_i, (*c)->npts, in_q, (*c)->npts, (*c)->qres, (*c)->fft_navg, (*c)->nfft, (*c)->win, (*c)->code_format);
        *out = fft_of_in;

        return err_code;
    }

    int gn_histz(uint64_t **hist, size_t *hist_len, const int32_t *qwf, gn_config *c)
    {
        int err_code;
        uint64_t *out = NULL;        

        err_code = gn_code_density_size(&((*c)->_code_density_size), (*c)->qres, (*c)->code_format);
        out = (uint64_t *)calloc((*c)->_code_density_size, sizeof(uint64_t));
        err_code += gn_hist32(out, (*c)->_code_density_size, qwf, (*c)->npts, (*c)->qres, (*c)->code_format, false);
        *hist = out;
        *hist_len = (*c)->_code_density_size;        

        return err_code;
    }

    int gn_dnlz(double **dnl, size_t *dnl_len, const uint64_t *hist, gn_config *c)
    {
        int err_code;        
        double *out = NULL;        

        err_code = gn_code_density_size(&((*c)->_code_density_size), (*c)->qres, (*c)->code_format);
        out = (double *)calloc((*c)->_code_density_size, sizeof(double));
        err_code = gn_dnl(out, (*c)->_code_density_size, hist, (*c)->_code_density_size, (*c)->dnla_signal_type);
        *dnl = out;
        *dnl_len = (*c)->_code_density_size;        

        return err_code;
    }

    int gn_inlz(double **inl, size_t *inl_len, const double *dnl, gn_config *c)
    {
        int err_code;        
        double *out = NULL;        

        err_code = gn_code_density_size(&((*c)->_code_density_size), (*c)->qres, (*c)->code_format);
        out = (double *)calloc((*c)->_code_density_size, sizeof(double));
        err_code = gn_inl(out, (*c)->_code_density_size, dnl, (*c)->_code_density_size, (*c)->inla_fit);
        *inl = out;
        *inl_len = (*c)->_code_density_size;        

        return err_code;
    }

    // Waveform/Histogram/DNL/INL/Fourier Analysis
    int gn_get_wfa_results(char ***rkeys, gn::real_t **rvalues, size_t *results_size, const int32_t *qwf, gn_config *c)
    {
        int err_code = 0;

        // get results size
        err_code = gn_analysis_results_size(&((*c)->_wfa_results_size), GnAnalysisTypeWaveform);
        
        // allocate memory for result keys and values
        (*c)->_wfa_result_keys = (char **)calloc(((*c)->_wfa_results_size), sizeof(char*));
        (*c)->_wfa_result_values = (gn::real_t *)calloc(((*c)->_wfa_results_size), sizeof(gn::real_t));
        
        // get result key sizes
        (*c)->_wfa_result_key_sizes = (size_t *)calloc(((*c)->_wfa_results_size), sizeof(size_t));
        err_code += gn_analysis_results_key_sizes((*c)->_wfa_result_key_sizes, (*c)->_wfa_results_size, GnAnalysisTypeWaveform);
        
        // allocate memory for each result key
        for (size_t i = 0; i < (*c)->_wfa_results_size; ++i)
            (*c)->_wfa_result_keys[i] = (char *)calloc((*c)->_wfa_result_key_sizes[i], sizeof(char));
        
        // execute analysis
        err_code += gn_wf_analysis32((*c)->_wfa_result_keys, (*c)->_wfa_results_size, (*c)->_wfa_result_values, (*c)->_wfa_results_size, qwf, (*c)->npts);

        // copy keys
        *rkeys = (char **)calloc(((*c)->_wfa_results_size), sizeof(char*));
        for (size_t i = 0; i < (*c)->_wfa_results_size; ++i)
            (*rkeys)[i] = (char *)calloc((*c)->_wfa_result_key_sizes[i], sizeof(char));

        // copy values
        *rvalues = (gn::real_t *)calloc(((*c)->_wfa_results_size), sizeof(gn::real_t));
        for (size_t i = 0; i < (*c)->_wfa_results_size; ++i) 
        {
            strcpy((*rkeys)[i], (*c)->_wfa_result_keys[i]);
            (*rvalues)[i] = (*c)->_wfa_result_values[i];
        }
        *results_size = (*c)->_wfa_results_size;

        return(err_code);
    }

    int gn_get_ha_results(char ***rkeys, gn::real_t **rvalues, size_t *results_size, const uint64_t *hist, gn_config *c)
    {
        int err_code = 0;

        // get results size
        err_code = gn_analysis_results_size(&((*c)->_hist_results_size), GnAnalysisTypeHistogram);
        
        // allocate memory for result keys and values
        (*c)->_hist_result_keys = (char **)calloc(((*c)->_hist_results_size), sizeof(char*));
        (*c)->_hist_result_values = (gn::real_t *)calloc(((*c)->_hist_results_size), sizeof(gn::real_t));
        
        // get result key sizes
        (*c)->_hist_result_key_sizes = (size_t *)calloc(((*c)->_hist_results_size), sizeof(size_t));
        err_code += gn_analysis_results_key_sizes((*c)->_hist_result_key_sizes, (*c)->_hist_results_size, GnAnalysisTypeHistogram);
        
        // allocate memory for each result key
        for (size_t i = 0; i < (*c)->_hist_results_size; ++i)
            (*c)->_hist_result_keys[i] = (char *)calloc((*c)->_hist_result_key_sizes[i], sizeof(char));
        
        // execute analysis
        err_code += gn_hist_analysis((*c)->_hist_result_keys, (*c)->_hist_results_size, (*c)->_hist_result_values, (*c)->_hist_results_size, hist, (*c)->_code_density_size);

        // copy keys
        *rkeys = (char **)calloc(((*c)->_hist_results_size), sizeof(char*));
        for (size_t i = 0; i < (*c)->_hist_results_size; ++i)
            (*rkeys)[i] = (char *)calloc((*c)->_hist_result_key_sizes[i], sizeof(char));

        // copy values
        *rvalues = (gn::real_t *)calloc(((*c)->_hist_results_size), sizeof(gn::real_t));
        for (size_t i = 0; i < (*c)->_hist_results_size; ++i) 
        {
            strcpy((*rkeys)[i], (*c)->_hist_result_keys[i]);
            (*rvalues)[i] = (*c)->_hist_result_values[i];
        }
        *results_size = (*c)->_hist_results_size;

        return(err_code);
    }

    int gn_get_dnla_results(char ***rkeys, gn::real_t **rvalues, size_t *results_size, const gn::real_t *dnl, gn_config *c)
    {
        int err_code;

        // get results size
        err_code = gn_analysis_results_size(&((*c)->_dnl_results_size), GnAnalysisTypeDNL);
        
        // allocate memory for result keys and values
        (*c)->_dnl_result_keys = (char **)calloc(((*c)->_dnl_results_size), sizeof(char*));
        (*c)->_dnl_result_values = (gn::real_t *)calloc(((*c)->_dnl_results_size), sizeof(gn::real_t));
        
        // get result key sizes
        (*c)->_dnl_result_key_sizes = (size_t *)calloc(((*c)->_dnl_results_size), sizeof(size_t));
        err_code += gn_analysis_results_key_sizes((*c)->_dnl_result_key_sizes, (*c)->_dnl_results_size, GnAnalysisTypeDNL);
        
        // allocate memory for each result key
        for (size_t i = 0; i < (*c)->_dnl_results_size; ++i)
            (*c)->_dnl_result_keys[i] = (char *)calloc((*c)->_dnl_result_key_sizes[i], sizeof(char));
        
        // execute analysis
        err_code += gn_dnl_analysis((*c)->_dnl_result_keys, (*c)->_dnl_results_size, (*c)->_dnl_result_values, (*c)->_dnl_results_size, dnl, (*c)->_code_density_size);

        // copy keys
        *rkeys = (char **)calloc(((*c)->_dnl_results_size), sizeof(char*));
        for (size_t i = 0; i < (*c)->_dnl_results_size; ++i)
            (*rkeys)[i] = (char *)calloc((*c)->_dnl_result_key_sizes[i], sizeof(char));

        // copy values
        *rvalues = (gn::real_t *)calloc(((*c)->_dnl_results_size), sizeof(gn::real_t));
        for (size_t i = 0; i < (*c)->_dnl_results_size; ++i) 
        {
            strcpy((*rkeys)[i], (*c)->_dnl_result_keys[i]);
            (*rvalues)[i] = (*c)->_dnl_result_values[i];
        }
        *results_size = (*c)->_dnl_results_size;

        return(err_code);
    }

    int gn_get_inla_results(char ***rkeys, gn::real_t **rvalues, size_t *results_size, const gn::real_t *inl, gn_config *c)
    {
        int err_code;

        // get results size
        err_code = gn_analysis_results_size(&((*c)->_inl_results_size), GnAnalysisTypeINL);
        
        // allocate memory for result keys and values
        (*c)->_inl_result_keys = (char **)calloc(((*c)->_inl_results_size), sizeof(char*));
        (*c)->_inl_result_values = (gn::real_t *)calloc(((*c)->_inl_results_size), sizeof(gn::real_t));
        
        // get result key sizes
        (*c)->_inl_result_key_sizes = (size_t *)calloc(((*c)->_inl_results_size), sizeof(size_t));
        err_code += gn_analysis_results_key_sizes((*c)->_inl_result_key_sizes, (*c)->_inl_results_size, GnAnalysisTypeINL);
        
        // allocate memory for each result key
        for (size_t i = 0; i < (*c)->_inl_results_size; ++i)
            (*c)->_inl_result_keys[i] = (char *)calloc((*c)->_inl_result_key_sizes[i], sizeof(char));
        
        // execute analysis
        err_code += gn_inl_analysis((*c)->_inl_result_keys, (*c)->_inl_results_size, (*c)->_inl_result_values, (*c)->_inl_results_size, inl, (*c)->_code_density_size);

        // copy keys
        *rkeys = (char **)calloc(((*c)->_inl_results_size), sizeof(char*));
        for (size_t i = 0; i < (*c)->_inl_results_size; ++i)
            (*rkeys)[i] = (char *)calloc((*c)->_inl_result_key_sizes[i], sizeof(char));

        // copy values
        *rvalues = (gn::real_t *)calloc(((*c)->_inl_results_size), sizeof(gn::real_t));
        for (size_t i = 0; i < (*c)->_inl_results_size; ++i) 
        {
            strcpy((*rkeys)[i], (*c)->_inl_result_keys[i]);
            (*rvalues)[i] = (*c)->_inl_result_values[i];
        }
        *results_size = (*c)->_inl_results_size;

        return(err_code);
    }

    int gn_get_fa_single_result(gn::real_t *rvalue, const char *metric_name, gn::real_t *fft_ilv, gn_config *c)
    {
        int err_code;
        size_t i;
        bool metric_found = false;

        // compute all results 
        err_code = gn_get_fa_results(NULL, NULL, NULL, fft_ilv, &(*c));
        for (i = 0; i < (*c)->_fa_results_size; i++) 
        {
            if (!strcmp(metric_name, (*c)->_fa_result_keys[i])) 
            {
                metric_found = true;
                break;
            }            
        }
        if (!metric_found)
        {
            printf("ERROR: Invalid selection of metric\n");
            return gn_failure;
        }
        *rvalue = (*c)->_fa_result_values[i];

        return err_code;
    }

    int gn_get_fa_results(char ***rkeys, gn::real_t **rvalues, size_t *results_size, gn::real_t *fft_ilv, gn_config *c)
    {
        int err_code = 0;

        if (!((*c)->_all_fa_results_computed))
        {
            // get results size
            err_code = gn_fft_analysis_results_size(&((*c)->_fa_results_size), (*c)->obj_key, 2*(*c)->nfft, (*c)->nfft);
            
            // allocate memory for result keys and values
            (*c)->_fa_result_keys = (char **)calloc(((*c)->_fa_results_size), sizeof(char*));
            (*c)->_fa_result_values = (gn::real_t *)calloc(((*c)->_fa_results_size), sizeof(gn::real_t));
            
            // get result key sizes
            (*c)->_fa_result_key_sizes = (size_t *)calloc(((*c)->_fa_results_size), sizeof(size_t));
            err_code += gn_fft_analysis_results_key_sizes((*c)->_fa_result_key_sizes, (*c)->_fa_results_size, (*c)->obj_key, 2*(*c)->nfft, (*c)->nfft);

            // allocate memory for each result key
            for (size_t i = 0; i < (*c)->_fa_results_size; ++i)
                (*c)->_fa_result_keys[i] = (char *)calloc((*c)->_fa_result_key_sizes[i], sizeof(char));
            
            // execute analysis
            err_code += gn_fft_analysis((*c)->_fa_result_keys, (*c)->_fa_results_size, (*c)->_fa_result_values, (*c)->_fa_results_size, (*c)->obj_key, fft_ilv, 2*(*c)->nfft, (*c)->nfft, (*c)->axis_type);
            
            if (err_code == 0)
                (*c)->_all_fa_results_computed = true;
        }

        if (rkeys && rvalues && results_size) 
        {
            // copy keys
            *rkeys = (char **)calloc(((*c)->_fa_results_size), sizeof(char*));
            for (size_t i = 0; i < (*c)->_fa_results_size; ++i)
                (*rkeys)[i] = (char *)calloc((*c)->_fa_result_key_sizes[i], sizeof(char));

            // copy values
            *rvalues = (gn::real_t *)calloc(((*c)->_fa_results_size), sizeof(gn::real_t));
            for (size_t i = 0; i < (*c)->_fa_results_size; ++i) 
            {
                strcpy((*rkeys)[i], (*c)->_fa_result_keys[i]);
                (*rvalues)[i] = (*c)->_fa_result_values[i];
            }
            *results_size = (*c)->_fa_results_size;
        }
        
        return (err_code);
    }
}