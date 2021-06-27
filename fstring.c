/** @file fstring.c
 *  @brief A C version of python's fstring
 *
 *  @copyright Nick Clifford, 2021
 * 
 *  @author - Nick Clifford (nick@crypto.geek.nz)
 */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/types.h>

#include "fstring.h"

/**
 * Internal function used to lookup the value for the given name from the values list
 * Will call the callback function if provided.
 */
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

#define MAX_BUFFER_LEN      1048576
/**
 * A dynamic version of fstring, it works exactly like fstring,
 * but it is (almost) always guarenteed to return a string, as it
 * will allocate a memory buffer large enough.
 * It is up to you to free() the memory
 * It will return an error when there is an unterminated curly brace
 * or the required buffer is over 1MB.
 */
char *dfstring(const char *format, fstring_value *values)
{
    int r;
    size_t buffer_len = strlen(format) + 5; // Ensure we always have a little room.
    char *buffer = NULL;
    do {
        if (buffer) free(buffer);
        buffer_len *= 2;
        if (buffer_len > MAX_BUFFER_LEN) return NULL;
        buffer = (char *) malloc(sizeof(char) * buffer_len);
        r = fstring(buffer, buffer_len, format, values);
        if (r == -1) {
            // Special case, it's an error
            free(buffer);
            return NULL;
        }
    } while (r < -1);
    return buffer;
}


#if 0
/* See function documentation in header file */
int old_fstring(char *buffer, size_t buffer_len, const char *format, fstring_value *values)
{
    char *sp, *dp, *name;
    const char *value;
    size_t buffer_remaining, value_len, remaining_len;

    /* Shortcut to ensure that we start off on the right foot! */
    if (buffer_len < strlen(format) + 1) {
        return 0 - (strlen(format) + 1);
    }
    /*
     * We work entirely in the buffer, and not in the format string, we copy the
     * contents of format into buffer
     */
    strncpy(buffer, format, buffer_len);

    buffer_remaining = buffer_len;
    /* 
     sp is the source pointer, when we are copying the bytes from the format
     to the buffer, this is where we are up to. sp will be ahead of dp when
     an escaped curly brace is found

     dp is the destination pointer, typically it is the same as sp, but it will
     temporarily be in a different position when encountering an escaped curly 
     brace or a variable
    */

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
                // fprintf(stderr, "Unfinished curly brace\n");
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

#endif

/* See function documentation in header file */
int fstring(char *buffer, size_t buffer_len, const char *format, fstring_value *values)
{
    const char *sp;
    char *dp, *name;
    const char *value;
    size_t buffer_remaining;
    size_t remaining_len, format_len, value_len, name_len;

    format_len = strlen(format);

    /* Shortcut to ensure that we start off on the right foot! */
    if (buffer_len < format_len + 1) {
        return 0 - (format_len + 1);
    }
    /*
     * We work entirely in the buffer, and not in the format string, we copy the
     * contents of format into buffer
     */
    strncpy(buffer, format, buffer_len);

    buffer_remaining = buffer_len;
    /* 
     sp is the source pointer, when we are copying the bytes from the format
     to the buffer, this is where we are up to. sp will be ahead of dp when
     an escaped curly brace is found

     dp is the destination pointer, typically it is the same as sp, but it will
     temporarily be in a different position when encountering an escaped curly 
     brace or a variable
    */

    sp = format;
    dp = buffer;
    while(*sp != 0 && buffer_remaining > 0) {
        if (*sp == '{' && *(sp+1) == '{') {
            // If it's a curly brace follow by another curlly brace, it's considered
            // escaping, so "blah {{ blah" becomes "blah { blah"
            sp+=2;
            *dp++ = '{';
            buffer_remaining--;
        } else if (*sp == '{') {
            // We are at the beginning of a named variable. 
            // We will copy the {name} over to the dest buffer because we need
            // to null terminate it to read the actual name, as well as if the name
            // doesn't exist it needs to be there. 
            name = dp+1;
            name_len = 0;
            // Copy the name to the dest buffer, so we can null terminate it.
            while(*sp != 0 && *sp != '}' && --buffer_remaining > 0) {
                *dp++ = *sp++;
                name_len++;
            }
            if (*sp == 0) {
                // Unfished curlybrace.
                // fprintf(stderr, "Unfinished curly brace\n");
                return -1;
            } else if (buffer_remaining == 0) {
                // Ran out of room storing the name
                return 0 - strlen(sp);
            }
            
            *dp = 0; // Terminate the string so we have a name.
            value = _value_lookup(name, values);
            if (value == NULL) {
                // Lookup failed, not there, we will simply output
                // the name {NAME} (so restore that closing brace)
                *dp++ = '}';
                sp++;
                buffer_remaining--;
            } else {
                value_len = strlen(value);
                remaining_len = strlen(sp+1);
                // Where are we?
                // sp is at the closing brace.
                // name is one past the opening curly brace
                // dp at the the end of the name(where the closing brace would be).
                //
                // We've already written out the {blah}, and we 
                // will be replacing it with THEVALUE,
                // We need to fit THEVALUE and <whatever remains>
                // Move dp back to the beginning, where we overwrite the {name}
                dp = name - 1;
                sp++; // sp is now just past the curly brace
                buffer_remaining += name_len + 1;

                if (buffer_remaining < value_len + remaining_len) {
                    // We can't fit the value and the remaining text
                    return 0 - (value_len + remaining_len);
                }

                memcpy(dp, value, value_len);
                dp += value_len;
                buffer_remaining -= (name_len - value_len);
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
