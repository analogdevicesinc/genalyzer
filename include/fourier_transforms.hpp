// Copyright (C) 2024 Analog Devices, Inc.
//
// SPDX short identifier: ADIBSD OR GPL-2.0-or-later
#ifndef GENALYZER_IMPL_FOURIER_TRANSFORMS_HPP
#define GENALYZER_IMPL_FOURIER_TRANSFORMS_HPP

#include "enums.hpp"
#include "type_aliases.hpp"

namespace genalyzer_impl {

void fft(const real_t *i_data, size_t i_size, const real_t *q_data,
		size_t q_size, real_t *out_data, size_t out_size, size_t navg,
		size_t nfft, Window window);

template <typename T>
void fft(const T *i_data, size_t i_size, const T *q_data, size_t q_size,
		real_t *out_data, size_t out_size, int n, size_t navg, size_t nfft,
		Window window, CodeFormat format);

size_t fft_size(size_t i_size, size_t q_size, size_t &navg, size_t &nfft);

void rfft(const real_t *in_data, size_t in_size, real_t *out_data,
		size_t out_size, size_t navg, size_t nfft, Window window,
		RfftScale scale);

template <typename T>
void rfft(const T *in_data, size_t in_size, real_t *out_data, size_t out_size,
		int n, size_t navg, size_t nfft, Window window, CodeFormat format,
		RfftScale scale);

size_t rfft_size(size_t in_size, size_t &navg, size_t &nfft);

} // namespace genalyzer_impl

#endif // GENALYZER_IMPL_FOURIER_TRANSFORMS_HPP