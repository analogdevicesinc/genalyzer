/*
 * test_genalyzer - Header file for running genalyzer tests
 *
 * Copyright (C) 2022 Analog Devices, Inc.
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


#include <cgenalyzer_simplified_beta.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "cJSON.h"

typedef enum datatype {INT16,
    INT32,
    INT64,
    UINT32,
    UINT64,
    DOUBLE} datatype;

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

int file_to_buffer(long *size, char **buffer, const char* file_name)
{
    FILE *fp;
    long f_size;
    char *buff;

    fp = fopen(file_name, "rb");
    if(!fp) {
        perror(file_name);
        exit(1);
    }

    fseek(fp, 0L, SEEK_END);
    f_size = ftell(fp);
    rewind(fp);

    // allocate memory
    buff = calloc(1, f_size+1);
    if(!buff) {
        fclose(fp);
        fputs("failed to allocate memory\n",stderr);
        exit(1);
    }

    // copy file into the buffer
    if(fread(buff, f_size, 1 , fp)!=1) {
        fclose(fp);
        free(buff);
        fputs("failed to read file\n",stderr);
        exit(1);
    }
    *size = f_size;
    *buffer = buff;

    return 0;
}

int extract_scalar(void *val, const char* file_name, const char* token_name, datatype result_type)
{
    const cJSON *token = NULL;
    int status = 0;
    long f_size;
    char *buffer;

    file_to_buffer(&f_size, &buffer, file_name);
    
    cJSON *file_name_json = cJSON_Parse(buffer);
    if (file_name_json == NULL)
    {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
            fprintf(stderr, "Error before: %s\n", error_ptr);
        status = 0;
        goto end;
    }

    token = cJSON_GetObjectItemCaseSensitive(file_name_json, token_name);
    if (result_type == INT16)
        *(int16_t*)val = token->valueint;
    else if (result_type == INT32)
        *(int32_t*)val = token->valueint;
    else if (result_type == INT64)
        *(int64_t*)val = token->valueint;
    else if (result_type == UINT32)
        *(uint32_t*)val = token->valueint;
    else if (result_type == UINT64)
        *(uint64_t*)val = token->valueint;
    else if (result_type == DOUBLE) {        
        *(double*)val = token->valuedouble;
    }

end:
    cJSON_Delete(file_name_json);
    return status;
}

int extract_array(void *val, const char* file_name, const char* token_name, datatype result_type, size_t len)
{
    const cJSON *token = NULL;
    int status = 0;
    long f_size;
    char *buffer;

    file_to_buffer(&f_size, &buffer, file_name);
    
    cJSON *file_name_json = cJSON_Parse(buffer);
    if (file_name_json == NULL)
    {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
            fprintf(stderr, "Error before: %s\n", error_ptr);
        status = 0;
        goto end;
    }

    token = cJSON_GetObjectItemCaseSensitive(file_name_json, token_name);
    const cJSON *count = NULL;
    size_t i = 0;
    if (result_type == INT16) {
        int16_t *tmp = (int16_t*)malloc(len*sizeof(int16_t));
        cJSON_ArrayForEach(count, token) {
            tmp[i] = count->valueint;
            i++;
        }
        memcpy(val, tmp, len*sizeof(int16_t));
        free(tmp);
    }
    else if (result_type == INT32) {
        int32_t *tmp = (int32_t*)malloc(len*sizeof(int32_t));
        cJSON_ArrayForEach(count, token) {
            tmp[i] = count->valueint;
            i++;
        }
        memcpy(val, tmp, len*sizeof(int32_t));
        free(tmp);
    }
    else if (result_type == INT64) {
        int64_t *tmp = (int64_t*)malloc(len*sizeof(int64_t));
        cJSON_ArrayForEach(count, token) {
            tmp[i] = count->valueint;
            i++;
        }
        memcpy(val, tmp, len*sizeof(int64_t));
        free(tmp);
    }
    else if (result_type == UINT32) {
        uint32_t *tmp = (uint32_t*)malloc(len*sizeof(uint32_t));
        cJSON_ArrayForEach(count, token) {
            tmp[i] = count->valueint;
            i++;
        }
        memcpy(val, tmp, len*sizeof(uint32_t));
        free(tmp);
    }
    else if (result_type == UINT64) {
        uint64_t *tmp = (uint64_t*)malloc(len*sizeof(uint64_t));
        cJSON_ArrayForEach(count, token) {
            tmp[i] = count->valueint;
            i++;
        }
        memcpy(val, tmp, len*sizeof(uint64_t));
        free(tmp);
    }
    else if (result_type == DOUBLE) {                
        double *tmp = (double*)malloc(len*sizeof(double));
        cJSON_ArrayForEach(count, token) {
            tmp[i] = count->valuedouble;
            i++;
        }
        memcpy(val, tmp, len*sizeof(double));
        free(tmp);
    }    

end:
    cJSON_Delete(file_name_json);
    return status;
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

int read_scalar_from_json_file(const char* file_name, const char* param_name, void* result, datatype result_type)
{
    int err_code;
        
    if (result_type == INT16) {
        int16_t i16_result;
        err_code = extract_scalar((void*)(&i16_result), file_name, param_name, result_type);
        *(int16_t*)result = i16_result;
    } else if (result_type == INT32) {
        int32_t i32_result;
        err_code = extract_scalar((void*)(&i32_result), file_name, param_name, result_type);
        *(int32_t*)result = i32_result;
    } else if (result_type == INT64) {
        int64_t i64_result;
        err_code = extract_scalar((void*)(&i64_result), file_name, param_name, result_type);
        *(int64_t*)result = i64_result;
    } else if (result_type == UINT32) {
        uint32_t ui32_result;
        err_code = extract_scalar((void*)(&ui32_result), file_name, param_name, result_type);
        *(uint32_t*)result = ui32_result;
    } else if (result_type == UINT64) {
        uint64_t ui64_result;
        err_code = extract_scalar((void*)(&ui64_result), file_name, param_name, result_type);
        *(uint64_t*)result = ui64_result;
    } else if (result_type == DOUBLE) {
        double d_result;
        err_code = extract_scalar((void*)(&d_result), file_name, param_name, result_type);
        *(double*)result = d_result;
    }
    return err_code;
}

int read_array_from_json_file(const char* file_name, const char* param_name, void* result, datatype result_type, size_t len)
{
    int err_code;
        
    if (result_type == INT16) {
        int16_t *i16_result = (int16_t*)malloc(len*sizeof(int16_t));
        err_code = extract_array(i16_result, file_name, param_name, result_type, len);
        memcpy (result, i16_result, len*sizeof(int16_t));
        free(i16_result);
    }
    else if (result_type == INT32) {
        int32_t *i32_result = (int32_t*)malloc(len*sizeof(int32_t));
        err_code = extract_array(i32_result, file_name, param_name, result_type, len);
        memcpy (result, i32_result, len*sizeof(int32_t));
        free(i32_result);
    } else if (result_type == INT64) {
        int64_t *i64_result = (int64_t*)malloc(len*sizeof(int64_t));
        err_code = extract_array(i64_result, file_name, param_name, result_type, len);
        memcpy (result, i64_result, len*sizeof(int64_t));
        free(i64_result);
    } else if (result_type == UINT32) {
        uint32_t *ui32_result = (uint32_t*)malloc(len*sizeof(uint32_t));
        err_code = extract_array(ui32_result, file_name, param_name, result_type, len);
        memcpy (result, ui32_result, len*sizeof(uint32_t));
        free(ui32_result);
    } else if (result_type == UINT64) {
        uint64_t *ui64_result = (uint64_t*)malloc(len*sizeof(uint64_t));
        err_code = extract_array(ui64_result, file_name, param_name, result_type, len);
        memcpy (result, ui64_result, len*sizeof(uint64_t));
        free(ui64_result);
    } else if (result_type == DOUBLE) {
        double *d_result = (double*)malloc(len*sizeof(double));
        err_code = extract_array(d_result, file_name, param_name, result_type, len);
        memcpy (result, d_result, len*sizeof(double));
        free(d_result);
    }
    return err_code;
}