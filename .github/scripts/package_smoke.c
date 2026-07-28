// Copyright (C) 2024-2026 Analog Devices, Inc.
//
// SPDX short identifier: ADIBSD OR GPL-2.0-or-later
#include <cgenalyzer.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef EXPECTED_VERSION
#error "EXPECTED_VERSION must be defined"
#endif

int main(void) {
	size_t size = 0;
	if (gn_version_string_size(&size) != 0 || size == 0) {
		return EXIT_FAILURE;
	}

	char *version = malloc(size + 1);
	if (version == NULL) {
		return EXIT_FAILURE;
	}

	const int result = gn_version_string(version, size + 1);
	if (result == 0) {
		printf("libgenalyzer %s\n", version);
	}
	const int version_matches = result == 0 && strcmp(version, EXPECTED_VERSION) == 0;
	free(version);
	return version_matches ? EXIT_SUCCESS : EXIT_FAILURE;
}