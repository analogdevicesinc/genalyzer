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
#include "cJSON.h"

typedef enum datatype { INT32,
    INT64,
    UINT32,
    UINT64,
    DOUBLE } datatype;

static inline bool floats_almost_equal(double a, double b, size_t num_digits)
{
    return ((((int)floor(fabs(a - b) * pow(10.0, 1.0*num_digits))) == 0) ? true : false);
}

bool float_arrays_almost_equal(double* a, double* b, size_t len, size_t num_digits)
{
    bool result = true;
    for (int n = 0; n < len; n++)
        result &= floats_almost_equal(a[n], b[n], num_digits);

    return result;
}

static inline bool int32_arrays_equal(const int* a, const int* b, size_t arr_size, size_t tol)
{
    bool result = true;
    for (int n = 0; n < arr_size; n++)
        result &= (abs(a[n] - b[n]) <= tol) ? true : false;

    return result;
}

static inline bool int64_arrays_equal(const long int* a, const long int* b, size_t arr_size, size_t tol)
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
    else
        return (false);
}

int extract_token(void *val, const char* file_name, const char* token_name)
{
    const cJSON *token = NULL;
    int status = 0;
    FILE *fp;
    long lSize;
    char *buffer;

    fp = fopen ( file_name , "rb" );
    if( !fp ) perror(file_name),exit(1);

    fseek( fp , 0L , SEEK_END);
    lSize = ftell( fp );
    rewind( fp );

    /* allocate memory for entire content */
    buffer = calloc( 1, lSize+1 );
    if( !buffer ) fclose(fp),fputs("memory alloc fails",stderr),exit(1);

    /* copy the file into the buffer */
    if( 1!=fread( buffer , lSize, 1 , fp) )
        fclose(fp),free(buffer),fputs("entire read fails",stderr),exit(1);

    cJSON *file_name_json = cJSON_Parse(buffer);
    if (file_name_json == NULL)
    {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            fprintf(stderr, "Error before: %s\n", error_ptr);
        }
        status = 0;
        goto end;
    }

    token = cJSON_GetObjectItemCaseSensitive(file_name_json, token_name);
    *(int*)val = token->valueint;

end:
    cJSON_Delete(file_name_json);
    return status;
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

int read_param(const char* file_name, const char* param_name, void* result, datatype result_type)
{
    unsigned int err_code;
    int I32_tmp_token;
    long int I64_tmp_token;
    unsigned long UI32_tmp_token;
    unsigned long long UI64_tmp_token;
    double D_tmp_token;
        
    if (result_type == INT32) {
        int i32_result;
        err_code = extract_token((void*)(&i32_result), file_name, param_name);
        *(int*)result = i32_result;
    } else if (result_type == INT64) {
        //long int i64_result = atol(tmp_token);
        //*(long int*)result = i64_result;
    } else if (result_type == UINT32) {
        //unsigned long ui32_result = atoll(tmp_token);
        //*(unsigned long*)result = ui32_result;
    } else if (result_type == UINT64) {
        unsigned long long ui64_result;
        err_code = extract_token((void*)(&ui64_result), file_name, param_name);
        *(unsigned long long*)result = ui64_result;printf("%llu\n",ui64_result);
    } else if (result_type == DOUBLE) {
        //double d_result = atof(tmp_token);
        //*(double*)result = d_result;
    }

    //free(tmp_token);
    return err_code;
}