/*
 * test_genalyzer - Header file for running genalyzer tests
 *
 * Copyright (C) 2021 Analog Devices, Inc.
 * Author: Srikanth Pagadarai
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * */


#include <cgenalyzer.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum datatype { INT32,
    INT64,
    DOUBLE } datatype;

static inline bool floats_almost_equal(double a, double b, size_t num_digits)
{
    return ((((int)floor(fabs(a - b) * pow(10.0, num_digits))) == 0) ? true : false);
}

bool float_arrays_almost_equal(double* a, double* b, size_t len, size_t num_digits)
{
    bool result = true;
    for (int n = 0; n < len; n++)
        result &= floats_almost_equal(a[n], b[n], num_digits);

    return result;
}

static inline bool int32_arrays_equal(int* a, int* b, size_t arr_size, size_t tol)
{
    bool result = true;
    for (int n = 0; n < arr_size; n++)
        result &= (abs(a[n] - b[n]) <= tol) ? true : false;

    return result;
}

static inline bool int64_arrays_equal(long int* a, long int* b, size_t arr_size, size_t tol)
{
    bool result = true;
    for (int n = 0; n < arr_size; n++)
        result &= (abs(a[n] - b[n]) <= tol) ? true : false;

    return result;
}

bool int_arrays_almost_equal(void* a, void* b, size_t arr_size, size_t tol, datatype arr_type)
{
    if (arr_type == INT32)
        return (int32_arrays_equal(a, b, arr_size, tol));
    else if (arr_type == INT64)
        return (int64_arrays_equal(a, b, arr_size, tol));
}

char* extract_token(const char* file_name, const char* token, unsigned int* err_val)
{
    char line[256];
    char *local_line_split;
    FILE* fp = fopen(file_name, "r");
    if (fp == NULL) {
        fprintf(stderr, "%s: %s\n", file_name, strerror(errno));
        *err_val = errno;
    }

    while (fgets(line, sizeof(line), fp) != NULL) {
	    local_line_split = strtok(line, "=");
        if (strcmp(local_line_split, token) == 0) {
            local_line_split = strtok(NULL, "=");
            if (local_line_split != NULL) {
                fclose(fp);
                *err_val = 0;
                return strdup(local_line_split);
            }
        }
    }

    fclose(fp);
    *err_val = EINVAL;
    return NULL;
}

int read_file_to_array(const char* file_name, void* result, datatype result_type)
{
    char line[256];
    FILE* fp = fopen(file_name, "r");
    if (fp == NULL) {
        fprintf(stderr, "%s: %s\n", file_name, strerror(errno));
        return errno;
    }

    size_t n = 0;
    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, "----------") != NULL)
            break;
    }

    if (result_type == INT32) {
        int* i32_result = result;
        while (fscanf(fp, "%d", &i32_result[n++]) != EOF)
            ;
    } else if (result_type == INT64) {
        long int* i64_result = result;
        while (fscanf(fp, "%ld", &i64_result[n++]) != EOF)
            ;
    } else if (result_type == DOUBLE) {
        double* d_result = result;
        while (fscanf(fp, "%lf", &d_result[n++]) != EOF)
            ;
    }
    fclose(fp);
    return 0;
}

int write_array_to_file(const char* file_name, void* result, size_t r_size, datatype result_type)
{
    char line[256];
    FILE* fp = fopen(file_name, "w");
    if (fp == NULL) {
        fprintf(stderr, "%s: %s\n", file_name, strerror(errno));
        return errno;
    }

    if (result_type == INT32) {
        int* i32_result = result;
        for (int n = 0; n < r_size; n++)
            fprintf(fp, "%d\n", i32_result[n]);
    } else if (result_type == DOUBLE) {
        double* d_result = result;
        for (int n = 0; n < r_size; n++)
            fprintf(fp, "%lf\n", d_result[n]);
    }

    fclose(fp);
    return 0;
}

void deinterleave(void* input, size_t in_size, void* result_re, void* result_im, datatype result_type)
{
    if (result_type == INT32) {
        int* i32_input = input;
        int* i32_result_re = result_re;
        int* i32_result_im = result_im;

        for (int n = 0; n < in_size; n += 2)
            i32_result_re[n / 2] = i32_input[n];
        for (int n = 1; n < in_size; n += 2)
            i32_result_im[(n - 1) / 2] = i32_input[n];
    } else if (result_type == DOUBLE) {
        double* d_input = input;
        double* d_result_re = result_re;
        double* d_result_im = result_im;

        for (int n = 0; n < in_size; n += 2)
            d_result_re[n / 2] = d_input[n];
        for (int n = 1; n < in_size; n += 2)
            d_result_im[(n - 1) / 2] = d_input[n];
    }
}
