// Copyright (C) 2024 Analog Devices, Inc.
//
// SPDX short identifier: ADIBSD OR GPL-2.0-or-later
#include "enum_maps.hpp"

#include <functional>

namespace genalyzer_impl {

const std::map<str_t, std::reference_wrapper<const enum_map>> enumeration_map = {
	{ analysis_type_map.name(), std::cref(analysis_type_map) },
	{ code_format_map.name(), std::cref(code_format_map) },
	{ dnl_signal_map.name(), std::cref(dnl_signal_map) },
	{ fa_comp_tag_map.name(), std::cref(fa_comp_tag_map) },
	{ fa_ssb_map.name(), std::cref(fa_ssb_map) },
	{ freq_axis_format_map.name(), std::cref(freq_axis_format_map) },
	{ freq_axis_type_map.name(), std::cref(freq_axis_type_map) },
	{ inl_line_fit_map.name(), std::cref(inl_line_fit_map) },
	{ rfft_scale_map.name(), std::cref(rfft_scale_map) },
	{ window_map.name(), std::cref(window_map) }
};

int enum_value(const str_t &enumeration, const str_t &enumerator) {
	if (enumeration_map.find(enumeration) == enumeration_map.end()) {
		throw runtime_error("enum_value : unknown enumeration, '" +
				enumeration + "'");
	}
	const enum_map &em = enumeration_map.at(enumeration);
	em.contains(enumerator, true);
	return em.at(enumerator);
}

template <>
AnalysisType get_enum<AnalysisType>(int i) {
	analysis_type_map.contains(i, true);
	return static_cast<AnalysisType>(i);
}

template <>
CodeFormat get_enum<CodeFormat>(int i) {
	code_format_map.contains(i, true);
	return static_cast<CodeFormat>(i);
}

template <>
DnlSignal get_enum<DnlSignal>(int i) {
	dnl_signal_map.contains(i, true);
	return static_cast<DnlSignal>(i);
}

template <>
FACompTag get_enum<FACompTag>(int i) {
	fa_comp_tag_map.contains(i, true);
	return static_cast<FACompTag>(i);
}

template <>
FASsb get_enum<FASsb>(int i) {
	fa_ssb_map.contains(i, true);
	return static_cast<FASsb>(i);
}

template <>
FreqAxisFormat get_enum<FreqAxisFormat>(int i) {
	freq_axis_format_map.contains(i, true);
	return static_cast<FreqAxisFormat>(i);
}

template <>
FreqAxisType get_enum<FreqAxisType>(int i) {
	freq_axis_type_map.contains(i, true);
	return static_cast<FreqAxisType>(i);
}

template <>
InlLineFit get_enum<InlLineFit>(int i) {
	inl_line_fit_map.contains(i, true);
	return static_cast<InlLineFit>(i);
}

template <>
RfftScale get_enum<RfftScale>(int i) {
	rfft_scale_map.contains(i, true);
	return static_cast<RfftScale>(i);
}

template <>
Window get_enum<Window>(int i) {
	window_map.contains(i, true);
	return static_cast<Window>(i);
}

} // namespace genalyzer_impl