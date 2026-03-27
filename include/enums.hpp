// Copyright (C) 2024-2026 Analog Devices, Inc.
//
// SPDX short identifier: ADIBSD OR GPL-2.0-or-later
#ifndef GENALYZER_IMPL_ENUMS_HPP
#define GENALYZER_IMPL_ENUMS_HPP

/**
 * @brief Public enumerations for the genalyzer library.
 */
namespace genalyzer_impl {

/** @brief Type of analysis performed. */
enum class AnalysisType : int {
	DNL, /**< Differential nonlinearity analysis. */
	Fourier, /**< Fourier (spectral) analysis. */
	Histogram, /**< Histogram (code density) analysis. */
	INL, /**< Integral nonlinearity analysis. */
	Waveform /**< Time-domain waveform analysis. */
};

/** @brief ADC code format. */
enum class CodeFormat : int {
	OffsetBinary, /**< Unsigned offset binary encoding. */
	TwosComplement /**< Signed two's complement encoding. */
};

/** @brief Signal type used for DNL computation. */
enum class DnlSignal : int {
	Ramp, /**< Linear ramp stimulus. */
	Tone /**< Sinusoidal (tone) stimulus. */
};

/**
 * @brief Fourier analysis component classification tag.
 *
 * Determines how a spectral component is categorized when computing
 * aggregate metrics. Components not tagged as DC, Signal, or a distortion
 * type are treated as noise.
 */
enum class FACompTag : int {
	DC, /**< DC component (always Bin 0). */
	Signal, /**< Signal component. */
	HD, /**< Harmonic distortion. */
	IMD, /**< Intermodulation distortion. */
	ILOS, /**< Interleaving offset component. */
	ILGT, /**< Interleaving gain/timing/BW component. */
	CLK, /**< Clock sub-harmonic component. */
	UserDist, /**< User-designated distortion component. */
	Noise /**< Noise component (e.g., WorstOther). */
};

/**
 * @brief Fourier analysis single-side-bin group selector.
 *
 * Controls the number of FFT bins assigned to each side of a tone's center
 * bin for different component groups.
 */
enum class FASsb : int {
	Default, /**< Default SSB (applies to auto-generated components). */
	DC, /**< SSB for the DC component. */
	Signal, /**< SSB for signal components. */
	WO /**< SSB for worst-other components. */
};

/** @brief Output format for frequency axis values. */
enum class FreqAxisFormat : int {
	Bins, /**< Frequency expressed as FFT bin indices. */
	Freq, /**< Frequency expressed in Hz. */
	Norm /**< Normalized frequency (relative to sample rate). */
};

/** @brief Frequency axis layout type. */
enum class FreqAxisType : int {
	DcCenter, /**< DC at center: [-fs/2, fs/2). */
	DcLeft, /**< DC at left: [0, fs). */
	Real /**< Real (one-sided): [0, fs/2]. */
};

/** @brief Line-fit method for INL computation. */
enum class InlLineFit : int {
	BestFit, /**< Remove a least-squares best-fit line. */
	EndFit, /**< Remove an endpoint-fit line. */
	NoFit /**< Do not remove any line (raw cumulative sum). */
};

/**
 * @brief dBFS scaling convention for real FFT output.
 *
 * Controls how full-scale is defined for dBFS measurements.
 */
enum class RfftScale : int {
	DbfsDc, /**< Full-scale sinusoid measures -3 dBFS. */
	DbfsSin, /**< Full-scale sinusoid measures 0 dBFS. */
	Native /**< Full-scale sinusoid measures -6 dBFS. */
};

/** @brief Window function for FFT computation. */
enum class Window : int {
	BlackmanHarris, /**< Blackman-Harris window. */
	Hann, /**< Hann window. */
	NoWindow /**< No window (rectangular). */
};

} // namespace genalyzer_impl

/*
 * Enumerations only used internally
 */
namespace genalyzer_impl {

enum class FACompType : int { // Fourier Analysis Component Type
	DC, // DC component (always Bin 0)
	FixedTone, // Tone with fixed, user-defined location
	MaxTone, // Next largest tone
	WOTone // Worst other tone
};

enum class FAResult : int { // Fourier Analysis Results
	// Meta Data
	AnalysisType = 0, // Analysis type
	SignalType, // Signal type: 0=Real, 1=Cplx
	NFFT, // FFT size
	DataSize, // Data size
	FBin, // Frequency resolution (Hz)
	FData, // Data rate (S/s)
	FSample, // Sample rate (S/s)
	FShift, // Shift frequency (Hz)
	// Primary Measurements
	FSNR, // Full-Scale-to-Noise ratio, a.k.a "SNRFS" (dB)
	SNR, // Signal-to-Noise ratio (dB)
	SINAD, // Signal-to-Noise-and-Distortion ratio (dB)
	SFDR, // Spurious-Free Dynamic Range (dB)
	ABN, // Average Bin Noise (dBFS)
	NSD, // Noise Spectral Density (dBFS/Hz)
	// Carrier and MaxSpur
	CarrierIndex, // Order index of Carrier tone
	MaxSpurIndex, // Order index of MaxSpur tone
	// Analysis Band Info
	AB_Width, // Width (Hz)
	AB_I1, // Index 1
	AB_I2, // Index 2
	AB_NBins, // Number of bins
	AB_RSS, // Root-sum-square
	// In-Band Tag Info
	Signal_NBins,
	Signal_RSS,
	CLK_NBins,
	CLK_RSS,
	HD_NBins,
	HD_RSS,
	ILOS_NBins,
	ILOS_RSS,
	ILGT_NBins,
	ILGT_RSS,
	IMD_NBins,
	IMD_RSS,
	UserDist_NBins,
	UserDist_RSS,
	// In-Band Composite Info
	THD_NBins, // HD + IMD (total harmonic distortion)
	THD_RSS,
	ILV_NBins, // ILOS + ILGT (total interleaving)
	ILV_RSS,
	Dist_NBins, // Distortion
	Dist_RSS,
	Noise_NBins, // Noise
	Noise_RSS,
	NAD_NBins, // Noise + Distortion
	NAD_RSS,
	//
	__SIZE__
};

enum class FAToneResult : int { // Fourier Analysis Tone Results
	OrderIndex = 0,
	Tag,
	Freq,
	FFinal,
	FWAvg,
	I1,
	I2,
	NBins,
	InBand,
	Mag,
	Mag_dBFS,
	Mag_dBc,
	Phase,
	Phase_c,
	//
	__SIZE__
};

enum class FPFormat { // Floating-point format
	Auto,
	Eng,
	Fix,
	Sci
};

enum class ObjectType { FourierAnalysis };

} // namespace genalyzer_impl

#endif // GENALYZER_IMPL_ENUMS_HPP