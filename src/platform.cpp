// Copyright (C) 2024-2025 Analog Devices, Inc.
//
// SPDX short identifier: ADIBSD OR GPL-2.0-or-later
#include "type_aliases.hpp"

namespace genalyzer_impl {

static_assert(sizeof(int16_t) == 2, "sizeof(int16_t) != 2");
static_assert(sizeof(int32_t) == 4, "sizeof(int32_t) != 4");
static_assert(sizeof(int64_t) == 8, "sizeof(int64_t) != 8");
static_assert(sizeof(uint16_t) == 2, "sizeof(uint16_t) != 2");
static_assert(sizeof(uint32_t) == 4, "sizeof(uint32_t) != 4");
static_assert(sizeof(uint64_t) == 8, "sizeof(uint64_t) != 8");
static_assert(sizeof(real_t) == 8, "sizeof(real_t) != 8");
static_assert(sizeof(cplx_t) == 16, "sizeof(cplx_t) != 16");

static_assert(sizeof(int) >= 4, "sizeof(int) < 4");

} // namespace genalyzer_impl