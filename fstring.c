/** @file fstring.c
 *  @brief A C version of python's fstring
 *
 *  @copyright Nick Clifford, 2021
 *  @license Apache 2.0 License
 * 
 *  @author - Nick Clifford (nick@crypto.geek.nz)
 */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>

#include "fstring.h"


const char *_value_lookup(const char *name, fstring_value *values)
{
    for(int i = 0; values && values[i].name != NULL; i++) {
        if (strcasecmp(name, values[i].name) == 0 || (values[i].name[0] == '*' && values[i].name[1] == 0)) {
            if (values[i].value == NULL && values[i].callback != NULL) {
                return (values[i].callback)(values[i].callback_data, name);
            } else {
                return values[i].value;
            }
        }
    }
    return NULL;
}

int fstring(char *buffer, size_t buffer_len, const char *format, fstring_value *values)
{
    char *sp, *dp, *name;
    const char *value;
    size_t buffer_remaining, value_len, remaining_len;

    if (buffer_len < strlen(format) + 1) {
        return 0 - (strlen(format) + 1);
    }
    strncpy(buffer, format, buffer_len);

    buffer_remaining = buffer_len;
    sp = buffer;
    dp = buffer;
    while(*sp != 0 && buffer_remaining > 0) {
        if (*sp == '{' && *(sp+1) == '{') {
            // If it's a curly brace follow by another curlly brace, it's considered
            // escaping, so "blah {{ blah" becomes "blah { blah"
            sp+=2;
            dp++;
        } else if (*sp == '{') {
            // We are at the beginning of a named variable. 
            sp++;
            name = sp;
            // Skip to the closing brace.
            while(*sp != 0 && *sp != '}') sp++;
            if (*sp == 0) {
                // Unfished curlybrace.
                fprintf(stderr, "Unfinished curly brace\n");
                return -1;
            }
            *sp = 0; // Terminate the string so we have a name.
            value = _value_lookup(name, values);
            if (value == NULL) {
                // Lookup failed, not there, we will simply output
                // the name {NAME} (so restore that closing brace)
                *sp = '}';
                sp = ++dp;
                buffer_remaining--;
            } else {
                
                // If the value is smaller than name, we only need
                // to move sp to past the name.
                // But if value is longer than name, we need to move
                // the remaining text so it can fit. 
                value_len = strlen(value);
                //name_len = strlen(name);
                remaining_len = strlen(sp+1);

                if (buffer_remaining < value_len + remaining_len) {
                    // We can't fit the value and the remaining text
                    return 0 - (value_len + remaining_len);
                }
                // Where are we?
                // sp is at the closing brace.
                // dp at the opening curly brace.
                // We need to shift
                // AAAAbbbCCCCC
                // the C's need to move further away.
                memmove(dp + value_len, sp+1, remaining_len + 1);
                // Now we have space between the opening curly brace
                // and the closing curly brace.
                memcpy(dp, value, value_len);
                dp += value_len;
                sp = dp;
                buffer_remaining -= value_len;
            }
        } else {
            *dp++ = *sp++;
            buffer_remaining--;
        }
    }
    if (buffer_remaining == 0) {
        return 0 - (strlen(sp) + 1);
    }
    *dp = 0;
    return buffer_len - buffer_remaining;
}
