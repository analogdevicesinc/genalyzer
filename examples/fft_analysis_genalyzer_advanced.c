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
    size_t nfft = 1024 * 16;                    // FFT size
    double fs = 1e9;                            // sample rate
    double fdata = fs / 1;                      // data rate
    double fshift = 0e6;                        // shift frequency
    double fsr = 2.0;                           // full-scale range
    double ampl_dbfs = -1.0;                    // input amplitude (dBFS)
    double freq = 70e6;                         // input frequency (Hz)
    double phase = 0.110;                       // input phase (rad)
    double td = 0.0;                            // input delay (s)
    double tj = 0.0;                            // RMS aperature uncertainty (s)
    double qpe = (atan(1.0) * 2) * 1e-5;        // quadrature phase error (pi/2 * x)
    double poco[] = {0.0, 1.0, 0.0, 0.003};     // distortion polynomial coefficients
    int qres = 12;                              // quantizer resolution
    double qnoise_dbfs = -63.0;                 // quantizer noise (dBFS)
    GnCodeFormat code_fmt = GnCodeFormatTwosComplement;
    GnFreqAxisFormat axis_fmt = GnFreqAxisFormatFreq;
    GnFreqAxisType axis_type = GnFreqAxisTypeDcCenter;
    GnWindow window = GnWindowNoWindow;

    int dsr = (int) (fs / fdata);
    size_t npts = navg * nfft * dsr;
    double ampl = (fsr / 2) * pow(10.0, ampl_dbfs / 20.0);
    double qnoise = pow(10.0, qnoise_dbfs / 20.0);
    int ssb_fund = 4;
    int ssb_rest = 3;

    /*
     * Pointers for allocated memory
     */
    double*  awfi               = NULL;     // in-phase analog waveform
    double*  awfq               = NULL;     // quadrature analog waveform
    int16_t* qwfi               = NULL;     // in-phase quantized waveform
    int16_t* qwfq               = NULL;     // quadrature quantized waveform
    int16_t* xwf                = NULL;     // frequency-shifted waveform
    int16_t* ywf                = NULL;     // downsampled waveform
    double*  fft_cplx           = NULL;     // FFT complex data
    double*  fft_db             = NULL;     // FFT magnitude data
    double*  freq_axis          = NULL;     // frequency axis
    char*    fa_preview         = NULL;     // FFT analysis configuration preview
    char*    fa_carrier         = NULL;     // FFT analysis carrier key
    char*    fa_maxspur         = NULL;     // FFT analysis maxspur key
    char**   fft_rkeys          = NULL;     // FFT analysis keys
    double*  fft_rvalues        = NULL;     // FFT analysis values
    size_t*  fft_rkey_sizes     = NULL;     // FFT analysis key sizes
    double*  fft_select_rvalues = NULL;     // FFT analysis select values

    size_t fft_results_size = 0;

    /*
     * Allocate memory
     */
    size_t xwf_size = npts * 2;
    size_t ywf_size = 0;
    size_t fft_cplx_size = nfft * 2;
    result += gn_downsample_size(&ywf_size, xwf_size, dsr, true);
    HANDLE_ERROR(result);
    awfi      = malloc(npts          * sizeof(double));
    awfq      = malloc(npts          * sizeof(double));
    qwfi      = malloc(npts          * sizeof(int16_t));
    qwfq      = malloc(npts          * sizeof(int16_t));
    xwf       = malloc(xwf_size      * sizeof(int16_t));
    ywf       = malloc(ywf_size      * sizeof(int16_t));
    fft_cplx  = malloc(fft_cplx_size * sizeof(double));

    /*
     * Signal generation and processing
     */
    if (GnWindowNoWindow == window) {
        result += gn_coherent(&freq, nfft, fdata, freq);
        HANDLE_ERROR(result);
        double fbin = fdata / nfft;
        fshift = round(fshift / fbin) * fbin;
        ssb_fund = 0;
        ssb_rest = 0;
    }
    result += gn_cos(awfi, npts, fs, ampl, freq, phase, td, tj);
    result += gn_sin(awfq, npts, fs, ampl, freq, phase + qpe, td, tj);
    result += gn_polyval(awfi, npts, awfi, npts, poco, (sizeof poco) / sizeof(double));
    result += gn_polyval(awfq, npts, awfq, npts, poco, (sizeof poco) / sizeof(double));
    result += gn_quantize16(qwfi, npts, awfi, npts, fsr, qres, qnoise, code_fmt);
    result += gn_quantize16(qwfq, npts, awfq, npts, fsr, qres, qnoise, code_fmt);
    result += gn_fshift16(xwf, xwf_size, qwfi, npts, qwfq, npts, qres, fs, fshift, code_fmt);
    result += gn_downsample16(ywf, ywf_size, xwf, xwf_size, dsr, true);
    result += gn_fft16(fft_cplx, fft_cplx_size, ywf, ywf_size, NULL, 0, qres, navg, nfft, window, code_fmt);
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
    result += gn_fa_fdata(key, fdata);
    result += gn_fa_fshift(key, fshift);
    result += gn_fa_conv_offset(key, 0.0 != fshift);
    HANDLE_ERROR(result);
    size_t fa_preview_size = 0;
    result += gn_fa_preview_size(&fa_preview_size, key, true);
    fa_preview = malloc(fa_preview_size);
    result += gn_fa_preview(fa_preview, fa_preview_size, key, true);
    HANDLE_ERROR(result);
    printf("%s\n", fa_preview);

    /*
     * Fourier analysis with all results
     */
    // 1. Get results size
    result += gn_fft_analysis_results_size(&fft_results_size, key, fft_cplx_size, nfft);
    // 2. Allocate memory for result keys and values
    fft_rkeys      = malloc(fft_results_size * sizeof(char*));
    fft_rvalues    = malloc(fft_results_size * sizeof(double));
    // 3. Get result key sizes
    fft_rkey_sizes = malloc(fft_results_size * sizeof(size_t));
    result += gn_fft_analysis_results_key_sizes(fft_rkey_sizes, fft_results_size, key, fft_cplx_size, nfft);
    HANDLE_ERROR(result);
    // 4. Allocate memory for each result key
    for (size_t i = 0; i < fft_results_size; ++i) {
        fft_rkeys[i] = malloc(fft_rkey_sizes[i]);
    }
    // 5. Execute analysis
    result += gn_fft_analysis(fft_rkeys, fft_results_size, fft_rvalues, fft_results_size,
        key, fft_cplx, fft_cplx_size, nfft, axis_type);
    HANDLE_ERROR(result);

    /*
     * Fourier analysis with single result
     */
    double fsnr = 0.0;
    double a_mag_dbfs = 0.0;
    result += gn_fft_analysis_single(&fsnr,       key, "fsnr",       fft_cplx, fft_cplx_size, nfft, axis_type);
    result += gn_fft_analysis_single(&a_mag_dbfs, key, "A:mag_dbfs", fft_cplx, fft_cplx_size, nfft, axis_type);
    HANDLE_ERROR(result);

    /*
     * Fourier analysis with select results
     */
    const char* fft_select_rkeys[] = {"sfdr", "-3A:mag_dbc"};
    size_t fft_select_results_size = (sizeof fft_select_rkeys) / sizeof(char*);
    fft_select_rvalues = malloc(fft_select_results_size * sizeof(double));
    result = gn_fft_analysis_select(fft_select_rvalues, fft_select_results_size,
        key, fft_select_rkeys, fft_select_results_size, fft_cplx, fft_cplx_size, nfft, axis_type);
    HANDLE_ERROR(result);

    /*
     * Carrier and MaxSpur keys (requires all results)
     */
    size_t fa_result_string_size = 0;
    result = gn_fa_result_string_size(&fa_result_string_size,
        (const char**)fft_rkeys, fft_results_size, fft_rvalues, fft_results_size, "carrierindex");
    HANDLE_ERROR(result);
    fa_carrier = malloc(fa_result_string_size);
    result = gn_fa_result_string(fa_carrier, fa_result_string_size,
        (const char**)fft_rkeys, fft_results_size, fft_rvalues, fft_results_size, "carrierindex");
    HANDLE_ERROR(result);
    result = gn_fa_result_string_size(&fa_result_string_size,
        (const char**)fft_rkeys, fft_results_size, fft_rvalues, fft_results_size, "maxspurindex");
    HANDLE_ERROR(result);
    fa_maxspur = malloc(fa_result_string_size);
    result = gn_fa_result_string(fa_maxspur, fa_result_string_size,
        (const char**)fft_rkeys, fft_results_size, fft_rvalues, fft_results_size, "maxspurindex");
    HANDLE_ERROR(result);

    /*
     * Print results
     */
    printf("\nAll Fourier Analysis Results:\n");
    for (size_t i = 0; i < fft_results_size; ++i) {
        printf("%4zu%20s%20.6f\n", i, fft_rkeys[i], fft_rvalues[i]);
    }
    // gn_fa_result extracts specified result from key-value arrays
    double snr = 0.0;
    double a_freq = 0.0;
    double a_ffinal = 0.0;
    result = gn_fa_result(&snr,
        (const char**)fft_rkeys, fft_results_size, fft_rvalues, fft_results_size, "snr");
    result = gn_fa_result(&a_freq,
        (const char**)fft_rkeys, fft_results_size, fft_rvalues, fft_results_size, "A:freq");
    result = gn_fa_result(&a_ffinal,
        (const char**)fft_rkeys, fft_results_size, fft_rvalues, fft_results_size, "A:ffinal");
    HANDLE_ERROR(result);
    printf("    %s = %.3f\n",     "snr",      snr);
    printf("    %s = %.3f MHz\n", "A:freq",   a_freq / 1e6);
    printf("    %s = %.3f MHz\n", "A:ffinal", a_ffinal / 1e6);
    printf("    %s = %s\n",       "Carrier",  fa_carrier);
    printf("    %s = %s\n",       "MaxSpur",  fa_maxspur);

    printf("\nSingle Fourier Analysis Results:\n");
    printf("%20s = %20.6f\n", "fsnr", fsnr);
    printf("%20s = %20.6f\n", "A::mag_dbfs", a_mag_dbfs);

    printf("\nSelect Fourier Analysis Results:\n");
    for (size_t i = 0; i < fft_select_results_size; ++i) {
        printf("%20s = %20.6f\n", fft_select_rkeys[i], fft_select_rvalues[i]);
    }

    /*
     * Save data
     */
    if (save_data) {
        const char* fn = "fft.txt";
        FILE* fp = fopen(fn, "w");
        if (fp) {
            fft_db = malloc(nfft * sizeof(double));
            result += gn_db(fft_db, nfft, fft_cplx, fft_cplx_size);
            if (GnFreqAxisTypeDcCenter == axis_type) {
                gn_fftshift(fft_db, nfft, fft_db, nfft);
            }
            for (size_t i = 0; i < nfft; ++i) {
                fprintf(fp, "%.6f\n", fft_db[i]);
            }
            fclose(fp);
            printf("\nWrote FFT magnitude data to %s\n", fn);
        }
        fn = "fft_axis.txt";
        fp = fopen(fn, "w");
        if (fp) {
            freq_axis = malloc(nfft * sizeof(double));
            result += gn_freq_axis(freq_axis, nfft, nfft, axis_type, fdata, axis_fmt);
            for (size_t i = 0; i < nfft; ++i) {
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
    free(fft_select_rvalues);
    if (fft_rkeys) {
        for (size_t i = 0; i < fft_results_size; ++i) {
            free(fft_rkeys[i]);
        }
    }
    free(fft_rkey_sizes);
    free(fft_rvalues);
    free(fft_rkeys);
    free(fa_maxspur);
    free(fa_carrier);
    free(fa_preview);
    free(freq_axis);
    free(fft_db);
    free(fft_cplx);
    free(ywf);
    free(xwf);
    free(qwfq);
    free(qwfi);
    free(awfq);
    free(awfi);

    return 0;
}