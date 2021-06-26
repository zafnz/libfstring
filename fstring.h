/** @file fstring.h
 *  @brief A C version of python's fstring
 *
 *  @copyright Copyright 2021 - Nick Clifford <nick@crypto.geek.nz>
 *  @licence  Apache 2.0 License
 * 
 *  @author - Nick Clifford (nick@crypto.geek.nz)
 */
#ifndef include_fstring_h
#define include_fstring_h

#include <sys/types.h>


typedef const char *(*fstring_callback_t)(void *data, const char *name);

struct fstring_value {
    const char *name;
    const char *value;
    fstring_callback_t callback;
    void *callback_data;
};

typedef struct fstring_value fstring_value;

extern int fstring(char *buffer, size_t buffer_len, const char *format, fstring_value *values);

#endif