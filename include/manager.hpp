// Copyright (C) 2024-2026 Analog Devices, Inc.
//
// SPDX short identifier: ADIBSD OR GPL-2.0-or-later
#ifndef GENALYZER_IMPL_MANAGER_HPP
#define GENALYZER_IMPL_MANAGER_HPP

#include "enums.hpp"
#include "object.hpp"
#include "type_aliases.hpp"

/**
 * @brief Public API for the global object manager.
 *
 * The manager stores named objects (e.g., fourier_analysis configurations)
 * that can be retrieved, compared, serialized, and removed by key.
 */
namespace genalyzer_impl::manager {

/**
 * @brief Remove all objects from the global object manager.
 */
void clear();

/**
 * @brief Check whether an object with the given key exists in the manager.
 *
 * @param key                Key to look up.
 * @param throw_if_not_found If true, throw an exception when the key is not found.
 * @return True if the key exists, false otherwise.
 */
bool contains(const str_t &key, bool throw_if_not_found = false);

/**
 * @brief Compare two managed objects for equality.
 *
 * @param key1 Key of the first object.
 * @param key2 Key of the second object.
 * @return True if the objects are equal, false otherwise.
 */
bool equal(const str_t &key1, const str_t &key2);

/**
 * @brief Remove the object with the given key from the manager.
 *
 * @param key Key of the object to remove.
 */
void remove(const str_t &key);

/**
 * @brief Serialize the object with the given key to a JSON file.
 *
 * If @p filename is empty, a default filename is generated from the key.
 *
 * @param key      Key of the object to save.
 * @param filename Output file path (empty for auto-generated name).
 * @return The filename that was written.
 */
str_t save(const str_t &key, const str_t &filename = "");

/**
 * @brief Return the number of objects currently stored in the manager.
 *
 * @return Object count.
 */
size_t size();

/**
 * @brief Return a string representation of a managed object.
 *
 * If @p key is empty, returns a summary of all objects in the manager.
 *
 * @param key Key of the object (empty for summary of all objects).
 * @return String representation.
 */
str_t to_string(const str_t &key = "");

/**
 * @brief Return the type name of a managed object as a string.
 *
 * @param key Key of the object.
 * @return Type name string.
 */
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
