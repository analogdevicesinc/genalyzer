#include "../tests/cJSON.h"
#include "../tests/test_genalyzer.h"
#include <cgenalyzer.h>
#include <cgenalyzer_simplified_beta.h>
#include <gperftools/profiler.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ERROR_BUFFER_SIZE 256

#define HANDLE_ERROR(x)                                                       \
  do                                                                          \
    {                                                                         \
      if (0 != x)                                                             \
        {                                                                     \
          gn_error_string (error_buffer, ERROR_BUFFER_SIZE);                  \
          printf ("%s\n", error_buffer);                                      \
          goto cleanup;                                                       \
        }                                                                     \
    }                                                                         \
  while (0)

int
main (int argc, char *argv[])
{
  bool save_data = false;
  if (1 < argc)
    {
      save_data = !((bool)strcmp (argv[1], "--save-data"));
    }

  /*
   * Setup
   */
  gn_set_string_termination (true);
  char error_buffer[ERROR_BUFFER_SIZE];
  bool error_occurred = true;
  int result = 0;

  size_t navg = 1;         // FFT averaging
  size_t nfft = 32768;     // FFT size
  double fs = 3e6;         // sample rate
  double fdata = fs / 1;   // data rate
  double fshift = 0e6;     // shift frequency
  double fsr = 2.0;        // full-scale range
  double ampl_dbfs = -1.0; // input amplitude (dBFS)
  double freq = 100000;    // input frequency (Hz)
  double phase = 0.0;      // input phase (rad)
  double td = 0.0;         // input delay (s)
  double tj = 0.0;         // RMS aperature uncertainty (s)
  double qpe = 0.0; // (atan(1.0) * 2) * 1e-5;        // quadrature phase error
                    // (pi/2 * x)
  double poco[]
      = { 0.0, 1.0, 0.0, 0.003 }; // distortion polynomial coefficients
  int qres = 12;                  // quantizer resolution
  double qnoise_dbfs = -63.0;     // quantizer noise (dBFS)
  GnCodeFormat code_fmt = GnCodeFormatTwosComplement;
  GnFreqAxisFormat axis_fmt = GnFreqAxisFormatFreq;
  GnFreqAxisType axis_type = GnFreqAxisTypeDcCenter;
  GnWindow window = GnWindowNoWindow;

  int dsr = (int)(fs / fdata);
  size_t npts = navg * nfft * dsr;
  double ampl = (fsr / 2) * pow (10.0, ampl_dbfs / 20.0);
  double qnoise = pow (10.0, qnoise_dbfs / 20.0);
  int ssb_fund = 4;
  int ssb_rest = 3;
  const char *test_filename
      = "../tests/test_vectors/test_Pluto_DDS_data_1658159639196.json";

  /*
   * Pointers for allocated memory
   */
  double *awfi = NULL;               // in-phase analog waveform
  double *awfq = NULL;               // quadrature analog waveform
  int16_t *qwfi = NULL;              // in-phase quantized waveform
  int16_t *qwfq = NULL;              // quadrature quantized waveform
  int16_t *xwf = NULL;               // frequency-shifted waveform
  int16_t *ywf = NULL;               // downsampled waveform
  double *fft_cplx = NULL;           // FFT complex data
  double *fft_db = NULL;             // FFT magnitude data
  double *freq_axis = NULL;          // frequency axis
  char *fa_preview = NULL;           // FFT analysis configuration preview
  char *fa_carrier = NULL;           // FFT analysis carrier key
  char *fa_maxspur = NULL;           // FFT analysis maxspur key
  char **fft_rkeys = NULL;           // FFT analysis keys
  double *fft_rvalues = NULL;        // FFT analysis values
  size_t *fft_rkey_sizes = NULL;     // FFT analysis key sizes
  double *fft_select_rvalues = NULL; // FFT analysis select values

  size_t fft_results_size = 0;

  /*
   * Allocate memory
   */
  size_t xwf_size = npts * 2;
  size_t ywf_size = 0;
  size_t fft_cplx_size = nfft * 2;
  result += gn_downsample_size (&ywf_size, xwf_size, dsr, true);
  HANDLE_ERROR (result);
  awfi = malloc (npts * sizeof (double));
  awfq = malloc (npts * sizeof (double));
  qwfi = malloc (npts * sizeof (int16_t));
  qwfq = malloc (npts * sizeof (int16_t));
  xwf = malloc (xwf_size * sizeof (int16_t));
  ywf = malloc (ywf_size * sizeof (int16_t));
  fft_cplx = malloc (fft_cplx_size * sizeof (double));

  /*
   * Signal generation and processing
   */
  if (GnWindowNoWindow == window)
    {
      result += gn_coherent (&freq, nfft, fdata, freq);
      HANDLE_ERROR (result);
      double fbin = fdata / nfft;
      fshift = round (fshift / fbin) * fbin;
      ssb_fund = 50;
      ssb_rest = 3;
    }

  /*
  result += gn_cos(awfi, npts, fs, ampl, freq, phase, td, tj);
  result += gn_sin(awfq, npts, fs, ampl, freq, phase + qpe, td, tj);
  result += gn_polyval(awfi, npts, awfi, npts, poco, (sizeof poco) /
  sizeof(double)); result += gn_polyval(awfq, npts, awfq, npts, poco, (sizeof
  poco) / sizeof(double)); result += gn_quantize16(qwfi, npts, awfi, npts, fsr,
  qres, qnoise, code_fmt); result += gn_quantize16(qwfq, npts, awfq, npts, fsr,
  qres, qnoise, code_fmt);
  */
  result += read_array_from_json_file (test_filename, "test_vec_i", qwfi,
                                       INT16, npts);
  result += read_array_from_json_file (test_filename, "test_vec_q", qwfq,
                                       INT16, npts);
  // result += read_array_from_json_file_int32(test_filename, "test_vec_i",
  // qwfi, npts); result += read_array_from_json_file_int32(test_filename,
  // "test_vec_q", qwfq, npts); result += gn_fshift16(xwf, xwf_size, qwfi,
  // npts, qwfq, npts, qres, fs, fshift, code_fmt); result +=
  // gn_downsample16(ywf, ywf_size, xwf, xwf_size, dsr, true);
  /*
  printf("qwf---------\n");
  for (int ii = 0; ii < 10; ii++)
      printf("%d\t%d\n", qwfi[ii], qwfq[ii]);
  */
  ProfilerStart ("profile.log");
  result += gn_fft16 (fft_cplx, fft_cplx_size, qwfi, npts, qwfq, npts, qres,
                      navg, nfft, window, code_fmt);
  /*
  printf("xwf---------\n");
  for (int ii = 0; ii < 10; ii++)
      printf("%d\n", xwf[ii]);
  */
  // result += gn_fft16(fft_cplx, fft_cplx_size, xwf, 2*npts, NULL, 0, qres,
  // navg, nfft, window, code_fmt); return 0;

  /*
  FILE *fp1 = fopen("fft_data_debug.txt", "w");
  for (int i =0; i < fft_cplx_size; i++)
      fprintf(fp1, "%f\n", fft_cplx[i]);
  fclose(fp1);
  HANDLE_ERROR(result);
  */

  /*
   * Fourier analysis configuration
   */
  char *key = "fa";
  result += gn_fa_create (key);
  result += gn_fa_fixed_tone (key, "A", GnFACompTagSignal, freq, ssb_fund);
  result += gn_fa_hd (key, 3);
  result += gn_fa_ssb (key, GnFASsbDefault, ssb_rest);
  result += gn_fa_ssb (key, GnFASsbDC, -1);
  result += gn_fa_ssb (key, GnFASsbSignal, -1);
  result += gn_fa_ssb (key, GnFASsbWO, -1);
  result += gn_fa_fsample (key, fs);
  result += gn_fa_fdata (key, fdata);
  result += gn_fa_fshift (key, fshift);
  result += gn_fa_conv_offset (key, 0.0 != fshift);
  HANDLE_ERROR (result);
  size_t fa_preview_size = 0;
  result += gn_fa_preview_size (&fa_preview_size, key, true);
  fa_preview = malloc (fa_preview_size);
  result += gn_fa_preview (fa_preview, fa_preview_size, key, true);
  HANDLE_ERROR (result);
  // printf("%s\n", fa_preview);

  /*
   * Fourier analysis with all results
   */
  // 1. Get results size
  result += gn_fft_analysis_results_size (&fft_results_size, key,
                                          fft_cplx_size, nfft);
  // 2. Allocate memory for result keys and values
  fft_rkeys = malloc (fft_results_size * sizeof (char *));
  fft_rvalues = malloc (fft_results_size * sizeof (double));
  // 3. Get result key sizes
  fft_rkey_sizes = malloc (fft_results_size * sizeof (size_t));
  result += gn_fft_analysis_results_key_sizes (
      fft_rkey_sizes, fft_results_size, key, fft_cplx_size, nfft);
  HANDLE_ERROR (result);
  // 4. Allocate memory for each result key
  for (size_t i = 0; i < fft_results_size; ++i)
    {
      fft_rkeys[i] = malloc (fft_rkey_sizes[i]);
    }
  // 5. Execute analysis
  result += gn_fft_analysis (fft_rkeys, fft_results_size, fft_rvalues,
                             fft_results_size, key, fft_cplx, fft_cplx_size,
                             nfft, axis_type);
  HANDLE_ERROR (result);
  // return 0;
  ProfilerStop ();

cleanup:
  if (error_occurred)
    {
      printf ("\nError occurred, freeing memory\n");
    }
  else
    {
      printf ("\nFreeing memory\n");
    }
  free (fft_select_rvalues);
  if (fft_rkeys)
    {
      for (size_t i = 0; i < fft_results_size; ++i)
        {
          free (fft_rkeys[i]);
        }
    }
  free (fft_rkey_sizes);
  free (fft_rvalues);
  free (fft_rkeys);
  free (fa_maxspur);
  free (fa_carrier);
  free (fa_preview);
  free (freq_axis);
  free (fft_db);
  free (fft_cplx);
  free (ywf);
  free (xwf);
  // free(qwfq);
  // free(qwfi);
  // free(awfq);
  // free(awfi);

  return 0;
}