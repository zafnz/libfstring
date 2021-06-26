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


struct cfstring_parameter {
    const char *name;
    const char *value;
    void (*callback)(void *, const char *, const char *);
    void *callback_data;
};


extern int cfstring(char *buffer, size_t buffer_len, const char *format, struct cfstring_parameter *parameters);

#endif