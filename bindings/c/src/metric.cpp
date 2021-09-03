#include "cgenalyzer.h"
#include "cgenalyzer_private.h"
#ifdef __APPLE__
#include <mach/error.h>
#else
#include <error.h>
#endif

extern "C" {
double metric(config c, void* input, const char* m_name, unsigned int* err_code)
{
    char m_names[16][10] = { "ABN", "FSNR", "nad", "noise",
        "NSD", "SFDR", "SINAD", "SNR",
        "sum_hd", "sum_ilgt", "sum_ilos", "sum_imd", "td", "thd", "til", "DNL" };
    double r = -1e-6;
    bool found_metric = false;
    fft_analysis_wrapper* cfftobj;
    size_t fft_size;
    double *fft_cplx_re, *fft_cplx_im;

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
            rfft(c, qwf, &fft_cplx_re, &fft_cplx_im, &fft_size);
        else if (c->wf_type == COMPLEX_EXP) {
            int32_t *qwf_i, *qwf_q;
            qwf_i = (int32_t*)calloc(c->nfft, sizeof(int32_t));
            qwf_q = (int32_t*)calloc(c->nfft, sizeof(int32_t));
            for (int i = 0; i < 2 * c->nfft; i += 2) {
                qwf_i[i / 2] = qwf[i];
                qwf_q[i / 2] = qwf[i + 1];
            }

            fft(c, qwf_i, qwf_q, &fft_cplx_re, &fft_cplx_im, &fft_size);
        }
    } else if (c->md == FREQ) {
        double* temp_fft_data = (double*)input;
        if (c->wf_type == COMPLEX_EXP) {
            fft_cplx_re = (double*)calloc(c->nfft, sizeof(double));
            fft_cplx_im = (double*)calloc(c->nfft, sizeof(double));
            for (int n = 0; n < 2 * c->nfft; n += 2) {
                fft_cplx_re[n / 2] = temp_fft_data[n];
                fft_cplx_im[n / 2] = temp_fft_data[n + 1];
            }
        } else {
            fft_cplx_re = (double*)calloc(c->nfft / 2, sizeof(double));
            fft_cplx_im = (double*)calloc(c->nfft / 2, sizeof(double));
            for (int n = 0; n < c->nfft; n += 2) {
                fft_cplx_re[n / 2] = temp_fft_data[n];
                fft_cplx_im[n / 2] = temp_fft_data[n + 1];
            }
        }
    }

    // compute metrics
    cfftobj = create_fft_analysis_wrapper();
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
            r = compute_metric(c, cfftobj, fft_cplx_re, fft_cplx_im, m_name);
            found_metric = true;
            break;
        }
    }

    // free memory
    destroy_fft_analysis_wrapper(cfftobj);
    free(fft_cplx_re);
    free(fft_cplx_im);
    free(tone_label);

    if (!found_metric)
        *err_code = EINVAL;
    else
        *err_code = 0;

    return r;
}
}
