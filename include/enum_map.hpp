// Copyright (C) 2024-2025 Analog Devices, Inc.
//
// SPDX short identifier: ADIBSD OR GPL-2.0-or-later
#ifndef GENALYZER_IMPL_ENUM_MAP_HPP
#define GENALYZER_IMPL_ENUM_MAP_HPP

#include "exceptions.hpp"
#include "type_aliases.hpp"

#include <initializer_list>
#include <map>

namespace genalyzer_impl {

class enum_map {
public:
	using itos_map_t = std::map<int, str_t>;
	using stoi_map_t = std::map<str_t, int>;
	using const_iterator = itos_map_t::const_iterator;

public:
	enum_map(const char *name,
			std::initializer_list<std::pair<int, str_t>> list);

public:
	~enum_map() = default;

public: // Element Access
	const str_t &at(int i) const {
		contains(i, true);
		return m_itos.at(i);
	}

	int at(const str_t &s) const {
		contains(s, true);
		return m_stoi.at(s);
	}

public: // Iterators
	const_iterator begin() const {
		return m_itos.begin();
	}

	const_iterator end() const {
		return m_itos.end();
	}

public: // Lookup
	bool contains(int i, bool throw_if_not_found = false) const;

	bool contains(const str_t &s, bool throw_if_not_found = false) const;

public: // Other Member Functions
	const str_t &name() const {
		return m_name;
	}

	size_t size() const {
		return m_itos.size();
	}

private:
	str_t m_name;
	itos_map_t m_itos; // int to string
	stoi_map_t m_stoi; // string to int

}; // class enum_map

} // namespace genalyzer_impl

#endif // GENALYZER_IMPL_ENUM_MAP_HPP