// Copyright (C) 2024 Analog Devices, Inc.
//
// SPDX short identifier: ADIBSD OR GPL-2.0-or-later
#include "utils.hpp"

#include <cmath>
#include <iomanip>
#include <limits>
#include <sstream>

namespace genalyzer_impl {

str_t to_string(real_t n, FPFormat fmt, int max_prec) {
	if (!std::isfinite(n)) {
		throw runtime_error("to_string : non-finite number");
	}
	int max_digits10 = std::numeric_limits<real_t>::max_digits10;
	if (max_prec < 0 || max_digits10 < max_prec) {
		max_prec = max_digits10;
	}
	std::ostringstream ss;
	ss.precision(max_prec);
	switch (fmt) {
		case FPFormat::Auto:
			ss << std::defaultfloat << n;
			break;
		case FPFormat::Eng: {
			bool neg = std::signbit(n);
			n = std::fabs(n);
			real_t exp3 = 0;
			if (0.0 < n) {
				exp3 = std::floor(std::log10(n) / 3) *
						3; // snap exponent to nearest multiple of 3
			}
			if (exp3 < -3 || 0 < exp3) {
				n *= std::pow(10.0, -exp3);
			}
			if (neg) {
				ss << '-';
			}
			ss << std::defaultfloat << n;
			if (exp3 < -3 || 0 < exp3) {
				ss << 'e' << exp3;
			}
			break;
		}
		case FPFormat::Fix:
			ss << std::fixed << n;
			break;
		case FPFormat::Sci:
			ss << std::scientific << n;
			break;
		default:
			throw runtime_error(
					"to_string : unknown floating point format");
	}
	return ss.str();
}

} // namespace genalyzer_impl