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
const char *_value_lookup(const char *name, fstr_value *values[])
{
    int i;
    static char tmpbuff[128];
    fstr_value *val;
    for(i = 0; values && values[i] != NULL && values[i]->name != NULL; i++) {
        val = values[i];
        if (strcasecmp(name, val->name) == 0 || (val->name[0] == '*' && val->name[1] == 0)) {
            switch(val->type) {
            case fstr_vt_str: 
                return val->value.s; 
            case fstr_vt_int:
                //ntoa is not standard
                snprintf(tmpbuff, sizeof(tmpbuff), "%d", val->value.i);
                return tmpbuff;
            case fstr_vt_cb:
                return (val->value.cb)(val->cb_data, name);
            }
            return values[i]->value.s;
        }
    }
    return NULL;
}

void _debug_dump_values(fstr_value *values[])
{
    int i;
    static char tmpbuff[128];
    const char *str;
    fstr_value *val;
    printf("Dumping values list\n");
    for(i = 0; values && values[i] != NULL && values[i]->name != NULL; i++) {
        val = values[i];
        switch(val->type) {
            case fstr_vt_str: 
                str = val->value.s; 
                break;
            case fstr_vt_int:
                //ntoa is not standard
                snprintf(tmpbuff, sizeof(tmpbuff), "%d", val->value.i);
                str = tmpbuff;
                break;
            case fstr_vt_cb:
                str = (val->value.cb)(val->cb_data, val->name);
                break;
            default:
                snprintf(tmpbuff, sizeof(tmpbuff), "INVALID TYPE %d", val->type);
        }
        printf("#%d: %s type %d: val: %s\n", i, val->name, val->type, str);
    }
    printf("End of list\n");
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
#if 0
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
        r = bfstring(buffer, buffer_len, format, values);
        if (r == -1) {
            // Special case, it's an error
            free(buffer);
            return NULL;
        }
    } while (r < -1);
    return buffer;
}
#endif 

int bfstring(char *buffer, size_t buffer_len, const char *format, ...)
{
    int r, list_size = 10, l_count = 0;
    va_list vl;
    fstr_value *val;
    fstr_value **list = calloc(sizeof(fstr_value *),list_size+1);
    va_start(vl, format);
    do {
        if (l_count == list_size) {
            list_size *= 2;
            list = realloc(list, sizeof(fstr_value *) * (list_size + 1));
        }
        val = va_arg(vl, fstr_value *);
        list[l_count] = val;
        l_count++;
    } while(val != NULL);
    va_end(vl);
    //_debug_dump_values(list);
    r = blfstring(buffer, buffer_len, format, list);
    free(list);
    return r;
}

/* Buffered fstring.*/
int blfstring(char *buffer, size_t buffer_len, const char *format, fstr_value *values[])
{
    const char *sp;
    char *dp, *name;
    const char *value;
    size_t buffer_remaining;
    size_t remaining_len, value_len, name_len;

    if (buffer_len == 0) {
        return 0 - strlen(format) - 1;
    }
    if (*format == 0) {
        *buffer = 0;
        return 0;
    }
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
            // We are copying the entire {blah}
            while(*sp != 0 && *sp != '}' && --buffer_remaining > 0) {
                *dp++ = *sp++;
            }
            name_len = (dp - name) + 1;
            if (*sp == 0) {
                // Unfished curlybrace.
                //fprintf(stderr, "Unfinished curly brace\n");
                return -1;
            } else if (buffer_remaining == 0) {
                // Ran out of room storing the name
                //fprintf(stderr,"Ran out of memory storing the name\n");
                return 0 - buffer_len - strlen(sp);
            }
            
            *dp = 0; // Terminate the string so we have a name.
            // Dest now looks like xxxxxx{blah\0
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
                // sp is at the closing curly brace.
                // name is one past the opening curly brace
                // dp at the the end of the name(where the closing brace would be, and is currently \0).
                //
                // We've already written out the {blah}, and we will be replacing it with THEVALUE,
                // We need to fit THEVALUE and <whatever remains>
                // Move dp back to the beginning, where we overwrite the {name}
                dp = name - 1; // name was at the start of {name} (just passed the curly brace)
                sp++; // sp is now just past the curly brace in the source, so at the "remaining"
                // We decremented the buffer remaining whilst we were copying {name} to the
                // destination, now we are about to replace with THEVALUE, so reset buffer_remaining
                buffer_remaining += name_len ;

                if (buffer_remaining < value_len + remaining_len) {
                    // We can't fit the value and the remaining text
                    return 0 - (value_len + remaining_len);
                }
                // Copy THEVALUE to the dest
                memcpy(dp, value, value_len);
                dp += value_len;
                buffer_remaining -= value_len;
            }
        } else {
            *dp++ = *sp++;
            buffer_remaining--;
        }
    }
    if (buffer_remaining == 0) {
        // We need all the space we had, plus the remainder of text, at least.
        return 0 - buffer_len - (strlen(sp) + 1);
    }
    *dp = 0;
    return buffer_len - buffer_remaining;
}
