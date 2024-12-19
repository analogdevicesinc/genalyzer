// Copyright (C) 2024 Analog Devices, Inc.
//
// SPDX short identifier: ADIBSD OR GPL-2.0-or-later
#include "enum_map.hpp"

namespace genalyzer_impl {

enum_map::enum_map(const char *name,
		std::initializer_list<std::pair<int, str_t>> list) :
		m_name(name), m_itos{}, m_stoi{} {
	if (m_name.empty()) {
		throw runtime_error("enum_map : enumeration name is required");
	}
	for (const auto &es : list) {
		int i = es.first;
		const str_t &s = es.second;
		if (s.empty()) {
			throw runtime_error(
					"enum_map : enumerator name is required");
		}
		bool success = false;
		std::tie(std::ignore, success) = m_itos.emplace(i, s);
		if (!success) {
			throw runtime_error(
					"enum_map : enumerator already exists");
		}
		std::tie(std::ignore, success) = m_stoi.emplace(s, i);
		if (!success) {
			throw runtime_error(
					"enum_map : enumerator name already exists");
		}
	}
}

bool enum_map::contains(int i, bool throw_if_not_found) const {
	bool not_found = m_itos.end() == m_itos.find(i);
	if (not_found && throw_if_not_found) {
		throw runtime_error("enum_map : " + m_name +
				" does not contain value " +
				std::to_string(i));
	}
	return !not_found;
}

bool enum_map::contains(const str_t &s, bool throw_if_not_found) const {
	bool not_found = m_stoi.end() == m_stoi.find(s);
	if (not_found && throw_if_not_found) {
		throw runtime_error("enum_map : " + m_name +
				" : does not contain enumerator " + s);
	}
	return !not_found;
}

} // namespace genalyzer_impl