#include "cgenalyzer.h"
#include "cgenalyzer_private.h"
#ifdef __APPLE__
#include <mach/error.h>
#elif _WIN32
#include <errno.h>
#else
#include <error.h>
#endif

extern "C" {
double gn_metric(gn_config c, const void* input, const char* m_name, double** fft_re, double** fft_im, size_t *fft_len, unsigned int* err_code)
{
    char m_names[16][10] = { "ABN", "FSNR", "nad", "noise",
        "NSD", "SFDR", "SINAD", "SNR",
        "sum_hd", "sum_ilgt", "sum_ilos", "sum_imd", "td", "thd", "til", "DNL" };
    double r = -1e-6;
    bool found_metric = false;
    fft_analysis_wrapper* cfftobj;
    size_t fft_size;
    double *fft_re_tmp, *fft_im_tmp;
    double *tmp;

    char** tone_label;
    tone_label = (char**)malloc(c->n_tones * sizeof(char*));
    for (int i = 0; i < c->n_tones; i++) {
        tone_label[i] = (char*)malloc(2 * sizeof(char));
        tone_label[i][0] = i + 'a';
        tone_label[i][1] = '\0';
    }

    if (c->md == TIME) {
        int32_t* qwf = (int32_t*)input;
        // compute FFT
        if ((c->wf_type == REAL_COSINE) || (c->wf_type == REAL_SINE))
            gn_rfft(c, qwf, &fft_re_tmp, &fft_im_tmp, &fft_size);
        else if (c->wf_type == COMPLEX_EXP) {
            int32_t *qwf_i, *qwf_q;
            qwf_i = (int32_t*)calloc(c->nfft*c->navg, sizeof(int32_t));
            qwf_q = (int32_t*)calloc(c->nfft*c->navg, sizeof(int32_t));
            for (int i = 0; i < 2 * c->nfft*c->navg; i += 2) {
                qwf_i[i / 2] = qwf[i];
                qwf_q[i / 2] = qwf[i + 1];
            }
            gn_fft(c, qwf_i, qwf_q, &fft_re_tmp, &fft_im_tmp, &fft_size);
        }
    } else if (c->md == FREQ) {
        double* temp_fft_data = (double*)input;
        if (c->wf_type == COMPLEX_EXP) {
            fft_re_tmp = (double*)calloc(c->nfft, sizeof(double));
            fft_im_tmp = (double*)calloc(c->nfft, sizeof(double));
            for (int n = 0; n < 2 * c->nfft; n += 2) {
                fft_re_tmp[n / 2] = temp_fft_data[n];
                fft_im_tmp[n / 2] = temp_fft_data[n + 1];
            }
        } else {
            fft_re_tmp = (double*)calloc(c->nfft / 2, sizeof(double));
            fft_im_tmp = (double*)calloc(c->nfft / 2, sizeof(double));
            for (int n = 0; n < c->nfft; n += 2) {
                fft_re_tmp[n / 2] = temp_fft_data[n];
                fft_im_tmp[n / 2] = temp_fft_data[n + 1];
            }
        }
    }
    *fft_re = fft_re_tmp;
    *fft_im = fft_im_tmp;
    *fft_len = fft_size;

    // compute metrics
    cfftobj = gn_create_fft_analysis_wrapper();
    for (int i = 0; i < c->n_tones; i++)
        add_max_tone(cfftobj, tone_label[i]);
    if (c->update_fsample)
        set_fsample(cfftobj, c->fs);
    if (c->update_fdata)
        set_fdata(cfftobj, c->fdata);
    if (c->update_fshift)
        set_fshift(cfftobj, c->fshift);

    for (int i = 0; i < 16; i++) {
        if (strcmp(m_name, m_names[i]) == 0) {
            r = gn_compute_metric(c, cfftobj, fft_re_tmp, fft_im_tmp, m_name);
            found_metric = true;
            break;
        }
    }

    // free memory
    gn_destroy_fft_analysis_wrapper(cfftobj);
    free(tone_label);

    if (!found_metric)
        *err_code = EINVAL;
    else
        *err_code = 0;

    return r;
}
}
