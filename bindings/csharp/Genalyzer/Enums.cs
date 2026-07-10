// Copyright (C) 2024-2025 Analog Devices, Inc.
//
// SPDX short identifier: ADIBSD OR GPL-2.0-or-later

namespace Genalyzer
{
    /// <summary>Enumerates analysis types.</summary>
    public enum AnalysisType
    {
        DNL       = 0,
        Fourier   = 1,
        Histogram = 2,
        INL       = 3,
        Waveform  = 4
    }

    /// <summary>Enumerates binary code formats.</summary>
    public enum CodeFormat
    {
        /// <summary>Offset Binary</summary>
        OffsetBinary    = 0,
        /// <summary>Two's Complement</summary>
        TwosComplement  = 1
    }

    /// <summary>Enumerates signal types for which DNL can be computed.</summary>
    public enum DnlSignal
    {
        /// <summary>Ramp</summary>
        Ramp = 0,
        /// <summary>Tone (Sinusoid)</summary>
        Tone = 1
    }

    /// <summary>Enumerates Fourier analysis component tags.</summary>
    public enum FACompTag
    {
        /// <summary>DC component (always Bin 0)</summary>
        DC       = 0,
        /// <summary>Signal component</summary>
        Signal   = 1,
        /// <summary>Harmonic distortion</summary>
        HD       = 2,
        /// <summary>Intermodulation distortion</summary>
        IMD      = 3,
        /// <summary>Interleaving offset component</summary>
        ILOS     = 4,
        /// <summary>Interleaving gain/timing/BW component</summary>
        ILGT     = 5,
        /// <summary>Clock component</summary>
        CLK      = 6,
        /// <summary>User-designated distortion</summary>
        UserDist = 7,
        /// <summary>Noise component (e.g. WorstOther)</summary>
        Noise    = 8
    }

    /// <summary>
    /// Enumerates the component categories for which the number of
    /// single side bins (SSB) can be set.
    /// </summary>
    public enum FASsb
    {
        /// <summary>Default SSB (applies to auto-generated components)</summary>
        Default = 0,
        /// <summary>SSB for DC component</summary>
        DC      = 1,
        /// <summary>SSB for Signal components</summary>
        Signal  = 2,
        /// <summary>SSB for WorstOther components</summary>
        WO      = 3
    }

    /// <summary>Enumerates frequency axis formats.</summary>
    public enum FreqAxisFormat
    {
        /// <summary>Bins</summary>
        Bins = 0,
        /// <summary>Frequency</summary>
        Freq = 1,
        /// <summary>Normalized</summary>
        Norm = 2
    }

    /// <summary>Enumerates frequency axis types.</summary>
    public enum FreqAxisType
    {
        /// <summary>DC centered, e.g. [-fs/2, fs/2) (complex FFT only)</summary>
        DcCenter = 0,
        /// <summary>DC on left, e.g. [0, fs) (complex FFT only)</summary>
        DcLeft   = 1,
        /// <summary>Real axis, e.g. [0, fs/2] (real FFT only)</summary>
        Real     = 2
    }

    /// <summary>Enumerates INL line fitting options.</summary>
    public enum InlLineFit
    {
        /// <summary>Best fit</summary>
        BestFit = 0,
        /// <summary>End fit</summary>
        EndFit  = 1,
        /// <summary>No fit</summary>
        NoFit   = 2
    }

    /// <summary>Enumerates real FFT scaling options.</summary>
    public enum RfftScale
    {
        /// <summary>Full-scale sinusoid measures -3 dBFS</summary>
        DbfsDc  = 0,
        /// <summary>Full-scale sinusoid measures 0 dBFS</summary>
        DbfsSin = 1,
        /// <summary>Full-scale sinusoid measures -6 dBFS</summary>
        Native  = 2
    }

    /// <summary>Enumerates window functions.</summary>
    public enum Window
    {
        /// <summary>Blackman-Harris</summary>
        BlackmanHarris = 0,
        /// <summary>Hann ("Hanning")</summary>
        Hann           = 1,
        /// <summary>No window (Rectangular)</summary>
        NoWindow       = 2
    }

    /// <summary>Enumerates tone types for simplified-beta waveform generation.</summary>
    public enum ToneType
    {
        RealCosine  = 0,
        RealSine    = 1,
        ComplexExp  = 2
    }
}
