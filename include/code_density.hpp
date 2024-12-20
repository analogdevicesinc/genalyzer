// Copyright (C) 2024 Analog Devices, Inc.
//
// SPDX short identifier: ADIBSD OR GPL-2.0-or-later
#ifndef GENALYZER_IMPL_CODE_DENSITY_HPP
#define GENALYZER_IMPL_CODE_DENSITY_HPP

#include "enums.hpp"
#include "type_aliases.hpp"

#include <map>

namespace genalyzer_impl {

size_t code_density_size(int n, CodeFormat format);

size_t code_densityx_size(int64_t min, int64_t max);

void code_axis(real_t *data, size_t size, int n, CodeFormat format);

void code_axisx(real_t *data, size_t size, int64_t min, int64_t max);

void dnl(real_t *dnl_data, size_t dnl_size, const uint64_t *hist_data,
		size_t hist_size, DnlSignal type);

std::map<str_t, real_t> dnl_analysis(const real_t *data, size_t size);

const std::vector<str_t> &dnl_analysis_ordered_keys();

template <typename T>
void hist(uint64_t *hist_data, size_t hist_size, const T *wf_data,
		size_t wf_size, int n, CodeFormat format, bool preserve);

template <typename T>
void histx(uint64_t *hist_data, size_t hist_size, const T *wf_data,
		size_t wf_size, int64_t min, int64_t max, bool preserve);

std::map<str_t, real_t> hist_analysis(const uint64_t *data, size_t size);

const std::vector<str_t> &hist_analysis_ordered_keys();

void inl(real_t *inl_data, size_t inl_size, const real_t *dnl_data,
		size_t dnl_size, InlLineFit fit);

std::map<str_t, real_t> inl_analysis(const real_t *data, size_t size);

const std::vector<str_t> &inl_analysis_ordered_keys();

} // namespace genalyzer_impl

#endif // GENALYZER_IMPL_CODE_DENSITY_HPP