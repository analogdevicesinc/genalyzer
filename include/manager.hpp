// Copyright (C) 2024 Analog Devices, Inc.
//
// SPDX short identifier: ADIBSD OR GPL-2.0-or-later
#ifndef GENALYZER_IMPL_MANAGER_HPP
#define GENALYZER_IMPL_MANAGER_HPP

#include "enums.hpp"
#include "object.hpp"
#include "type_aliases.hpp"

/*
 * Part of the API
 */
namespace genalyzer_impl::manager {

void clear();

bool contains(const str_t &key, bool throw_if_not_found = false);

bool equal(const str_t &key1, const str_t &key2);

void remove(const str_t &key);

str_t save(const str_t &key, const str_t &filename = "");

size_t size();

str_t to_string(const str_t &key = "");

str_t type_str(const str_t &key);

} // namespace genalyzer_impl::manager

/*
 * Internal use only
 */
namespace genalyzer_impl::manager {

static const str_t key_pattern = "[[:alpha:]][[:alnum:]._]*";

void add_object(const str_t &key, object::pointer obj, bool replace);

str_t get_filename_from_object_key(const str_t &key, str_t filename);

object::pointer get_object(const str_t &key);

ObjectType type(const str_t &key);

} // namespace genalyzer_impl::manager

#endif // GENALYZER_IMPL_MANAGER_HPP
