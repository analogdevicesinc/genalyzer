/*
File        : $HeadURL: https://swdev.cld.analog.com/svn/projects/icdev/sandbox/users/pderouni/platform-upgrades/src/analysis/enum_maps.hpp $
Originator  : pderouni
Revision    : $Revision: 12032 $
Last Commit : $Date: 2019-11-08 08:01:21 -0500 (Fri, 08 Nov 2019) $
Last Editor : $Author: pderouni $
*/

#ifndef ICD_ANALYSIS_ENUM_MAPS_HPP
#define ICD_ANALYSIS_ENUM_MAPS_HPP

#include "enum_map.hpp"
#include "enums.hpp"

namespace analysis {

const enum_map<AnalysisType> analysis_type_map(
    "AnalysisType",
    { { AnalysisType::FFT, "FFT" },
        { AnalysisType::Waveform, "Waveform" } });

const enum_map<CodeFormat> code_format_map(
    "CodeFormat",
    { { CodeFormat::OffsetBinary, "OffsetBinary" },
        { CodeFormat::TwosComplement, "TwosComplement" } });

const enum_map<FFTAxisFormat> fft_axis_format_map(
    "FFTAxisFormat",
    { { FFTAxisFormat::Bin, "Bin" },
        { FFTAxisFormat::Freq, "Freq" },
        { FFTAxisFormat::Norm, "Norm" } });

const enum_map<FFTCompTag> fft_comp_tag_map(
    "FFTCompTag",
    { { FFTCompTag::Signal, "Signal" },
        { FFTCompTag::DC, "DC" },
        { FFTCompTag::HD, "HD" },
        { FFTCompTag::IMD, "IMD" },
        { FFTCompTag::Dist, "Dist" },
        { FFTCompTag::ILOS, "ILOS" },
        { FFTCompTag::ILGT, "ILGT" },
        { FFTCompTag::CLK, "CLK" },
        { FFTCompTag::Noise, "Noise" } });

const enum_map<FFTCompType> fft_comp_type_map(
    "FFTCompType",
    { { FFTCompType::Band, "Band" },
        { FFTCompType::FixedTone, "FixedTone" },
        { FFTCompType::MaxTone, "MaxTone" } });

const enum_map<ObjectType> object_type_map(
    "ObjectType",
    { { ObjectType::FFTAnalysis, "FFTAnalysis" },
        { ObjectType::QuantArch, "QuantArch" },
        { ObjectType::QuantModel, "QuantModel" },
        { ObjectType::VarItem, "VarItem" },
        { ObjectType::VarMap, "VarMap" },
        { ObjectType::VarVector, "VarVector" } });

const enum_map<PmfType> pmf_type_map(
    "PmfType",
    { { PmfType::Ramp, "Ramp" },
        { PmfType::Tone, "Tone" } });

const enum_map<WindowType> window_type_map(
    "WindowType",
    { { WindowType::BlackmanHarris, "BlackmanHarris" },
        { WindowType::Hann, "Hann" },
        { WindowType::Rect, "Rect" } });

} // namespace analysis

#endif // ICD_ANALYSIS_ENUM_MAPS_HPP
