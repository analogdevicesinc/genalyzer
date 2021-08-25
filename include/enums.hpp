/*
File        : $HeadURL: https://swdev.cld.analog.com/svn/projects/icdev/sandbox/users/pderouni/platform-upgrades/src/analysis/enums.hpp $
Originator  : pderouni
Revision    : $Revision: 12078 $
Last Commit : $Date: 2019-11-20 10:59:23 -0500 (Wed, 20 Nov 2019) $
Last Editor : $Author: pderouni $
*/

#ifndef ICD_ANALYSIS_ENUMS_HPP
#define ICD_ANALYSIS_ENUMS_HPP

namespace analysis {

/// @{

/// @ingroup Enumerations

/**
     * @brief The AnalysisType enum class
     * @anchor AnalysisType
     */
enum class AnalysisType : int {
    FFT, ///< FFT
    Waveform ///< Waveform
};

/**
     * @brief The CodeFormat enum class
     */
enum class CodeFormat : int {
    OffsetBinary, ///< Offset Binary
    TwosComplement ///< Two's Complement
};

/**
     * @brief FFTAxisFormat enumerates FFT axis formats
     */
enum class FFTAxisFormat : int {
    Bin, ///< Bin (Cycles)
    Freq, ///< Frequency
    Norm ///< Normalized
};

/**
     * @brief FFTCompTag enumerates ways components are used in calculations
     * @anchor FFTCompTag
     */
enum class FFTCompTag : int {
    Signal, ///< Signal component
    DC, ///< DC component; always bin zero
    HD, ///< Harmonic distortion
    IMD, ///< Intermodulation distortion
    Dist, ///< Unspecified distortion (i.e. not Noise)
    ILOS, ///< Interleaving offset component
    ILGT, ///< Interleaving gain/timing/BW component
    CLK, ///< Clock component
    Noise ///< Noise, unless otherwise assigned
};

/**
     * @brief FFTCompType enumerates methods used to locate FFT components
     * @anchor FFTCompType
     */
enum class FFTCompType : int {
    Band, ///< Band
    FixedTone, ///< Tone at specified location
    MaxTone ///< Next largest tone within specified band
};

/**
     * @brief The FileType enum class
     */
enum class FileType : int {
    Bin, ///< Binary
    Json, ///< JSON
    Xml, ///< XML
    Yaff ///< Yet Another File Format (experimental; do not use)
};

/**
     * @brief The ObjectType enum class
     */
enum class ObjectType : int {
    FFTAnalysis, ///< FFT Analysis
    QuantArch, ///< Quantizer Architecture
    QuantModel, ///< Quantizer Model
    VarItem, ///< Var Item
    VarMap, ///< Var Map
    VarVector ///< Var Vector
};

/**
     * @brief The PmfType enum class represents the probability mass function
     *        of different types of well-defined discrete-valued signals
     */
enum class PmfType : int {
    Ramp, ///< Ramp
    Tone ///< Tone
};

/**
     * @brief RealFormat enumerates real number formats
     */
enum class RealFormat : int {
    Auto, ///< Automatic
    Eng, ///< Engineering
    Fix, ///< Fixed Precision
    Sci ///< Scientific
};

/**
     * @brief The VarType enum class
     */
enum class VarType : int {
    Bool, ///< Boolean
    Int, ///< Integer
    Real, ///< Real number
    Cplx, ///< Complex number
    Str, ///< String
    Map, ///< Map
    Vector ///< Vector
};

/**
     * @brief The WindowFunction enum class represents different types of
     *        windowing functions
     */
enum class WindowType : int {
    BlackmanHarris, ///< Blackman-Harris
    Hann, ///< Hann
    Rect ///< Rectangular
};

/// @}

} // namespace analysis

#endif // ICD_ANALYSIS_ENUMS_HPP
