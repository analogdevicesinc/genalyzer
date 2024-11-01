#include <cgenalyzer_simplified_beta.h>
#include <cgenalyzer.h>

#include <stdbool.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ERROR_BUFFER_SIZE 256

#define HANDLE_ERROR(x) \
    do { \
        if (0 != x) { \
            gn_error_string(error_buffer, ERROR_BUFFER_SIZE); \
            printf("%s\n", error_buffer); \
            goto cleanup; \
        } \
    } while (0)

int main(int argc, char* argv[])
{
    bool save_data = false;
    if (1 < argc) {
        save_data = !((bool) strcmp(argv[1], "--save-data"));
    }

    /*
     * Setup
     */
    gn_set_string_termination(true);
    char error_buffer[ERROR_BUFFER_SIZE];
    bool error_occurred = true;
    int result = 0;

    size_t navg = 2;                            // FFT averaging
    size_t nfft = 1024 * 256;                   // FFT size
    double fs = 1e9;                            // sample rate
    double fsr = 2.0;                           // full-scale range
    double ampl_dbfs = -1.0;                    // input amplitude (dBFS)
    double freq = 10e6;                         // input frequency (Hz)
    double phase = 0.110;                       // input phase (rad)
    double td = 0.0;                            // input delay (s)
    double tj = 0.0;                            // RMS aperature uncertainty (s)
    double poco[] = {0.0, 1.0, 0.0, 0.003};     // distortion polynomial coefficients
    int qres = 12;                              // quantizer resolution
    double qnoise_dbfs = -63.0;                 // quantizer noise (dBFS)
    GnCodeFormat code_fmt = GnCodeFormatTwosComplement;
    GnDnlSignal sig_type = GnDnlSignalTone;
    GnInlLineFit inl_fit = GnInlLineFitBestFit;
    GnRfftScale rfft_scale = GnRfftScaleDbfsSin;
    GnWindow window = GnWindowNoWindow;

    size_t npts = navg * nfft;
    double ampl = (fsr / 2) * pow(10.0, ampl_dbfs / 20.0);
    double qnoise = pow(10.0, qnoise_dbfs / 20.0);
    int ssb_fund = 4;
    int ssb_rest = 3;

    /*
     * Pointers for allocated memory
     */
    double*   awf             = NULL;       // analog waveform
    int16_t*  qwf             = NULL;       // quantized waveform
    uint64_t* hist            = NULL;       // histogram data
    double*   dnl             = NULL;       // DNL data
    double*   inl             = NULL;       // INL data
    double*   code_axis       = NULL;       // code density axis
    double*   fft_cplx        = NULL;       // FFT complex data
    double*   fft_db          = NULL;       // FFT magnitude data
    double*   freq_axis       = NULL;       // frequency axis
    char*     fa_preview      = NULL;       // FFT configuration preview
    char**    wf_rkeys        = NULL;       // waveform analysis keys
    double*   wf_rvalues      = NULL;       // waveform analysis values
    size_t*   wf_rkey_sizes   = NULL;       // waveform analysis key sizes
    char**    hist_rkeys      = NULL;       // histogram analysis keys
    double*   hist_rvalues    = NULL;       // histogram analysis values
    size_t*   hist_rkey_sizes = NULL;       // histogram analysis key sizes
    char**    dnl_rkeys       = NULL;       // DNL analysis keys
    double*   dnl_rvalues     = NULL;       // DNL analysis values
    size_t*   dnl_rkey_sizes  = NULL;       // DNL analysis key sizes
    char**    inl_rkeys       = NULL;       // INL analysis keys
    double*   inl_rvalues     = NULL;       // INL analysis values
    size_t*   inl_rkey_sizes  = NULL;       // INL analysis key sizes
    char**    fft_rkeys       = NULL;       // FFT analysis keys
    double*   fft_rvalues     = NULL;       // FFT analysis values
    size_t*   fft_rkey_sizes  = NULL;       // FFT analysis key sizes

    size_t wf_results_size = 0;
    size_t hist_results_size = 0;
    size_t dnl_results_size = 0;
    size_t inl_results_size = 0;
    size_t fft_results_size = 0;

    /*
     * Allocate memory
     */
    size_t cd_size = 0; // cd = code density
    size_t fft_cplx_size = 0;
    result += gn_code_density_size(&cd_size, qres, code_fmt);
    result += gn_rfft_size(&fft_cplx_size, npts, navg, nfft);
    HANDLE_ERROR(result);
    awf      = malloc(npts          * sizeof(double));
    qwf      = malloc(npts          * sizeof(int16_t));
    hist     = malloc(cd_size       * sizeof(uint64_t));
    dnl      = malloc(cd_size       * sizeof(double));
    inl      = malloc(cd_size       * sizeof(double));
    fft_cplx = malloc(fft_cplx_size * sizeof(double));

    /*
     * Signal generation and processing
     */
    if (GnWindowNoWindow == window) {
        result += gn_coherent(&freq, nfft, fs, freq);
        HANDLE_ERROR(result);
        ssb_fund = 0;
        ssb_rest = 0;
    }
    result += gn_cos(awf, npts, fs, ampl, freq, phase, td, tj);
    result += gn_polyval(awf, npts, awf, npts, poco, (sizeof poco) / sizeof(double));
    result += gn_quantize16(qwf, npts, awf, npts, fsr, qres, qnoise, code_fmt);
    result += gn_hist16(hist, cd_size, qwf, npts, qres, code_fmt, false);
    result += gn_dnl(dnl, cd_size, hist, cd_size, sig_type);
    result += gn_inl(inl, cd_size, dnl, cd_size, inl_fit);
    result += gn_rfft16(fft_cplx, fft_cplx_size, qwf, npts, qres, navg, nfft, window, code_fmt, rfft_scale);
    HANDLE_ERROR(result);

    /*
     * Fourier analysis configuration
     */
    char* key = "fa";
    result += gn_fa_create(key);
    result += gn_fa_fixed_tone(key, "A", GnFACompTagSignal, freq, ssb_fund);
    result += gn_fa_hd(key, 3);
    result += gn_fa_ssb(key, GnFASsbDefault, ssb_rest);
    result += gn_fa_ssb(key, GnFASsbDC, -1);
    result += gn_fa_ssb(key, GnFASsbSignal, -1);
    result += gn_fa_ssb(key, GnFASsbWO, -1);
    result += gn_fa_fsample(key, fs);
    HANDLE_ERROR(result);
    size_t fa_preview_size = 0;
    result += gn_fa_preview_size(&fa_preview_size, key, false);
    fa_preview = malloc(fa_preview_size);
    result += gn_fa_preview(fa_preview, fa_preview_size, key, false);
    HANDLE_ERROR(result);
    printf("%s\n", fa_preview);

    // To do an analysis:
    // 1. Get results size
    // 2. Allocate memory for result keys and values
    // 3. Get result key sizes
    // 4. Allocate memory for each result key
    // 5. Execute analysis

    /*
     * Waveform analysis
     */
    // 1.
    result += gn_analysis_results_size(&wf_results_size, GnAnalysisTypeWaveform);
    // 2.
    wf_rkeys      = malloc(wf_results_size * sizeof(char*));
    wf_rvalues    = malloc(wf_results_size * sizeof(double));
    // 3.
    wf_rkey_sizes = malloc(wf_results_size * sizeof(size_t));
    result += gn_analysis_results_key_sizes(wf_rkey_sizes, wf_results_size, GnAnalysisTypeWaveform);
    HANDLE_ERROR(result);
    // 4.
    for (size_t i = 0; i < wf_results_size; ++i) {
        wf_rkeys[i] = malloc(wf_rkey_sizes[i]);
    }
    // 5.
    result += gn_wf_analysis16(wf_rkeys, wf_results_size, wf_rvalues, wf_results_size, qwf, npts);
    HANDLE_ERROR(result);

    /*
     * Histogram analysis
     */
    result += gn_analysis_results_size(&hist_results_size, GnAnalysisTypeHistogram);
    hist_rkeys      = malloc(hist_results_size * sizeof(char*));
    hist_rvalues    = malloc(hist_results_size * sizeof(double));
    hist_rkey_sizes = malloc(hist_results_size * sizeof(size_t));
    result += gn_analysis_results_key_sizes(hist_rkey_sizes, hist_results_size, GnAnalysisTypeHistogram);
    HANDLE_ERROR(result);
    for (size_t i = 0; i < hist_results_size; ++i) {
        hist_rkeys[i] = malloc(hist_rkey_sizes[i]);
    }
    result += gn_hist_analysis(hist_rkeys, hist_results_size, hist_rvalues, hist_results_size, hist, cd_size);
    HANDLE_ERROR(result);

    /*
     * DNL analysis
     */
    result += gn_analysis_results_size(&dnl_results_size, GnAnalysisTypeDNL);
    dnl_rkeys      = malloc(dnl_results_size * sizeof(char*));
    dnl_rvalues    = malloc(dnl_results_size * sizeof(double));
    dnl_rkey_sizes = malloc(dnl_results_size * sizeof(size_t));
    result += gn_analysis_results_key_sizes(dnl_rkey_sizes, dnl_results_size, GnAnalysisTypeDNL);
    HANDLE_ERROR(result);
    for (size_t i = 0; i < dnl_results_size; ++i) {
        dnl_rkeys[i] = malloc(dnl_rkey_sizes[i]);
    }
    result += gn_dnl_analysis(dnl_rkeys, dnl_results_size, dnl_rvalues, dnl_results_size, dnl, cd_size);
    HANDLE_ERROR(result);

    /*
     * INL analysis
     */
    result += gn_analysis_results_size(&inl_results_size, GnAnalysisTypeINL);
    inl_rkeys      = malloc(inl_results_size * sizeof(char*));
    inl_rvalues    = malloc(inl_results_size * sizeof(double));
    inl_rkey_sizes = malloc(inl_results_size * sizeof(size_t));
    result += gn_analysis_results_key_sizes(inl_rkey_sizes, inl_results_size, GnAnalysisTypeINL);
    HANDLE_ERROR(result);
    for (size_t i = 0; i < inl_results_size; ++i) {
        inl_rkeys[i] = malloc(inl_rkey_sizes[i]);
    }
    result += gn_inl_analysis(inl_rkeys, inl_results_size, inl_rvalues, inl_results_size, inl, cd_size);
    HANDLE_ERROR(result);

    /*
     * Fourier analysis
     */
    result += gn_fft_analysis_results_size(&fft_results_size, key, fft_cplx_size, nfft);
    HANDLE_ERROR(result);
    fft_rkeys      = malloc(fft_results_size * sizeof(char*));
    fft_rvalues    = malloc(fft_results_size * sizeof(double));
    fft_rkey_sizes = malloc(fft_results_size * sizeof(size_t));
    result += gn_fft_analysis_results_key_sizes(fft_rkey_sizes, fft_results_size, key, fft_cplx_size, nfft);
    HANDLE_ERROR(result);
    for (size_t i = 0; i < fft_results_size; ++i) {
        fft_rkeys[i] = malloc(fft_rkey_sizes[i]);
    }
    result += gn_fft_analysis(fft_rkeys, fft_results_size, fft_rvalues, fft_results_size,
        key, fft_cplx, fft_cplx_size, nfft, GnFreqAxisTypeReal);
    HANDLE_ERROR(result);

    /*
     * Print results
     */
    printf("\nWaveform Analysis Results:\n");
    for (size_t i = 0; i < wf_results_size; ++i) {
        printf("%4zu%16s%16.6f\n", i, wf_rkeys[i], wf_rvalues[i]);
    }
    printf("\nHistogram Analysis Results:\n");
    for (size_t i = 0; i < hist_results_size; ++i) {
        printf("%4zu%16s%16.6f\n", i, hist_rkeys[i], hist_rvalues[i]);
    }
    printf("\nDNL Analysis Results:\n");
    for (size_t i = 0; i < dnl_results_size; ++i) {
        printf("%4zu%16s%16.6f\n", i, dnl_rkeys[i], dnl_rvalues[i]);
    }
    printf("\nINL Analysis Results:\n");
    for (size_t i = 0; i < inl_results_size; ++i) {
        printf("%4zu%16s%16.6f\n", i, inl_rkeys[i], inl_rvalues[i]);
    }
    printf("\nFourier Analysis Results:\n");
    for (size_t i = 0; i < fft_results_size; ++i) {
        printf("%4zu%20s%20.6f\n", i, fft_rkeys[i], fft_rvalues[i]);
    }

    /*
     * Save data
     */
    if (save_data) {
        const char* fn = "qwf.txt";
        FILE* fp = fopen(fn, "w");
        if (fp) {
            for (size_t i = 0; i < npts; ++i) {
                fprintf(fp, "%d\n", qwf[i]);
            }
            fclose(fp);
            printf("\nWrote quantized waveform data to %s\n", fn);
        }
        fn = "hist.txt";
        fp = fopen(fn, "w");
        if (fp) {
            for (size_t i = 0; i < cd_size; ++i) {
                fprintf(fp, "%zu\n", hist[i]);
            }
            fclose(fp);
            printf("Wrote histogram data to %s\n", fn);
        }
        fn = "dnl.txt";
        fp = fopen(fn, "w");
        if (fp) {
            for (size_t i = 0; i < cd_size; ++i) {
                fprintf(fp, "%.9f\n", dnl[i]);
            }
            fclose(fp);
            printf("Wrote DNL data to %s\n", fn);
        }
        fn = "inl.txt";
        fp = fopen(fn, "w");
        if (fp) {
            for (size_t i = 0; i < cd_size; ++i) {
                fprintf(fp, "%.9f\n", inl[i]);
            }
            fclose(fp);
            printf("Wrote INL data to %s\n", fn);
        }
        fn = "code_axis.txt";
        fp = fopen(fn, "w");
        if (fp) {
            code_axis = malloc(cd_size * sizeof(double));
            result += gn_code_axis(code_axis, cd_size, qres, code_fmt);
            for (size_t i = 0; i < cd_size; ++i) {
                fprintf(fp, "%.0f\n", code_axis[i]);
            }
            fclose(fp);
            printf("Wrote code density axis data to %s\n", fn);
        }
        fn = "rfft.txt";
        fp = fopen(fn, "w");
        if (fp) {
            fft_db = malloc((fft_cplx_size / 2) * sizeof(double));
            result += gn_db(fft_db, (fft_cplx_size / 2), fft_cplx, fft_cplx_size);
            for (size_t i = 0; i < (fft_cplx_size / 2); ++i) {
                fprintf(fp, "%.9f\n", fft_db[i]);
            }
            fclose(fp);
            printf("Wrote FFT magnitude data to %s\n", fn);
        }
        fn = "rfft_axis.txt";
        fp = fopen(fn, "w");
        if (fp) {
            freq_axis = malloc((fft_cplx_size / 2) * sizeof(double));
            result = gn_freq_axis(freq_axis, (fft_cplx_size / 2), nfft, GnFreqAxisTypeReal, fs, GnFreqAxisFormatFreq);
            for (size_t i = 0; i < (fft_cplx_size / 2); ++i) {
                fprintf(fp, "%.6f\n", freq_axis[i]);
            }
            fclose(fp);
            printf("Wrote FFT x-axis data to %s\n", fn);
        }
    }

    error_occurred = false;
cleanup:
    if (error_occurred) {
        printf("\nError occurred, freeing memory\n");
    } else {
        printf("\nFreeing memory\n");
    }
    if (fft_rkeys) {
        for (size_t i = 0; i < fft_results_size; ++i) {
            free(fft_rkeys[i]);
        }
    }
    free(fft_rkey_sizes);
    free(fft_rvalues);
    free(fft_rkeys);
    if (inl_rkeys) {
        for (size_t i = 0; i < inl_results_size; ++i) {
            free(inl_rkeys[i]);
        }
    }
    free(inl_rkey_sizes);
    free(inl_rvalues);
    free(inl_rkeys);
    if (dnl_rkeys) {
        for (size_t i = 0; i < dnl_results_size; ++i) {
            free(dnl_rkeys[i]);
        }
    }
    free(dnl_rkey_sizes);
    free(dnl_rvalues);
    free(dnl_rkeys);
    if (hist_rkeys) {
        for (size_t i = 0; i < hist_results_size; ++i) {
            free(hist_rkeys[i]);
        }
    }
    free(hist_rkey_sizes);
    free(hist_rvalues);
    free(hist_rkeys);
    if (wf_rkeys) {
        for (size_t i = 0; i < wf_results_size; ++i) {
            free(wf_rkeys[i]);
        }
    }
    free(wf_rkey_sizes);
    free(wf_rvalues);
    free(wf_rkeys);
    free(fa_preview);
    free(freq_axis);
    free(fft_db);
    free(fft_cplx);
    free(code_axis);
    free(inl);
    free(dnl);
    free(hist);
    free(qwf);
    free(awf);

    return 0;
}