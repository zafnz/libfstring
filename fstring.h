/** @file fstring.h
 *  @brief A C version of python's fstring
 *
 *  @copyright Copyright 2021 - Nick Clifford <nick@crypto.geek.nz>
 * 
 *  @author - Nick Clifford (nick@crypto.geek.nz)
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.    
 */
#ifndef include_fstring_h
#define include_fstring_h

#include <sys/types.h>

/**
 * @brief The callback type for dynamic values. 
 */
typedef const char *(*fstring_callback_t)(void *data, const char *name);

#define fstr_vt_null    0
#define fstr_vt_str     1
#define fstr_vt_int     2
#define fstr_vt_long    3
#define fstr_vt_float   4
#define fstr_vt_double  5
#define fstr_vt_cb      6

/**
 * @brief Values to pass to fstring's values list
 * 
 */
typedef struct {
    const char *name;
    char type;
    const union {
        const char *s;
        int i;
        long l;
        float f;
        double d;
        fstring_callback_t cb;
    } value;
    void *cb_data;
} fstr_value;

#define fstr_values_cast  (fstr_value *[] )


#define fstr_nstr(N, V)     &((fstr_value){.name=N, .type=fstr_vt_str, .value.s=V})
#define fstr_nint(N, V)     &((fstr_value){.name=N, .type=fstr_vt_int, .value.i=V})
#define fstr_nlong(N, V)    &((fstr_value){.name=N, .type=fstr_vt_long, .value.l=V})
#define fstr_nfloat(N, V)   &((fstr_value){.name=N, .type=fstr_vt_float, .value.f=V})
#define fstr_ndouble(N, V)  &((fstr_value){.name=N, .type=fstr_vt_double, .value.d=V})

#define fstr_str(X)         &((fstr_value){.name=#X, .type=fstr_vt_str, .value.s=X})
#define fstr_int(X)         &((fstr_value){.name=#X, .type=fstr_vt_int, .value.i=X})
#define fstr_long(X)        &((fstr_value){.name=#X, .type=fstr_vt_long, .value.l=X})
#define fstr_float(X)       &((fstr_value){.name=#X, .type=fstr_vt_float, .value.f=X})
#define fstr_double(X)      &((fstr_value){.name=#X, .type=fstr_vt_double, .value.d=X})

#define fstr_ncb(N, CB, DATA)   &((fstr_value){.name=N, .type=fstr_vt_cb, .value.cb=CB, .cb_data=DATA})
#define fstr_cb(CB, DATA)      &((fstr_value){.name=#CB, .type=fstr_vt_cb, .value.cb=CB, .cb_data=DATA})

#define fstr_end        NULL


/**
 * @brief Formatted string using supplied variables (similar to Python's fstrings)
 * 
 * @details Fills buffer with an output as described below.
 * 
 * The format string is a character string that is copied to the buffer. Where a
 * value directive occurs, which is a string surrounded by curly braces, the
 * provided values list is looked up for the corresponding value, and that value
 * is substituded into the string. The value can be any length, longer or shorter
 * than the name without issue. 
 * 
 * @param[out] buffer     The buffer to store the output in, based on the contents of
 *                   format and the values. If there is an error condition, the
 *                   contents of buffer is unspecified and may not be NULL 
 *                   terminated.
 * 
 * @param[in] buffer_len The maximum length of the buffer, the string will be NULL 
 *                   terminated when writing to the buffer, buffer_len needs to
 *                   be large enough to include the NULL character.
 * 
 * @param[in] format     The string to parse to produce the output. The string can
 *                   contain variable names enclosed with braces, and when variables
 *                   are found they are looked up in the values list
 * 
 * @param[in] values     The values of the variables are in this list. The list is an
 *                   array of fstring_value structures, where the last structure's
 *                   name property must be NULL.
 * 
 * @return           Returns the formatted string.
 * 
 * ## Examples
 * 
 * Inline passing
 * @code
 * fstring(buffer, sizeof(buffer), "This is a {variable} which is {what}", (fstring_value[]) {
 *          {.name="variable", .value="thing"},
 *          {.name="what", .value="cool"}
 *          {.name=NULL}
 * });
 * // Returns "This is a thing which is cool"
 * @endcode
 * 
 * More expanded
 * @code
 * fstring_value values[] = {
 *      { .name="variable", .value="thing" },
 *      { .name="what", .value="cool" }
 * };
 * fstring(buffer, sizeof(buffer), "This is a {variable} which is {what}", values);
 * // Returns "this is a thing which is cool"
 * @endcode
 */




extern char *fstring(const char *format, fstr_value *, ...);
extern char *vfstring(const char *format, va_list vl);
extern char *lfstring(const char *format, fstr_value *values[]);

extern int bfstring(char *buffer, size_t buffer_len, const char *format, fstr_value *, ...);
extern int vbfstring(char *buffer, size_t buffer_len, const char *format, va_list vl);
extern int lbfstring(char *buffer, size_t buffer_len, const char *format, fstr_value *values[]);


/**
 * @brief Formatted string using supplied variables with malloc'd output
 * 
 * @details     This function is identical to fstring, except it dynamically allocates the
 *              memory for you.
 * 
 * @param[in]   format      See fstring for an explanation
 * @param[in]   values      See fstring for an explanation
 * 
 * @return      The formatted string with the variables replaced. 
 *              It will return NULL if there is a parsing error in the string or 
 *              if the buffer is over 1MB.
 *              It is up to you to free() the string.
 * 
 * @code
 *      char *result = dfstring("{hello}", (fstring_value[]) {{.name="hello", .value="world"}});
 *      puts(result); // prints "world"
 */



#endif