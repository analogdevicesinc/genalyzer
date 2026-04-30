// Copyright (C) 2024-2025 Analog Devices, Inc.
//
// SPDX short identifier: ADIBSD OR GPL-2.0-or-later
#ifndef GENALYZER_IMPL_TYPE_ALIASES_HPP
#define GENALYZER_IMPL_TYPE_ALIASES_HPP

#include <complex>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace genalyzer_impl {

using diff_t = std::ptrdiff_t;
using size_t = std::size_t;
using int16_t = std::int_least16_t;
using int32_t = std::int_least32_t;
using int64_t = std::int_least64_t;
using uint16_t = std::uint_least16_t;
using uint32_t = std::uint_least32_t;
using uint64_t = std::uint_least64_t;
using real_t = double;
using cplx_t = std::complex<real_t>;
using str_t = std::string;

// pairs
using diff_p = std::pair<diff_t, diff_t>;
using size_p = std::pair<size_t, size_t>;
using real_p = std::pair<real_t, real_t>;

// vectors
using str_vector = std::vector<str_t>;

} // namespace genalyzer_impl

#endif // GENALYZER_IMPL_TYPE_ALIASES_HPP