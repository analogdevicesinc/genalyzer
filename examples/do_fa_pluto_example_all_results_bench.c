#include "cgenalyzer_simplified_beta.h"
#include "../tests/test_genalyzer.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

static double now_sec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

int main(int argc, const char *argv[]) {
    const char *test_filename = "tests/test_vectors/test_Pluto_DDS_data_1658159639196.json";
    size_t iters = 200;
    size_t warmup = 20;

    for (int i = 1; i + 1 < argc; i += 2) {
        if (0 == strcmp(argv[i], "--input")) {
            test_filename = argv[i + 1];
        } else if (0 == strcmp(argv[i], "--iters")) {
            iters = strtoull(argv[i + 1], NULL, 10);
        } else if (0 == strcmp(argv[i], "--warmup")) {
            warmup = strtoull(argv[i + 1], NULL, 10);
        }
    }

    int32_t *ref_qwfi, *ref_qwfq;
    double *fft_out;
    size_t results_size;
    char **rkeys;
    double *rvalues;

    int qres;
    unsigned long long npts, navg, nfft, tmp_win;
    double fs;
    GnWindow win;

    read_scalar_from_json_file(test_filename, "qres", (void *)(&qres), INT32);
    read_scalar_from_json_file(test_filename, "npts", (void *)(&npts), UINT64);
    read_scalar_from_json_file(test_filename, "navg", (void *)(&navg), UINT64);
    read_scalar_from_json_file(test_filename, "fs", (void *)(&fs), DOUBLE);
    read_scalar_from_json_file(test_filename, "nfft", (void *)(&nfft), UINT64);
    read_scalar_from_json_file(test_filename, "win", (void *)(&tmp_win), UINT64);
    if (tmp_win == 1)
        win = GnWindowBlackmanHarris;
    else if (tmp_win == 2)
        win = GnWindowHann;
    else
        win = GnWindowNoWindow;

    ref_qwfi = (int32_t *)malloc(npts * sizeof(int32_t));
    ref_qwfq = (int32_t *)malloc(npts * sizeof(int32_t));
    read_array_from_json_file(test_filename, "test_vec_i", ref_qwfi, INT32, npts);
    read_array_from_json_file(test_filename, "test_vec_q", ref_qwfq, INT32, npts);

    gn_config c = NULL;
    gn_config_fftz(npts, qres, navg, nfft, win, &c);
    gn_config_set_sample_rate(fs, &c);
    gn_config_fa_auto(120, &c);

    double checksum = 0.0;

    for (size_t i = 0; i < warmup; i++) {
        gn_fftz(&fft_out, ref_qwfi, ref_qwfq, &c);
        gn_get_fa_results(&rkeys, &rvalues, &results_size, fft_out, &c);
        for (size_t k = 0; k < results_size; k++) {
            checksum += rvalues[k];
        }
        free(fft_out);
    }

    double t0 = now_sec();
    for (size_t i = 0; i < iters; i++) {
        gn_fftz(&fft_out, ref_qwfi, ref_qwfq, &c);
        gn_get_fa_results(&rkeys, &rvalues, &results_size, fft_out, &c);
        for (size_t k = 0; k < results_size; k++) {
            checksum += rvalues[k];
        }
        free(fft_out);
    }
    double elapsed = now_sec() - t0;

    printf("{\"impl\":\"c\",\"iters\":%zu,\"warmup\":%zu,\"elapsed_sec\":%.9f,\"checksum\":%.9f}\n", iters, warmup, elapsed, checksum);

    free(ref_qwfi);
    free(ref_qwfq);
    gn_config_free(&c);
    return 0;
}
