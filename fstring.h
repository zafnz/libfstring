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
#include <stdlib.h>

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
#define fstr_vt_list    7
#define fstr_vt_environment 8

struct fstr_value_t;
struct _fstr_env;

/**
 * @brief Values to pass to fstring's values list
 * 
 */
typedef struct fstr_value_t {
    const char *name;
    char type;
    const union {
        const char *s;
        int i;
        long l;
        float f;
        double d;
        fstring_callback_t cb;
        struct fstr_value_t **list;
        struct _fstr_env *env;
    } value;
    void *cb_data;
} fstr_value;

struct _fstr_env {
    size_t count, allocated;
    fstr_value variables[1];
};
typedef struct _fstr_env *fstr_environment; 

/**
 * @brief A convienence define for passing lists to lfstring and lbfstring
 * 
 * @details
 * Eg instead of 
 * @code
 * lfstring("blah", (fstr_value *[]) { 
 *       f_str(thing) 
 * });
 * @endcode
 * 
 * You can do 
 * @code
 * lfstring("blah", vstr_values_cast { 
 *       f_str(thing)
 * });
 * @endcode
 * 
 * Purely syntactic sugar
 */
#define fstr_values_cast  (fstr_value *[] )

/**
 * @brief Macros for converting local variables to pass to fstring functions
 * @details
 * These macros are for converting your local variables into structures to pass to
 * the fstring functions. They handle the naming and creating of stack memory structures.
 * 
 * Each starts with fstr_ or fstr_n, and the second part is the variable type.
 * Macros with fstr_n are "named", as in you specify both the name of the variable and
 * the value. 
 * Macros that are just fstr_ use the name of the variable passed as the name.
 * 
 * eg:
 * @code
 *     int x = 10;
 *     fstr_int(x); // Specifies a variable named "x" that contains the contents of x (10).
 *     fstr_nint("boo", x) // A variable named "boo" that contains the contents of x (10). 
 * 
 *     char *magic = "value";
 *     fstr_str(magic); // A variable called "magic", that contains "value";
 * @endcode
 * 
 * There are a few different built-in variables, each of these has the fstr_<type> and fstr_n<type>:
 * 
 *      fstr_str    - A string (char *)
 *      fstr_int    - An integer (int)
 *      fstr_long   - A long int (long int)
 *      fstr_float  - A floating point number (float)
 *      fstr_double - A double float (double)
 * 
 * There is also callbacks, called fstr_cb and fstr_ncb. See the lbnfstring() for more information
 * on those.
 * 
 */
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

#define fstr_list(LIST)     &((fstr_value){.name="*", .type=fstr_vt_list, .value.list=LIST})

#define fstr_env(ENV)       &((fstr_value){.name="*", .type=fstr_vt_environment, .value.env=ENV})

#define fstr_end        NULL


#define env_mem_size(N) (sizeof(struct _fstr_env) + sizeof(fstr_value) * N)

#define fstr_env_init(NAME) do { NAME = calloc(env_mem_size(20), 1); NAME->var_allocated = 20; } while(0)

inline void fstr_env_add(fstr_environment *eptr, fstr_value *val) {
    fstr_environment e = *eptr;
    if (++e->count == e->allocated) {
         e->allocated *= 2; 
         *eptr = realloc(*eptr, env_mem_size(e->allocated));
    };
    memcpy(&e->variables[e->count], val, sizeof(fstr_value));
}

/** int lbfstring(char *buffer, size_t buffer_len, const char *format, fstr_value *values[]);
 * @brief Formatted string using supplied variables (similar to Python's fstrings)
 * 
 * @details 
 * 
 * The format string is a character string that is copied to the buffer. Where a value directive 
 * occurs, which is a string surrounded by curly braces, the provided values list is looked up
 * for the corresponding value, and that value is substituded into the string. The value can be
 * any length, longer or shorter than the name without issue. 
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
 *                   array of points to fstr_value structures, where the last pointer
 *                   must be NULL.
 * 
 * @return           Returns the number of characters written to the buffer (excluding the
 *                   end of string \0). If this function returns a positive integer then
 *                   the string is guarenteed to be \0 terminated. If the function returns
 *                   0 or a negative number then the contents of buffer is unspecified.
 * 
 *                   If the buffer is not large enough to fit the contents of the formatted
 *                   input string and variables, then the minimum size is returned as a negative
 *                   number. NOTE: This is what the function has calculated so far, and it may
 *                   need to be much bigger. Eg if you supply a 10 character buffer, and a 
 *                   format string that has many substitutions that expand to a much larger
 *                   space, then this function will probably return a number too small to begin
 *                   with.
 * 
 *                   If you want a function that will always return a large enough buffer, look
 *                   a fstring, vstring or lstring, which return a malloc()'d buffer of sufficient
 *                   size.
 *                   
 *  There is a lot of syntactic sugar around these functions to make it easy to use. To the point
 *  where it is best not to worry about how it works underneath and just accept the world as it
 *  is presented to you. 
 * 
 *  To that end, work with the examples to learn how to use it.
 * 
 *  ## Examples
 * 
 *  Inline passing. This demonstrates probably the most verbose way of calling fstring functions.
 * 
 *  @code
 *  lbfstring(buffer, sizeof(buffer), "This is a {variable} which is {what}", fstr_values_cast {
 *          fstr_nstr("variable", "thing"),
 *          fstr_nstr("what", "cool"),
 *          fstr_end // syntactic sugar for NULL
 *  });
 *  // Returns "This is a thing which is cool"
 *  @endcode
 * 
 *  More expanded
 *  @code
 *  float pi = 3.14159;
 *  char *how_good = "awesome";
 *  fstr_value *values[] = {
 *      fstr_nstr("what is it", "magic"),
 *      fstr_float(pi),
 *      fstr_str(how_good),
 *      fstr_end
 *  };
 * 
 *  fstring(buffer, sizeof(buffer), "This {what is it} library does {how_good} things like: {pi}");
 *  // Buffer now contains: "This magic library does awesome things like: 3.14159"
 *  @endcode
 * 
 */
extern int lbfstring(char *buffer, size_t buffer_len, const char *format, fstr_value *values[]);


/** 
 * @brief a version of lbfstring that instead of taking an array, takes values as function arguments instead.
 * 
 * @details         This function works in the same way as lbfstring, except instead of taking a
 *                  fstr_value *[] list, the variables are passed as arguments to the function, similar to
 *                  how printf works. The final argument must be a NULL (or fstr_end). Best to see the 
 *                  examples below.
 *                
 *                  The difference between this function and lbfstring is minimal, with the exception
 *                  that with this function you must know exactly what variables you are going to pass
 *                  where as with lbfstring you can provide an array that you've assembled elsewhere,
 *                  possibly dynamically.
 * 
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
 * @return          Identical to lbfstring, see that function for detailed information.
 * 
 * ## Examples ##
 * 
 * @code
 *  lbfstring(buffer, sizeof(buffer), "This is a {variable} which is {what}", fstr_nstr("variable", "thing"), fstr_nstr("cool"), fstr_end);
 *  // Results in as you'd expect. "This is a thing that is cool".
 */
extern int bfstring(char *buffer, size_t buffer_len, const char *format, fstr_value *, ...);


/**
 *  @brief a version of lbfstring that instead of taking an array, takes a va_list. See bfstring for further.
 */
extern int vbfstring(char *buffer, size_t buffer_len, const char *format, va_list vl);


extern char *fstring(const char *format, fstr_value *, ...);
extern char *vfstring(const char *format, va_list vl);
extern char *lfstring(const char *format, fstr_value *values[]);


#endif