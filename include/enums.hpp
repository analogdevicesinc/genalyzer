// Copyright (C) 2024 Analog Devices, Inc.
//
// SPDX short identifier: ADIBSD OR GPL-2.0-or-later
#ifndef GENALYZER_IMPL_ENUMS_HPP
#define GENALYZER_IMPL_ENUMS_HPP

/*
 * Enumerations that are part of the API (exposed as type int)
 */
namespace genalyzer_impl {

enum class AnalysisType : int { DNL,
	Fourier,
	Histogram,
	INL,
	Waveform };

enum class CodeFormat : int { OffsetBinary,
	TwosComplement };

enum class DnlSignal : int { Ramp,
	Tone };

// Noise means not DC and not Signal and not Distortion
enum class FACompTag : int { // Fourier Analysis Component Tag
	DC, // DC component (always Bin 0)
	Signal, // Signal component
	HD, // Harmonic distortion
	IMD, // Intermodulation distortion
	ILOS, // Interleaving offset component
	ILGT, // Interleaving gain/timing/BW component
	CLK, // Clock component
	UserDist, // User-designated distortion
	Noise // Noise component (e.g. WorstOther)
};

enum class FASsb : int {
	Default, // Default SSB (applies to auto-generated components)
	DC, // SSB for DC component
	Signal, // SSB for Signal components
	WO, // SSB for WorstOther components
};

enum class FreqAxisFormat : int { Bins,
	Freq,
	Norm };

enum class FreqAxisType : int { DcCenter,
	DcLeft,
	Real };

enum class InlLineFit : int { BestFit,
	EndFit,
	NoFit };

enum class RfftScale : int {
	DbfsDc, // Full-scale sinusoid measures -3 dBFS
	DbfsSin, // Full-scale sinusoid measures  0 dBFS
	Native // Full-scale sinusoid measures -6 dBFS
};

enum class Window : int { BlackmanHarris,
	Hann,
	NoWindow };

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