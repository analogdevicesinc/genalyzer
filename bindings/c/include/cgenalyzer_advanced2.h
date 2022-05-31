#ifndef CGENALYZER_ADVANCED2_H
#define CGENALYZER_ADVANCED2_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * \mainpage Genalyzer Library API Documentation
 * \section Overview
 * The Genalyzer Library provides analysis routines for data converter testing.  The
 * library contains routines that analyze waveforms, FFTs, and the output of the traditional code
 * density tests: histogram, DNL, and INL.  In addition, the library provides basic signal
 * generation and processing utilties.
 * \section AnalysisRoutines Analysis Routines
 * The library provides the following types of analysis:
 * \li \ref gn_dnl_analysis "DNL Analysis"
 * \li \ref gn_fft_analysis "Fourier Analysis"
 * \li \ref gn_hist_analysis "Histogram Analysis"
 * \li \ref gn_inl_analysis "INL Analysis"
 * \li \ref gn_wf_analysis "Waveform Analysis"
 * 
 * Each analysis routine returns results by filling a Keys array (rkeys) and a corresponding Values
 * array (rvalues).  Together, rkeys and rvalues represent a set of key-value result pairs:
 * rkeys[0] corresponds to rvalues[0], rkeys[1] to rvalues[1], and so on.
 */

/* Enumerations */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup Enumerations Enumerations
 * @{
 */

/**
 * @brief GnAnalysisType enumerates analysis types
 */
typedef enum GnAnalysisType {
             GnAnalysisTypeDNL,                ///< DNL (differential nonlinearity)
             GnAnalysisTypeFourier,            ///< Fourier (FFT)
             GnAnalysisTypeHistogram,          ///< Histogram
             GnAnalysisTypeINL,                ///< INL (integral nonlinearity)
             GnAnalysisTypeWaveform            ///< Waveform
           } GnAnalysisType;

/**
 * @brief GnCodeFormat enumerates binary code formats
 */
typedef enum GnCodeFormat {
             GnCodeFormatOffsetBinary,         ///< Offset Binary
             GnCodeFormatTwosComplement        ///< Two's Complement
           } GnCodeFormat;

/**
 * @brief GnDnlSignal enumerates signal types for which DNL can be computed
 */
typedef enum GnDnlSignal {
             GnDnlSignalRamp,                  ///< Ramp
             GnDnlSignalTone                   ///< Tone (Sinusoid)
           } GnDnlSignal;

/**
 * @brief GnFACompTag enumerates Fourier analysis component tags
 */
typedef enum GnFACompTag {
             GnFACompTagDC,                    ///< DC component (always Bin 0)
             GnFACompTagSignal,                ///< Signal component
             GnFACompTagHD,                    ///< Harmonic distortion
             GnFACompTagIMD,                   ///< Intermodulation distortion
             GnFACompTagILOS,                  ///< Interleaving offset component
             GnFACompTagILGT,                  ///< Interleaving gain/timing/BW component
             GnFACompTagCLK,                   ///< Clock component
             GnFACompTagUserDist,              ///< User-designated distortion
             GnFACompTagNoise                  ///< Noise component (e.g. WorstOther)
           } GnFACompTag;

/**
 * @brief GnFASsb enumerates the component categories for which the number of single side bins
 *        (SSB) can be set
 */
typedef enum GnFASsb {
             GnFASsbDefault,                   ///< Default SSB (applies to auto-generated components)
             GnFASsbDC,                        ///< SSB for DC component
             GnFASsbSignal,                    ///< SSB for Signal components
             GnFASsbWO,                        ///< SSB for WorstOther components
           } GnFASsb;

/**
 * @brief GnFreqAxisFormat enumerates frequency axis formats
 */
typedef enum GnFreqAxisFormat {
             GnFreqAxisFormatBins,             ///< Bins
             GnFreqAxisFormatFreq,             ///< Frequency
             GnFreqAxisFormatNorm              ///< Normalized
           } GnFreqAxisFormat;

/**
 * @brief GnFreqAxisType enumerates frequency axis types
 */
typedef enum GnFreqAxisType {
             GnFreqAxisTypeDcCenter,           ///< DC centered, e.g. [-fs/2, fs/2) (complex FFT only)
             GnFreqAxisTypeDcLeft,             ///< DC on left, e.g. [0, fs) (complex FFT only)
             GnFreqAxisTypeReal                ///< Real axis, e.g. [0, fs/2] (real FFT only)
           } GnFreqAxisType;

/**
 * @brief GnInlLineFit enumerates INL line fitting options
 */
typedef enum GnInlLineFit {
             GnInlLineFitBestFit,              ///< Best fit
             GnInlLineFitEndFit,               ///< End fit
             GnInlLineFitNoFit                 ///< No fit
           } GnInlLineFit;

/**
 * @brief GnRfftScale enumerates real FFT scaling options
 */
typedef enum GnRfftScale {
             GnRfftScaleDbfsDc,                ///< Full-scale sinusoid measures -3 dBFS
             GnRfftScaleDbfsSin,               ///< Full-scale sinusoid measures  0 dBFS
             GnRfftScaleNative                 ///< Full-scale sinusoid measures -6 dBFS
           } GnRfftScale;

/**
 * @brief GnWindow enumerates window functions
 */
typedef enum GnWindow {
             GnWindowBlackmanHarris,           ///< Blackman-Harris
             GnWindowHann,                     ///< Hann ("Hanning")
             GnWindowNoWindow                  ///< No window (Rectangular)
           } GnWindow;

/** @} Enumerations */

#ifdef __cplusplus
} // extern "C"
#endif

/* API Utilities */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup APIUtilities API Utilities
 * @{
 */

/**
 * @brief gn_analysis_results_key_sizes
 * @return 0 on success, non-zero otherwise
 * @details The library string termination setting determines whether or not a null terminator
 * is included in the key sizes.  See \ref gn_set_string_termination.
 */
int gn_analysis_results_key_sizes(
    size_t* key_sizes,                  ///< [out] Key size array pointer
    size_t key_sizes_size,              ///< [in] Key size array size
    GnAnalysisType type                ///< [in] Analysis type
    );

/**
 * @brief gn_analysis_results_size
 * @return 0 on success, non-zero otherwise
 */
int gn_analysis_results_size(
    size_t* size,                       ///< [out] Number of key-value result pairs
    GnAnalysisType type                ///< [in] Analysis type
    );

/**
 * @brief gn_enum_value
 * @return 0 on success, non-zero otherwise
 */
int gn_enum_value(
    int* value,                         ///< [out] Underlying value of enumeration::enumerator
    const char* enumeration,            ///< [in] Enumeration name
    const char* enumerator              ///< [in] Enumerator name
    );

/**
 * @brief gn_error_check
 * @return Always returns 0
 */
int gn_error_check(
    bool* error                         ///< [out] true if an error has occurred; false otherwise
);

/**
 * @brief gn_error_clear
 * @return Always returns 0
 */
int gn_error_clear();

/**
 * @brief gn_error_string
 * @return 0 on success, non-zero otherwise
 */
int gn_error_string(
    char* buf,                          ///< [out] Pointer to character array
    size_t size                         ///< [in] Size of character array
    );

/**
 * @brief gn_set_string_termination
 * @return Always returns 0
 * @details Some functions in this library return strings by filling character buffers (arrays)
 * provided by the caller.  This function sets a global library setting that determines whether or
 * not strings should be null-terminated.  If set to true, functions that return strings will
 * write a '\0' as the last character.  In addition, functions that return the size of a string
 * will include the null terminator in the size.
 */
int gn_set_string_termination(
    bool null_terminated                ///< [in] If true, strings are terminated with '\0'
);

/**
 * @brief gn_version_string
 * @return 0 on success, non-zero otherwise
 */
int gn_version_string(
    char* buf,                          ///< [in,out] Pointer to character array
    size_t size                         ///< [in] Size of character array
    );

/**
 * \defgroup APIUtilityHelpers Helpers
 * @{
 */

/**
 * @brief gn_error_string_size
 * @return Always returns 0
 * @details The library string termination setting determines whether or not a null terminator
 * is included in the size.  See \ref gn_set_string_termination.
 */
int gn_error_string_size(
    size_t* size                        ///< [out] Number of characters in error string
);

/**
 * @brief gn_version_string_size
 * @return Always returns 0
 * @details The library string termination setting determines whether or not a null terminator
 * is included in the size.  See \ref gn_set_string_termination.
 */
int gn_version_string_size(
    size_t* size                        ///< [out] Number of characters in version string
);

/** @} APIUtilityHelpers */

/** @} APIUtilities */

#ifdef __cplusplus
} // extern "C"
#endif

/* Array Operations */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup ArrayOperations Array Operations
 * @{
 */

/**
 * @brief gn_abs
 * @return 0 on success, non-zero otherwise
 */
int gn_abs(
    double* out,                        ///< [out] Output array pointer
    size_t out_size,                    ///< [in] Output array size
    const double* in,                   ///< [in] Input array pointer
    size_t in_size                      ///< [in] Input array size
    );

/**
 * @brief gn_angle
 * @return 0 on success, non-zero otherwise
 */
int gn_angle(
    double* out,                        ///< [out] Output array pointer
    size_t out_size,                    ///< [in] Output array size
    const double* in,                   ///< [in] Input array pointer
    size_t in_size                      ///< [in] Input array size
    );

/**
 * @brief gn_db
 * @return 0 on success, non-zero otherwise
 */
int gn_db(
    double* out,                        ///< [out] Output array pointer
    size_t out_size,                    ///< [in] Output array size
    const double* in,                   ///< [in] Input array pointer
    size_t in_size                      ///< [in] Input array size
    );

/**
 * @brief gn_db10
 * @return 0 on success, non-zero otherwise
 */
int gn_db10(
    double* out,                        ///< [out] Output array pointer
    size_t out_size,                    ///< [in] Output array size
    const double* in,                   ///< [in] Input array pointer
    size_t in_size                      ///< [in] Input array size
    );

/**
 * @brief gn_db20
 * @return 0 on success, non-zero otherwise
 */
int gn_db20(
    double* out,                        ///< [out] Output array pointer
    size_t out_size,                    ///< [in] Output array size
    const double* in,                   ///< [in] Input array pointer
    size_t in_size                      ///< [in] Input array size
    );

/**
 * @brief gn_norm
 * @return 0 on success, non-zero otherwise
 */
int gn_norm(
    double* out,                        ///< [out] Output array pointer
    size_t out_size,                    ///< [in] Output array size
    const double* in,                   ///< [in] Input array pointer
    size_t in_size                      ///< [in] Input array size
    );

/** @} ArrayOperations */

#ifdef __cplusplus
} // extern "C"
#endif

/* Code Density */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup CodeDensity Code Density
 * @{
 */

/**
 * @brief gn_code_axis
 * @return 0 on success, non-zero otherwise
 */
int gn_code_axis(
    double* out,                        ///< [out] Array pointer
    size_t size,                        ///< [in] Array size
    int n,                              ///< [in] Resolution
    GnCodeFormat format                ///< [in] Code format
    );

/**
 * @brief gn_code_axisx
 * @return 0 on success, non-zero otherwise
 */
int gn_code_axisx(
    double* out,                       ///< [out] Array pointer
    size_t size,                        ///< [in] Array size
    int64_t min,                        ///< [in] Min code
    int64_t max                         ///< [in] Max code
    );

/**
 * @brief gn_dnl
 * @return 0 on success, non-zero otherwise
 */
int gn_dnl(
    double* dnl,                        ///< [out] Output array pointer
    size_t dnl_size,                    ///< [in] Output array size
    const uint64_t* hist,               ///< [in] Input array pointer
    size_t hist_size,                   ///< [in] Input array size
    GnDnlSignal type                   ///< [in] Signal type
    );

/**
 * @brief gn_dnl_analysis
 * @return 0 on success, non-zero otherwise
 * @details The results contain the following key-value pairs (see general description of
 * \ref AnalysisRoutines "Analysis Routines").
 * <table>
 *   <tr><th> Key            <th> Description
 *   <tr><td> min            <td> Minumum value
 *   <tr><td> max            <td> Maximum value
 *   <tr><td> avg            <td> Average value
 *   <tr><td> rms            <td> RMS value
 *   <tr><td> min_index      <td> Index of first occurence of minimum value
 *   <tr><td> max_index      <td> Index of first occurence of maximum value
 *   <tr><td> first_nm_index <td> Index of first non-missing code
 *   <tr><td> last_nm_index  <td> Index of last non-missing code
 *   <tr><td> nm_range       <td> Non-missing code range (1 + (last_nm_index - first_nm_index))
 * </table>
 */
int gn_dnl_analysis(
    char** rkeys,                       ///< [out] Result keys array pointer
    size_t rkeys_size,                  ///< [in] Result keys array size
    double* rvalues,                    ///< [out] Result values array pointer
    size_t rvalues_size,                ///< [in] Result values array size
    const double* dnl,                  ///< [in] Input array pointer
    size_t dnl_size                     ///< [in] Input array size
    );

/**
 * @brief gn_hist16
 * @return 0 on success, non-zero otherwise
 */
int gn_hist16(
    uint64_t* hist,                     ///< [out] Histogram array pointer
    size_t hist_size,                   ///< [in] Histogram array size
    const int16_t* in,                  ///< [in] Input array pointer
    size_t in_size,                     ///< [in] Input array size
    int n,                              ///< [in] Code width (i.e. ADC resolution)
    GnCodeFormat format,               ///< [in] Code format
    bool preserve                       ///< [in] If true, hist is not cleared before computing the histogram
    );

/**
 * @brief gn_hist32
 * @return 0 on success, non-zero otherwise
 */
int gn_hist32(
    uint64_t* hist,                     ///< [out] Histogram array pointer
    size_t hist_size,                   ///< [in] Histogram array size
    const int32_t* in,                  ///< [in] Input array pointer
    size_t in_size,                     ///< [in] Input array size
    int n,                              ///< [in] Code width (i.e. ADC resolution)
    GnCodeFormat format,                ///< [in] Code format
    bool preserve                       ///< [in] If true, hist is not cleared before computing the histogram
    );

/**
 * @brief gn_hist64
 * @return 0 on success, non-zero otherwise
 */
int gn_hist64(
    uint64_t* hist,                     ///< [out] Histogram array pointer
    size_t hist_size,                   ///< [in] Histogram array size
    const int64_t* in,                  ///< [in] Input array pointer
    size_t in_size,                     ///< [in] Input array size
    int n,                              ///< [in] Code width (i.e. ADC resolution)
    GnCodeFormat format,                ///< [in] Code format
    bool preserve                       ///< [in] If true, hist is not cleared before computing the histogram
    );

/**
 * @brief gn_histx16
 * @return 0 on success, non-zero otherwise
 */
int gn_histx16(
    uint64_t* hist,                     ///< [out] Histogram array pointer
    size_t hist_size,                   ///< [in] Histogram array size
    const int16_t* in,                  ///< [in] Input array pointer
    size_t in_size,                     ///< [in] Input array size
    int64_t min,                        ///< [in] Min code
    int64_t max,                        ///< [in] Max code
    bool preserve                       ///< [in] If true, hist is not cleared before computing the histogram
    );

/**
 * @brief gn_histx32
 * @return 0 on success, non-zero otherwise
 */
int gn_histx32(
    uint64_t* hist,                     ///< [out] Histogram array pointer
    size_t hist_size,                   ///< [in] Histogram array size
    const int32_t* in,                  ///< [in] Input array pointer
    size_t in_size,                     ///< [in] Input array size
    int64_t min,                        ///< [in] Min code
    int64_t max,                        ///< [in] Max code
    bool preserve                       ///< [in] If true, hist is not cleared before computing the histogram
    );

/**
 * @brief gn_histx64
 * @return 0 on success, non-zero otherwise
 */
int gn_histx64(
    uint64_t* hist,                     ///< [out] Histogram array pointer
    size_t hist_size,                   ///< [in] Histogram array size
    const int64_t* in,                  ///< [in] Input array pointer
    size_t in_size,                     ///< [in] Input array size
    int64_t min,                        ///< [in] Min code
    int64_t max,                        ///< [in] Max code
    bool preserve                       ///< [in] If true, hist is not cleared before computing the histogram
    );

/**
 * @brief gn_hist_analysis
 * @return 0 on success, non-zero otherwise
 * @details The results contain the following key-value pairs (see general description of
 * \ref AnalysisRoutines "Analysis Routines").
 * <table>
 *   <tr><th> Key            <th> Description
 *   <tr><td> sum            <td> Sum of all histogram bins
 *   <tr><td> first_nz_index <td> First non-zero bin
 *   <tr><td> last_nz_index  <td> Last non-zero bin
 *   <tr><td> nz_range       <td> Non-zero bin range (1 + (last_nz_index - first_nz_index))
 * </table>
 */
int gn_hist_analysis(
    char** rkeys,                       ///< [out] Result keys array pointer
    size_t rkeys_size,                  ///< [in] Result keys array size
    double* rvalues,                    ///< [out] Result values array pointer
    size_t rvalues_size,                ///< [in] Result values array size
    const uint64_t* hist,               ///< [in] Input array pointer
    size_t hist_size                    ///< [in] Input array size
    );

/**
 * @brief gn_inl
 * @return 0 on success, non-zero otherwise
 */
int gn_inl(
    double* inl,                        ///< [out] Output array pointer
    size_t inl_size,                    ///< [in] Output array size
    const double* dnl,                  ///< [in] Input array pointer
    size_t dnl_size,                    ///< [in] Input array size
    GnInlLineFit fit                   ///< [in] Line fit type
    );

/**
 * @brief gn_inl_analysis
 * @return 0 on success, non-zero otherwise
 * @details The results contain the following key-value pairs (see general description of
 * \ref AnalysisRoutines "Analysis Routines").
 * <table>
 *   <tr><th> Key       <th> Description
 *   <tr><td> min       <td> Minumum value
 *   <tr><td> max       <td> Maximum value
 *   <tr><td> min_index <td> Index of first occurence of minimum value
 *   <tr><td> max_index <td> Index of first occurence of maximum value
 * </table>
 */
int gn_inl_analysis(
    char** rkeys,                       ///< [out] Result keys array pointer
    size_t rkeys_size,                  ///< [in] Result keys array size
    double* rvalues,                    ///< [out] Result values array pointer
    size_t rvalues_size,                ///< [in] Result values array size
    const double* inl,                  ///< [in] Input array pointer
    size_t inl_size                     ///< [in] Input array size
    );

/**
 * \defgroup CodeDensityHelpers Helpers
 * @{
 */

/**
 * @brief gn_code_density_size
 * @return 0 on success, non-zero otherwise
 */
int gn_code_density_size(
    size_t* size,                       ///< [out] Output array size
    int n,                              ///< [in] Code width (i.e. ADC resolution)
    GnCodeFormat format                ///< [in] Code format
    );

/**
 * @brief gn_code_densityx_size
 * @return 0 on success, non-zero otherwise
 */
int gn_code_densityx_size(
    size_t* size,                       ///< [out] Output array size
    int64_t min,                        ///< [in] Min code
    int64_t max                         ///< [in] Max code
    );

/** @} CodeDensityHelpers */

/** @} CodeDensity */

#ifdef __cplusplus
} // extern "C"
#endif

/* Fourier Analysis */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup FourierAnalysis Fourier Analysis
 * @{
 */

/**
 * @brief gn_fft_analysis returns all Fourier analysis results
 * @return 0 on success, non-zero otherwise
 * @details The results contain the following key-value pairs (see general description of
 * \ref AnalysisRoutines "Analysis Routines").
 * <table>
 *   <tr><th> Key                  <th> Description                                   <th> Units
 *   <tr><td> signaltype           <td> Signal type: 0=Real, 1=Complex                <td>
 *   <tr><td> nfft                 <td> FFT size                                      <td>
 *   <tr><td> datasize             <td> Data size                                     <td>
 *   <tr><td> fbin                 <td> Frequency bin size                            <td> Hz
 *   <tr><td> fdata                <td> Data rate                                     <td> S/s
 *   <tr><td> fsample              <td> Sample rate                                   <td> S/s
 *   <tr><td> fshift               <td> Shift frequency                               <td> Hz
 *   <tr><td> fsnr                 <td> Full-scale-to-noise ratio (a.k.a. "SNRFS")    <td> dB
 *   <tr><td> snr                  <td> Signal-to-noise ratio                         <td> dB
 *   <tr><td> sinad                <td> Signal-to-noise-and-distortion ratio          <td> dB
 *   <tr><td> sfdr                 <td> Spurious-free dynamic range                   <td> dB
 *   <tr><td> abn                  <td> Average bin noise                             <td> dBFS
 *   <tr><td> nsd                  <td> Noise spectral density                        <td> dBFS/Hz
 *   <tr><td> carrierindex         <td> Order index of the Carrier tone               <td>
 *   <tr><td> maxspurindex         <td> Order index of the MaxSpur tone               <td>
 *   <tr><td> ab_width             <td> Analysis band width                           <td> Hz
 *   <tr><td> ab_i1                <td> Analysis band first index                     <td>
 *   <tr><td> ab_i2                <td> Analysis band last index                      <td>
 *   <tr><td> {PREFIX}_nbins       <td> Number of bins associated with PREFIX         <td>
 *   <tr><td> {PREFIX}_rss         <td> Root-sum-square associated with PREFIX        <td>
 *   <tr><td> {TONEKEY}:orderindex <td> Tone order index                              <td>
 *   <tr><td> {TONEKEY}:freq       <td> Tone frequency                                <td> Hz
 *   <tr><td> {TONEKEY}:ffinal     <td> Tone final frequency                          <td> Hz
 *   <tr><td> {TONEKEY}:fwavg      <td> Tone weighted-average frequency               <td> Hz
 *   <tr><td> {TONEKEY}:i1         <td> Tone first index                              <td>
 *   <tr><td> {TONEKEY}:i2         <td> Tone last index                               <td>
 *   <tr><td> {TONEKEY}:nbins      <td> Tone number of bins                           <td>
 *   <tr><td> {TONEKEY}:inband     <td> 0: tone is in-band; 1: tone is out-of-band    <td>
 *   <tr><td> {TONEKEY}:mag        <td> Tone magnitude                                <td>
 *   <tr><td> {TONEKEY}:mag_dbfs   <td> Tone magnitude relative to full-scale         <td> dBFS
 *   <tr><td> {TONEKEY}:mag_dbc    <td> Tone magnitude relative to carrier            <td> dBc
 *   <tr><td> {TONEKEY}:phase      <td> Tone phase                                    <td> rad
 *   <tr><td> {TONEKEY}:phase_c    <td> Tone phase relative to carrier                <td> rad
 * </table>
 */
int gn_fft_analysis(
    char** rkeys,                       ///< [out] Result keys array pointer
    size_t rkeys_size,                  ///< [in] Result keys array size
    double* rvalues,                    ///< [out] Result values array pointer
    size_t rvalues_size,                ///< [in] Result values array size
    const char* cfg_id,                 ///< [in] Configuration identifier (filename or object key)
    const double* in,                   ///< [in] Interleaved Re/Im input array pointer
    size_t in_size,                     ///< [in] Input array size
    size_t nfft,                        ///< [in] FFT size
    GnFreqAxisType axis_type           ///< [in] Frequency axis type
    );

/**
 * @brief gn_fft_analysis_select returns select Fourier analysis results
 * @return 0 on success, non-zero otherwise
 */
int gn_fft_analysis_select(
    double* rvalues,                    ///< [out] Result values array pointer
    size_t rvalues_size,                ///< [in] Result values array size
    const char* cfg_id,                 ///< [in] Configuration identifier (filename or object key)
    const char** rkeys,                 ///< [in] Result keys array pointer
    size_t rkeys_size,                  ///< [in] Result keys array size
    const double* in,                   ///< [in] Interleaved Re/Im input array pointer
    size_t in_size,                     ///< [in] Input array size
    size_t nfft,                        ///< [in] FFT size
    GnFreqAxisType axis_type           ///< [in] Frequency axis type
    );

/**
 * @brief gn_fft_analysis_single returns a single Fourier analysis result
 * @return 0 on success, non-zero otherwise
 */
int gn_fft_analysis_single(
    double* rvalue,                     ///< [out] Result value
    const char* cfg_id,                 ///< [in] Configuration identifier (filename or object key)
    const char* rkey,                   ///< [in] Result key
    const double* in,                   ///< [in] Interleaved Re/Im input array pointer
    size_t in_size,                     ///< [in] Input array size
    size_t nfft,                        ///< [in] FFT size
    GnFreqAxisType axis_type           ///< [in] Frequency axis type
    );

/**
 * \defgroup FourierAnalysisConfiguration Configuration
 * @{
 */

/**
 * @brief gn_fa_analysis_band
 * @return 0 on success, non-zero otherwise
 */
int gn_fa_analysis_band(
    const char* obj_key,                ///< [in] Object key
    double center,                      ///< [in] Analysis band center
    double width                        ///< [in] Analysis band width
    );

/**
 * @brief gn_fa_analysis_band_e
 * @return 0 on success, non-zero otherwise
 */
int gn_fa_analysis_band_e(
    const char* obj_key,                ///< [in] Object key
    const char* center,                 ///< [in] Analysis band center expression
    const char* width                   ///< [in] Analysis band width expression
    );

/**
 * @brief gn_fa_clk
 * @return 0 on success, non-zero otherwise
 */
int gn_fa_clk(
    const char* obj_key,                ///< [in] Object key
    const int* clk,                     ///< [in] Pointer to array of clock divisors
    size_t clk_size,                    ///< [in] Size of array of clock divisors
    bool as_noise                       ///< [in] If true, CLK components will be treated as noise
    );

/**
 * @brief gn_fa_conv_offset
 * @return 0 on success, non-zero otherwise
 */
int gn_fa_conv_offset(
    const char* obj_key,                ///< [in] Object key
    bool enable                         ///< [in] If true, enable converter offset
    );

/**
 * @brief gn_fa_create
 * @return 0 on success, non-zero otherwise
 */
int gn_fa_create(
    const char* obj_key                 ///< [in] Object key
    );

/**
 * @brief gn_fa_dc
 * @return 0 on success, non-zero otherwise
 */
int gn_fa_dc(
    const char* obj_key,                ///< [in] Object key
    bool as_dist                        ///< [in] If true, treat DC as distortion
    );

/**
 * @brief gn_fa_fdata
 * @return 0 on success, non-zero otherwise
 */
int gn_fa_fdata(
    const char* obj_key,                ///< [in] Object key
    double f                            ///< [in] fdata
    );

/**
 * @brief gn_fa_fdata_e
 * @return 0 on success, non-zero otherwise
 */
int gn_fa_fdata_e(
    const char* obj_key,                ///< [in] Object key
    const char* f                       ///< [in] fdata expression
    );

/**
 * @brief gn_fa_fixed_tone
 * @return 0 on success, non-zero otherwise
 */
int gn_fa_fixed_tone(
    const char* obj_key,                ///< [in] Object key
    const char* comp_key,               ///< [in] Component key
    GnFACompTag tag,                   ///< [in] Tag
    double freq,                        ///< [in] Frequency
    int ssb                             ///< [in] Number of single-side bins
    );

/**
 * @brief gn_fa_fixed_tone_e
 * @return 0 on success, non-zero otherwise
 */
int gn_fa_fixed_tone_e(
    const char* obj_key,                ///< [in] Object key
    const char* comp_key,               ///< [in] Component key
    GnFACompTag tag,                   ///< [in] Tag
    const char* freq,                   ///< [in] Frequency expression
    int ssb                             ///< [in] Number of single-side bins
    );

/**
 * @brief gn_fa_fsample
 * @return 0 on success, non-zero otherwise
 */
int gn_fa_fsample(
    const char* obj_key,                ///< [in] Object key
    double f                            ///< [in] fsample
    );

/**
 * @brief gn_fa_fsample_e
 * @return 0 on success, non-zero otherwise
 */
int gn_fa_fsample_e(
    const char* obj_key,                ///< [in] Object key
    const char* f                       ///< [in] fsample expression
    );

/**
 * @brief gn_fa_fshift
 * @return 0 on success, non-zero otherwise
 */
int gn_fa_fshift(
    const char* obj_key,                ///< [in] Object key
    double f                            ///< [in] fshift
    );

/**
 * @brief gn_fa_fshift_e
 * @return 0 on success, non-zero otherwise
 */
int gn_fa_fshift_e(
    const char* obj_key,                ///< [in] Object key
    const char* f                       ///< [in] fshift expression
    );

/**
 * @brief gn_fa_fund_images
 * @return 0 on success, non-zero otherwise
 */
int gn_fa_fund_images(
    const char* obj_key,                ///< [in] Object key
    bool enable                         ///< [in] If true, enable fundamental images
    );

/**
 * @brief gn_fa_hd
 * @return 0 on success, non-zero otherwise
 */
int gn_fa_hd(
    const char* obj_key,                ///< [in] Object key
    int n                               ///< [in] Order of harmonic distortion, i.e., the maximum harmonic
    );

/**
 * @brief gn_fa_ilv
 * @return 0 on success, non-zero otherwise
 */
int gn_fa_ilv(
    const char* obj_key,                ///< [in] Object key
    const int* ilv,                     ///< [in] Pointer to array of interleaving factors
    size_t ilv_size,                    ///< [in] Size of array of interleaving factors
    bool as_noise                       ///< [in] If true, ILV components will be treated as noise
    );

/**
 * @brief gn_fa_imd
 * @return 0 on success, non-zero otherwise
 */
int gn_fa_imd(
    const char* obj_key,                ///< [in] Object key
    int n                               ///< [in] Order of intermodulation distortion
    );

/**
 * @brief gn_fa_load
 * @return 0 on success, non-zero otherwise
 * @details If obj_key is empty, the object key is derived from filename.
 */
int gn_fa_load(
    char* buf,                          ///< [out] Pointer to character array
    size_t size,                        ///< [in] Size of character array
    const char* filename,               ///< [in] Filename
    const char* obj_key                 ///< [in] Object key
    );

/**
 * @brief gn_fa_max_tone
 * @return 0 on success, non-zero otherwise
 */
int gn_fa_max_tone(
    const char* obj_key,                ///< [in] Object key
    const char* comp_key,               ///< [in] Component key
    GnFACompTag tag,                   ///< [in] Tag
    int ssb                             ///< [in] Number of single-side bins
    );

/**
 * @brief gn_fa_preview
 * @return 0 on success, non-zero otherwise
 */
int gn_fa_preview(
    char* buf,                          ///< [out] Pointer to character array
    size_t size,                        ///< [in] Size of character array
    const char* cfg_id,                 ///< [in] Configuration identifier (filename or object key)
    bool cplx                           ///< [in] If true, preview will include complex components
    );

/**
 * @brief gn_fa_quad_errors
 * @return 0 on success, non-zero otherwise
 */
int gn_fa_quad_errors(
    const char* obj_key,                ///< [in] Object key
    bool enable                         ///< [in] If true, enable quadrature errors
    );

/**
 * @brief gn_fa_remove_comp
 * @return 0 on success, non-zero otherwise
 */
int gn_fa_remove_comp(
    const char* obj_key,                ///< [in] Object key
    const char* comp_key                ///< [in] Component key
    );

/**
 * @brief gn_fa_reset
 * @return 0 on success, non-zero otherwise
 */
int gn_fa_reset(
    const char* obj_key                 ///< [in] Object key
    );

/**
 * @brief gn_fa_ssb
 * @return 0 on success, non-zero otherwise
 */
int gn_fa_ssb(
    const char* obj_key,                ///< [in] Object key
    GnFASsb group,                     ///< [in] SSB group
    int ssb                             ///< [in] Number of single-side bins
    );

/**
 * @brief gn_fa_var
 * @return 0 on success, non-zero otherwise
 */
int gn_fa_var(
    const char* obj_key,                ///< [in] Object key
    const char* name,                   ///< [in] Variable name
    double value                        ///< [in] Variable value
    );

/**
 * @brief gn_fa_wo
 * @return 0 on success, non-zero otherwise
 */
int gn_fa_wo(
    const char* obj_key,                ///< [in] Object key
    int n                               ///< [in] Number of worst others
    );

/** @} FourierAnalysisConfiguration */

/**
 * \defgroup FourierAnalysisResults Results
 * @{
 */

/**
 * @brief gn_fa_result
 * @return 0 on success, non-zero otherwise
 */
int gn_fa_result(
    double* result,                     ///< [out] Result associated with rkey
    const char** rkeys,                 ///< [in] Result keys array pointer
    size_t rkeys_size,                  ///< [in] Result keys array size
    const double* rvalues,              ///< [in] Result values array pointer
    size_t rvalues_size,                ///< [in] Result values array size
    const char* rkey                    ///< [in] Key of desired result
);

/**
 * @brief gn_fa_result_string
 * @return 0 on success, non-zero otherwise
 */
int gn_fa_result_string(
    char* result,                       ///< [out] Result string associated with rkey
    size_t result_size,                 ///< [in] Size of result string
    const char** rkeys,                 ///< [in] Result keys array pointer
    size_t rkeys_size,                  ///< [in] Result keys array size
    const double* rvalues,              ///< [in] Result values array pointer
    size_t rvalues_size,                ///< [in] Result values array size
    const char* rkey                    ///< [in] Key of desired result
);

/** @} FourierAnalysisResults */

/**
 * \defgroup FourierAnalysisHelpers Helpers
 * @{
 */

/**
 * @brief gn_fa_load_key_size
 * @return 0 on success, non-zero otherwise
 * @details The library string termination setting determines whether or not a null terminator
 * is included in the size.  See \ref gn_set_string_termination.
 */
int gn_fa_load_key_size(
    size_t* size,                       ///< [out] Number of characters in key
    const char* filename,               ///< [in] Filename
    const char* obj_key                 ///< [in] Object key
    );

/**
 * @brief gn_fa_preview_size
 * @return 0 on success, non-zero otherwise
 * @details The library string termination setting determines whether or not a null terminator
 * is included in the size.  See \ref gn_set_string_termination.
 */
int gn_fa_preview_size(
    size_t* size,                       ///< [out] Number of characters in compenent list string
    const char* cfg_id,                 ///< [in] Configuration identifier (filename or object key)
    bool cplx                           ///< [in] If true, list will include complex components
    );

/**
 * @brief gn_fa_result_string_size
 * @return 0 on success, non-zero otherwise
 * @details The library string termination setting determines whether or not a null terminator
 * is included in the size.  See \ref gn_set_string_termination.
 */
int gn_fa_result_string_size(
    size_t* size,                       ///< [out] Number of characters result string
    const char** rkeys,                 ///< [in] Result keys array pointer
    size_t rkeys_size,                  ///< [in] Result keys array size
    const double* rvalues,              ///< [in] Result values array pointer
    size_t rvalues_size,                ///< [in] Result values array size
    const char* rkey                    ///< [in] Key of desired result
    );

/**
 * @brief gn_fft_analysis_results_key_sizes
 * @return 0 on success, non-zero otherwise
 * @details The library string termination setting determines whether or not a null terminator
 * is included in the key sizes.  See \ref gn_set_string_termination.
 */
int gn_fft_analysis_results_key_sizes(
    size_t* key_sizes,                  ///< [out] Key size array pointer
    size_t key_sizes_size,              ///< [in] Key size array size
    const char* cfg_id,                 ///< [in] Configuration identifier (filename or object key)
    size_t in_size,                     ///< [in] Input array size
    size_t nfft                         ///< [in] FFT size
    );

/**
 * @brief gn_fft_analysis_results_size
 * @return 0 on success, non-zero otherwise
 */
int gn_fft_analysis_results_size(
    size_t* size,                       ///< [out] Number of key-value result pairs
    const char* cfg_id,                 ///< [in] Configuration identifier (filename or object key)
    size_t in_size,                     ///< [in] Input array size
    size_t nfft                         ///< [in] FFT size
    );

/** @} FourierAnalysisHelpers */

/** @} FourierAnalysis */

#ifdef __cplusplus
} // extern "C"
#endif

/* Fourier Transforms */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup FourierTransforms Fourier Transforms
 * @{
 */

  /**
 * @brief gn_fft
 * @return 0 on success, non-zero otherwise
 * @param out: [out] Interleaved Re/Im output array pointer
 * @param out_size: [in] Output array size
 * @param i: [in] In-phase input array pointer
 * @param i_size: [in] In-phase input array size
 * @param q: [in] Quadrature input array pointer
 * @param q_size: [in] Quadrature input array size
 * @param navg: [in] FFT averaging number
 * @param nfft: [in] FFT size
 * @param window: [in] Window
 */
int gn_fft(double* out, size_t out_size, const double* i,
                 size_t i_size, const double* q, size_t q_size,
                 size_t navg, size_t nfft, GnWindow window);

/**
 * @brief gn_fft16
 * @return 0 on success, non-zero otherwise
 * @param out: [out] Interleaved Re/Im output array pointer
 * @param out_size: [in] Output array size
 * @param i: [in] In-phase input array pointer
 * @param i_size: [in] In-phase input array size
 * @param q: [in] Quadrature input array pointer
 * @param q_size: [in] Quadrature input array size
 * @param n: [in] Resolution
 * @param navg: [in] FFT averaging number
 * @param nfft: [in] FFT size
 * @param window: [in] Window
 * @param format: [in] Code format
 */
int gn_fft16(double* out, size_t out_size, const int16_t* i,
                size_t i_size, const int16_t* q, size_t q_size,
                int n, size_t navg, size_t nfft, GnWindow window,
                GnCodeFormat format);

/**
 * @brief gn_fft32
 * @return 0 on success, non-zero otherwise
 */
int gn_fft32(
    double* out,                        ///< [out] Interleaved Re/Im output array pointer
    size_t out_size,                    ///< [in] Output array size
    const int32_t* i,                   ///< [in] In-phase input array pointer
    size_t i_size,                      ///< [in] In-phase input array size
    const int32_t* q,                   ///< [in] Quadrature input array pointer
    size_t q_size,                      ///< [in] Quadrature input array size
    int n,                              ///< [in] Resolution
    size_t navg,                        ///< [in] FFT averaging number
    size_t nfft,                        ///< [in] FFT size
    GnWindow window,                   ///< [in] Window
    GnCodeFormat format                ///< [in] Code format
    );

/**
 * @brief gn_fft64
 * @return 0 on success, non-zero otherwise
 */
int gn_fft64(
    double* out,                        ///< [out] Interleaved Re/Im output array pointer
    size_t out_size,                    ///< [in] Output array size
    const int64_t* i,                   ///< [in] In-phase input array pointer
    size_t i_size,                      ///< [in] In-phase input array size
    const int64_t* q,                   ///< [in] Quadrature input array pointer
    size_t q_size,                      ///< [in] Quadrature input array size
    int n,                              ///< [in] Resolution
    size_t navg,                        ///< [in] FFT averaging number
    size_t nfft,                        ///< [in] FFT size
    GnWindow window,                   ///< [in] Window
    GnCodeFormat format                ///< [in] Code format
    );

/**
 * @brief gn_rfft16
 * @return 0 on success, non-zero otherwise
 */
int gn_rfft16(
    double* out,                        ///< [out] Interleaved Re/Im output array pointer
    size_t out_size,                    ///< [in] Output array size
    const int16_t* in,                  ///< [in] Input array pointer
    size_t in_size,                     ///< [in] Input array size
    int n,                              ///< [in] Resolution
    size_t navg,                        ///< [in] FFT averaging number
    size_t nfft,                        ///< [in] FFT size
    GnWindow window,                   ///< [in] Window
    GnCodeFormat format,               ///< [in] Code format
    GnRfftScale scale                  ///< [in] Scaling mode
    );

/**
 * @brief gn_rfft32
 * @return 0 on success, non-zero otherwise
 */
int gn_rfft32(
    double* out,                        ///< [out] Interleaved Re/Im output array pointer
    size_t out_size,                    ///< [in] Output array size
    const int32_t* in,                  ///< [in] Input array pointer
    size_t in_size,                     ///< [in] Input array size
    int n,                              ///< [in] Resolution
    size_t navg,                        ///< [in] FFT averaging number
    size_t nfft,                        ///< [in] FFT size
    GnWindow window,                   ///< [in] Window
    GnCodeFormat format,               ///< [in] Code format
    GnRfftScale scale                  ///< [in] Scaling mode
    );

/**
 * @brief gn_rfft64
 * @return 0 on success, non-zero otherwise
 */
int gn_rfft64(
    double* out,                        ///< [out] Interleaved Re/Im output array pointer
    size_t out_size,                    ///< [in] Output array size
    const int64_t* in,                  ///< [in] Input array pointer
    size_t in_size,                     ///< [in] Input array size
    int n,                              ///< [in] Resolution
    size_t navg,                        ///< [in] FFT averaging number
    size_t nfft,                        ///< [in] FFT size
    GnWindow window,                   ///< [in] Window
    GnCodeFormat format,               ///< [in] Code format
    GnRfftScale scale                  ///< [in] Scaling mode
    );

/**
 * \defgroup FourierTransformHelpers Helpers
 * @{
 */

/**
 * @brief gn_fft_size
 * @return 0 on success, non-zero otherwise
 */
int gn_fft_size(
    size_t* out_size,                   ///< [out] Output array size
    size_t i_size,                      ///< [in] In-phase input array size
    size_t q_size,                      ///< [in] Quadrature input array size
    size_t navg,                        ///< [in] FFT averaging number
    size_t nfft                         ///< [in] FFT size
    );

/**
 * @brief gn_rfft_size
 * @return 0 on success, non-zero otherwise
 */
int gn_rfft_size(
    size_t* out_size,                   ///< [out] Output array size
    size_t in_size,                     ///< [in] Input array size
    size_t navg,                        ///< [in] FFT averaging number
    size_t nfft                         ///< [in] FFT size
    );

/** @} FourierTransformHelpers */

/** @} FourierTransforms */

#ifdef __cplusplus
} // extern "C"
#endif

/* Fourier Utilities */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup FourierUtilities Fourier Utilities
 * @{
 */

/**
 * @brief gn_alias
 * @return 0 on success, non-zero otherwise
 */
int gn_alias(
    double* out,                        ///< [out] Output pointer
    double fs,                          ///< [in] Sample rate (S/s)
    double freq,                        ///< [in] Frequency (Hz)
    GnFreqAxisType axis_type           ///< [in] Frequency axis type
    );

/**
 * @brief gn_coherent
 * @return 0 on success, non-zero otherwise
 */
int gn_coherent(
    double* out,                        ///< [out] Output pointer
    size_t nfft,                        ///< [in] FFT size
    double fs,                          ///< [in] Sample rate (S/s)
    double freq                         ///< [in] Desired frequency (Hz)
    );

/**
 * @brief gn_fftshift
 * @return 0 on success, non-zero otherwise
 */
int gn_fftshift(
    double* out,                        ///< [out] Output array pointer
    size_t out_size,                    ///< [in] Output array size
    const double* in,                   ///< [in] Input array pointer
    size_t in_size                      ///< [in] Input array size
    );

/**
 * @brief gn_freq_axis
 * @return 0 on success, non-zero otherwise
 */
int gn_freq_axis(
    double* out,                        ///< [out] Array pointer
    size_t size,                        ///< [in] Array size
    size_t nfft,                        ///< [in] FFT size
    GnFreqAxisType axis_type,          ///< [in] Frequency axis type
    double fs,                          ///< [in] Sample rate (S/s)
    GnFreqAxisFormat axis_format       ///< [in] Frequency axis format
    );

/**
 * @brief gn_ifftshift
 * @return 0 on success, non-zero otherwise
 */
int gn_ifftshift(
    double* out,                        ///< [out] Output array pointer
    size_t out_size,                    ///< [in] Output array size
    const double* in,                   ///< [in] Input array pointer
    size_t in_size                      ///< [in] Input array size
    );

/**
 * \defgroup FourierUtilityHelpers Helpers
 * @{
 */

/**
 * @brief gn_freq_axis_size
 * @return 0 on success, non-zero otherwise
 */
int gn_freq_axis_size(
    size_t* size,                       ///< [out] Output array size
    size_t nfft,                        ///< [in] FFT size
    GnFreqAxisType axis_type           ///< [in] Frequency axis type
    );

/** @} FourierUtilityHelpers */

/** @} FourierUtilities */

#ifdef __cplusplus
} // extern "C"
#endif

/* Manager */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup Manager Manager
 * @{
 */

/**
 * @brief gn_mgr_clear
 * @return Always returns 0
 */
int gn_mgr_clear();

/**
 * @brief gn_mgr_compare
 * @return 0 on success, non-zero otherwise
 */
int gn_mgr_compare(
    bool* result,                       ///< [out] true if the objects are equal, false otherwise
    const char* obj_key1,               ///< [in] Object key 1
    const char* obj_key2                ///< [in] Object key 2
    );

/**
 * @brief gn_mgr_contains
 * @return Always returns 0
 */
int gn_mgr_contains(
    bool* result,                       ///< [out] true if Manager contains key, false otherwise
    const char* obj_key                 ///< [in] Object key
    );

/**
 * @brief gn_mgr_remove
 * @return Always returns 0
 */
int gn_mgr_remove(
    const char* obj_key                 ///< [in] Object key
    );

/**
 * @brief gn_mgr_save
 * @return 0 on success, non-zero otherwise
 * @details If filename is empty, the filename is derived from obj_key.
 */
int gn_mgr_save(
    char* buf,                          ///< [out] Pointer to character array
    size_t size,                        ///< [in] Size of character array
    const char* obj_key,                ///< [in] Object key
    const char* filename                ///< [in] Filename
    );

/**
 * @brief gn_mgr_size
 * @return Always returns 0
 */
int gn_mgr_size(
    size_t* size                        ///< [out] Number of objects owned by the manager
    );

/**
 * @brief gn_mgr_to_string
 * @return 0 on success, non-zero otherwise
 */
int gn_mgr_to_string(
    char* buf,                          ///< [out] Pointer to character array
    size_t size,                        ///< [in] Size of character array
    const char* obj_key                 ///< [in] Object key
    );

/**
 * @brief gn_mgr_type
 * @return 0 on success, non-zero otherwise
 */
int gn_mgr_type(
    char* buf,                          ///< [out] Pointer to character array
    size_t size,                        ///< [in] Size of character array
    const char* obj_key                 ///< [in] Object key
    );

/**
 * \defgroup ManagerHelpers Helpers
 * @{
 */

/**
 * @brief gn_mgr_save_filename_size
 * @return 0 on success, non-zero otherwise
 * @details The library string termination setting determines whether or not a null terminator
 * is included in the size.  See \ref gn_set_string_termination.
 */
int gn_mgr_save_filename_size(
    size_t* size,                       ///< [out] Number of characters in filename
    const char* obj_key,                ///< [in] Object key
    const char* filename                ///< [in] Filename
    );

/**
 * @brief gn_mgr_to_string_size
 * @return 0 on success, non-zero otherwise
 * @details The library string termination setting determines whether or not a null terminator
 * is included in the size.  See \ref gn_set_string_termination.
 */
int gn_mgr_to_string_size(
    size_t* size,                       ///< [out] Number of characters in the string
    const char* obj_key                 ///< [in] Object key
    );

/**
 * @brief gn_mgr_type_size
 * @return 0 on success, non-zero otherwise
 * @details The library string termination setting determines whether or not a null terminator
 * is included in the size.  See \ref gn_set_string_termination.
 */
int gn_mgr_type_size(
    size_t* size,                       ///< [out] Number of characters in object type string
    const char* obj_key                 ///< [in] Object key
    );

/** @} ManagerHelpers */

/** @} Manager */

#ifdef __cplusplus
} // extern "C"
#endif

/* Signal Processing */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup SignalProcessing Signal Processing
 * @{
 */

/**
 * @brief gn_downsample
 * @return 0 on success, non-zero otherwise
 */
int gn_downsample(
    double* out,                        ///< [out] Output array pointer
    size_t out_size,                    ///< [in] Output array size
    const double* in,                   ///< [in] Input array pointer
    size_t in_size,                     ///< [in] Input array size
    int ratio,                          ///< [in] Downsample ratio
    bool interleaved                    ///< [in] If true, 'in' is interleaved I/Q data
    );

/**
 * @brief gn_downsample16
 * @return 0 on success, non-zero otherwise
 */
int gn_downsample16(
    int16_t* out,                       ///< [out] Output array pointer
    size_t out_size,                    ///< [in] Output array size
    const int16_t* in,                  ///< [in] Input array pointer
    size_t in_size,                     ///< [in] Input array size
    int ratio,                          ///< [in] Downsample ratio
    bool interleaved                    ///< [in] If true, 'in' is interleaved I/Q data
    );

/**
 * @brief gn_downsample32
 * @return 0 on success, non-zero otherwise
 */
int gn_downsample32(
    int32_t* out,                       ///< [out] Output array pointer
    size_t out_size,                    ///< [in] Output array size
    const int32_t* in,                  ///< [in] Input array pointer
    size_t in_size,                     ///< [in] Input array size
    int ratio,                          ///< [in] Downsample ratio
    bool interleaved                    ///< [in] If true, 'in' is interleaved I/Q data
    );

/**
 * @brief gn_downsample64
 * @return 0 on success, non-zero otherwise
 */
int gn_downsample64(
    int64_t* out,                       ///< [out] Output array pointer
    size_t out_size,                    ///< [in] Output array size
    const int64_t* in,                  ///< [in] Input array pointer
    size_t in_size,                     ///< [in] Input array size
    int ratio,                          ///< [in] Downsample ratio
    bool interleaved                    ///< [in] If true, 'in' is interleaved I/Q data
    );

/**
 * @brief gn_fshift
 * @return 0 on success, non-zero otherwise
 */
int gn_fshift(
    double* out,                        ///< [out] Output array pointer
    size_t out_size,                    ///< [in] Output array size
    const double* i,                    ///< [in] In-phase input array pointer
    size_t i_size,                      ///< [in] In-phase input array size
    const double* q,                    ///< [in] Quadrature input array pointer
    size_t q_size,                      ///< [in] Quadrature input array size
    double fs,                          ///< [in] Sample rate
    double fshift                       ///< [in] Shift frequency
    );

/**
 * @brief gn_fshift16
 * @return 0 on success, non-zero otherwise
 */
int gn_fshift16(
    int16_t* out,                       ///< [out] Output array pointer
    size_t out_size,                    ///< [in] Output array size
    const int16_t* i,                   ///< [in] In-phase input array pointer
    size_t i_size,                      ///< [in] In-phase input array size
    const int16_t* q,                   ///< [in] Quadrature input array pointer
    size_t q_size,                      ///< [in] Quadrature input array size
    int n,                              ///< [in] Code width
    double fs,                          ///< [in] Sample rate
    double fshift,                      ///< [in] Shift frequency
    GnCodeFormat format                ///< [in] Code format
    );

/**
 * @brief gn_fshift32
 * @return 0 on success, non-zero otherwise
 */
int gn_fshift32(
    int32_t* out,                       ///< [out] Output array pointer
    size_t out_size,                    ///< [in] Output array size
    const int32_t* i,                   ///< [in] In-phase input array pointer
    size_t i_size,                      ///< [in] In-phase input array size
    const int32_t* q,                   ///< [in] Quadrature input array pointer
    size_t q_size,                      ///< [in] Quadrature input array size
    int n,                              ///< [in] Code width
    double fs,                          ///< [in] Sample rate
    double fshift,                      ///< [in] Shift frequency
    GnCodeFormat format                ///< [in] Code format
    );

/**
 * @brief gn_fshift64
 * @return 0 on success, non-zero otherwise
 */
int gn_fshift64(
    int64_t* out,                       ///< [out] Output array pointer
    size_t out_size,                    ///< [in] Output array size
    const int64_t* i,                   ///< [in] In-phase input array pointer
    size_t i_size,                      ///< [in] In-phase input array size
    const int64_t* q,                   ///< [in] Quadrature input array pointer
    size_t q_size,                      ///< [in] Quadrature input array size
    int n,                              ///< [in] Code width
    double fs,                          ///< [in] Sample rate
    double fshift,                      ///< [in] Shift frequency
    GnCodeFormat format                ///< [in] Code format
    );

/**
 * @brief gn_normalize16
 * @return 0 on success, non-zero otherwise
 */
int gn_normalize16(
    double* out,                        ///< [out] Output array pointer
    size_t out_size,                    ///< [in] Output array size
    const int16_t* in,                  ///< [in] Input array pointer
    size_t in_size,                     ///< [in] Input array size
    int n,                              ///< [in] Resolution
    GnCodeFormat format                ///< [in] Code format
    );

/**
 * @brief gn_normalize32
 * @return 0 on success, non-zero otherwise
 */
int gn_normalize32(
    double* out,                        ///< [out] Output array pointer
    size_t out_size,                    ///< [in] Output array size
    const int32_t* in,                  ///< [in] Input array pointer
    size_t in_size,                     ///< [in] Input array size
    int n,                              ///< [in] Resolution
    GnCodeFormat format                ///< [in] Code format
    );

/**
 * @brief gn_normalize64
 * @return 0 on success, non-zero otherwise
 */
int gn_normalize64(
    double* out,                        ///< [out] Output array pointer
    size_t out_size,                    ///< [in] Output array size
    const int64_t* in,                  ///< [in] Input array pointer
    size_t in_size,                     ///< [in] Input array size
    int n,                              ///< [in] Resolution
    GnCodeFormat format                ///< [in] Code format
    );

/**
 * @brief gn_polyval
 * @return 0 on success, non-zero otherwise
 */
int gn_polyval(
    double* out,                        ///< [out] Output array pointer
    size_t out_size,                    ///< [in] Output array size
    const double* in,                   ///< [in] Input array pointer
    size_t in_size,                     ///< [in] Input array size
    const double* c,                    ///< [in] Coefficient array pointer
    size_t c_size                       ///< [in] Coefficient array size
    );

/**
 * @brief gn_quantize16
 * @return 0 on success, non-zero otherwise
 */
int gn_quantize16(
    int16_t* out,                       ///< [out] Output array pointer
    size_t out_size,                    ///< [in] Output array size
    const double* in,                   ///< [in] Input array pointer
    size_t in_size,                     ///< [in] Input array size
    double fsr,                         ///< [in] Full-scale range
    int n,                              ///< [in] Resolution
    double noise,                       ///< [in] Input referred RMS noise
    GnCodeFormat format                ///< [in] Code format
);

/**
 * @brief gn_quantize32
 * @return 0 on success, non-zero otherwise
 */
int gn_quantize32(
    int32_t* out,                       ///< [out] Output array pointer
    size_t out_size,                    ///< [in] Output array size
    const double* in,                   ///< [in] Input array pointer
    size_t in_size,                     ///< [in] Input array size
    double fsr,                         ///< [in] Full-scale range
    int n,                              ///< [in] Resolution
    double noise,                       ///< [in] Input referred RMS noise
    GnCodeFormat format                ///< [in] Code format
);

/**
 * @brief gn_quantize64
 * @return 0 on success, non-zero otherwise
 */
int gn_quantize64(
    int64_t* out,                       ///< [out] Output array pointer
    size_t out_size,                    ///< [in] Output array size
    const double* in,                   ///< [in] Input array pointer
    size_t in_size,                     ///< [in] Input array size
    double fsr,                         ///< [in] Full-scale range
    int n,                              ///< [in] Resolution
    double noise,                       ///< [in] Input referred RMS noise
    GnCodeFormat format                ///< [in] Code format
);

/**
 * \defgroup SignalProcessingHelpers Helpers
 * @{
 */

/**
 * @brief gn_downsample_size
 * @return 0 on success, non-zero otherwise
 */
int gn_downsample_size(
    size_t* out_size,                   ///< [out] Output array size
    size_t in_size,                     ///< [in] Input array size
    int ratio,                          ///< [in] Downsample ratio
    bool interleaved                    ///< [in] If bool, 'in' is interleaved I/Q data
    );

/**
 * @brief gn_fshift_size
 * @return 0 on success, non-zero otherwise
 */
int gn_fshift_size(
    size_t* out_size,                   ///< [out] Output array size
    size_t i_size,                      ///< [in] In-phase input array size
    size_t q_size                       ///< [in] Quadrature input array size
    );

/** @} SignalProcessingHelpers */

/** @} SignalProcessing */

#ifdef __cplusplus
} // extern "C"
#endif

/* Waveforms */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup Waveforms Waveforms
 * @{
 */

/**
 * @brief gn_cos
 * @return 0 on success, non-zero otherwise
 */
int gn_cos(
    double* out,                        ///< [out] Array pointer
    size_t size,                        ///< [in] Array size
    double fs,                          ///< [in] Sample rate (S/s)
    double ampl,                        ///< [in] Amplitude
    double freq,                        ///< [in] Frequency (Hz)
    double phase,                       ///< [in] Phase (rad)
    double td,                          ///< [in] Time delay (s)
    double tj                           ///< [in] RMS Aperture jitter (s)
    );

/**
 * @brief gn_gaussian
 * @return 0 on success, non-zero otherwise
 */
int gn_gaussian(
    double* out,                        ///< [out] Array pointer
    size_t size,                        ///< [in] Array size
    double mean,                        ///< [in] Mean
    double sd                           ///< [in] Standard deviation
    );

/**
 * @brief gn_ramp
 * @return 0 on success, non-zero otherwise
 */
int gn_ramp(
    double* out,                        ///< [out] Array pointer
    size_t size,                        ///< [in] Array size
    double start,                       ///< [in] Start value
    double stop,                        ///< [in] Stop value
    double noise                        ///< [in] RMS noise
    );

/**
 * @brief gn_sin
 * @return 0 on success, non-zero otherwise
 */
int gn_sin(
    double* out,                        ///< [out] Array pointer
    size_t size,                        ///< [in] Array size
    double fs,                          ///< [in] Sample rate (S/s)
    double ampl,                        ///< [in] Amplitude
    double freq,                        ///< [in] Frequency (Hz)
    double phase,                       ///< [in] Phase (rad)
    double td,                          ///< [in] Time delay (s)
    double tjrms                        ///< [in] RMS Aperture jitter (s)
    );

/**
 * @brief gn_wf_analysis
 * @return 0 on success, non-zero otherwise
 * @details The results contain the following key-value pairs (see general description of
 * \ref AnalysisRoutines "Analysis Routines").
 * <table>
 *   <tr><th> Key       <th> Description
 *   <tr><td> min       <td> Minumum value
 *   <tr><td> max       <td> Maximum value
 *   <tr><td> mid       <td> Middle value ((max + min) / 2)
 *   <tr><td> range     <td> Range (max - min)
 *   <tr><td> avg       <td> Average value
 *   <tr><td> rms       <td> RMS value
 *   <tr><td> rmsac     <td> RMS value with DC removed
 *   <tr><td> min_index <td> Index of first occurence of minimum value
 *   <tr><td> max_index <td> Index of first occurence of maximum value
 * </table>
 */
int gn_wf_analysis(
    char** rkeys,                       ///< [out] Result keys array pointer
    size_t rkeys_size,                  ///< [in] Result keys array size
    double* rvalues,                    ///< [out] Result values array pointer
    size_t rvalues_size,                ///< [in] Result values array size
    const double* in,                   ///< [in] Waveform array pointer
    size_t in_size                      ///< [in] Waveform array size
    );

/**
 * @brief gn_wf_analysis16
 * @return 0 on success, non-zero otherwise
 * @details See description of \ref gn_wf_analysis.
 */
int gn_wf_analysis16(
    char** rkeys,                       ///< [out] Result keys array pointer
    size_t rkeys_size,                  ///< [in] Result keys array size
    double* rvalues,                    ///< [out] Result values array pointer
    size_t rvalues_size,                ///< [in] Result values array size
    const int16_t* in,                  ///< [in] Waveform array pointer
    size_t in_size                      ///< [in] Waveform array size
    );

/**
 * @brief gn_wf_analysis32
 * @return 0 on success, non-zero otherwise
 * @details See description of \ref gn_wf_analysis.
 */
int gn_wf_analysis32(
    char** rkeys,                       ///< [out] Result keys array pointer
    size_t rkeys_size,                  ///< [in] Result keys array size
    double* rvalues,                    ///< [out] Result values array pointer
    size_t rvalues_size,                ///< [in] Result values array size
    const int32_t* in,                  ///< [in] Waveform array pointer
    size_t in_size                      ///< [in] Waveform array size
    );

/**
 * @brief gn_wf_analysis64
 * @return 0 on success, non-zero otherwise
 * @details See description of \ref gn_wf_analysis.
 */
int gn_wf_analysis64(
    char** rkeys,                       ///< [out] Result keys array pointer
    size_t rkeys_size,                  ///< [in] Result keys array size
    double* rvalues,                    ///< [out] Result values array pointer
    size_t rvalues_size,                ///< [in] Result values array size
    const int64_t* in,                  ///< [in] Waveform array pointer
    size_t in_size                      ///< [in] Waveform array size
    );

/** @} Waveforms */

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CGENALYZER_ADVANCED2_H