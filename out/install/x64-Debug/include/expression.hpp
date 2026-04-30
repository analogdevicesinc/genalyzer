// Copyright (C) 2024-2025 Analog Devices, Inc.
//
// SPDX short identifier: ADIBSD OR GPL-2.0-or-later
#ifndef GENALYZER_IMPL_EXPRESSION_HPP
#define GENALYZER_IMPL_EXPRESSION_HPP

#include "enums.hpp"
#include "type_aliases.hpp"

#include <map>
#include <memory>
#include <set>

namespace genalyzer_impl {

struct expression_token;

class expression {
public:
	using token_ptr = std::unique_ptr<expression_token>;
	using token_vector = std::vector<token_ptr>;
	using var_map = std::map<str_t, real_t>;
	using var_set = std::set<str_t>;

public:
	expression(const str_t &infix_string);

	~expression();

public:
	// Returns true if expression depends on one or more variables in vars
	bool depends_on(const var_set &vars) const;

	real_t evaluate(const var_map &vars = var_map()) const;

	str_t to_postfix_string(FPFormat fmt = FPFormat::Auto,
			int max_prec = -1) const;

	str_t to_string(FPFormat fmt = FPFormat::Auto, int max_prec = -1) const;

	// Returns the variables expression depends on
	var_set vars() const;

	// Returns the first expression variable not found in vars
	str_t vars_defined(const var_map &vars) const;

private:
	token_vector m_infix_tokens;

}; // class expression

} // namespace genalyzer_impl

#endif // GENALYZER_IMPL_EXPRESSION_HPP