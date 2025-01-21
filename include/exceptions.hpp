// Copyright (C) 2024 Analog Devices, Inc.
//
// SPDX short identifier: ADIBSD OR GPL-2.0-or-later
#ifndef GENALYZER_IMPL_EXCEPTIONS_HPP
#define GENALYZER_IMPL_EXCEPTIONS_HPP

#include <stdexcept>

namespace genalyzer_impl {

class logic_error final : public std::logic_error {
	using std::logic_error::logic_error;
};

class runtime_error final : public std::runtime_error {
	using std::runtime_error::runtime_error;
};

} // namespace genalyzer_impl

#endif // GENALYZER_IMPL_EXCEPTIONS_HPP