/** @file fstring.c
 *  @brief A C version of python's fstring
 *
 *  @copyright Nick Clifford, 2021
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

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/types.h>

#include "fstring.h"


/* The maximum size of a buffer that will be allocated by fstring, vfstring or lfstring */
#define MAX_BUFFER_LEN      1048576


fstr_value **_va_to_list(fstr_value *first, va_list vl);


/**
 * @brief Internal function used to lookup the value for the given name from the values list
 * 
 * Will call the callback function if provided.
 * 
 * @return Returns the value for that name, or NULL if not found.
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
                snprintf(tmpbuff, sizeof(tmpbuff), "%d", val->value.i);
                return tmpbuff;
            case fstr_vt_long:
                snprintf(tmpbuff, sizeof(tmpbuff), "%ld", val->value.l);
                return tmpbuff;
            case fstr_vt_float:
                snprintf(tmpbuff, sizeof(tmpbuff), "%f", val->value.f);
                return tmpbuff;
            case fstr_vt_double:
                snprintf(tmpbuff, sizeof(tmpbuff), "%lf", val->value.d);
                return tmpbuff;
            case fstr_vt_cb:
                return (val->value.cb)(val->cb_data, name);
            default:
                fprintf(stderr, "Unknown value type\n");
                return NULL;
            }
        }
    }
    return NULL;
}


/**
 * @brief Internal debug function, prints the contents of a value list
 * 
 */
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


fstr_value **_va_to_list(fstr_value *first, va_list vl)
{
    int list_size = 10, l_count = 0;
    fstr_value *val;
    fstr_value **list = calloc(sizeof(fstr_value *),list_size+1);

    if (first) list[l_count++] = first;
    do {
        if (l_count == list_size) {
            list_size *= 2;
            list = realloc(list, sizeof(fstr_value *) * (list_size + 1));
        }
        val = va_arg(vl, fstr_value *);
        list[l_count] = val;
        l_count++;
    } while(val != NULL);
    //_debug_dump_values(list);
    return list;
}


char *fstring(const char *format, fstr_value *first, ...)
{
    char *r;
    va_list vl;
    fstr_value **list;
    va_start(vl, first);
    list = _va_to_list(first, vl);    
    va_end(vl);
    r = lfstring(format, list);
    free(list);
    return r;
}


char *vfstring(const char *format, va_list vl)
{
    fstr_value **list = _va_to_list(NULL, vl);    
    char *r = lfstring(format, list);
    free(list);
    return r;
}


char *lfstring(const char *format, fstr_value **list)
{
    int r;
    size_t buffer_len = strlen(format) + 20; // Ensure we always have a little room.
    char *buffer = NULL;

    if (*format == 0) return strdup("");

    while(buffer_len < MAX_BUFFER_LEN) {
        buffer = (char *)malloc(sizeof(char) * buffer_len);
    
        r = lbfstring(buffer, buffer_len, format, list);
    
        if (r > 0) {
            return buffer;
        } else if (r == 0 || r == -1) {
            // Both of these results indicate an error condition.
            return NULL;
        } else if (r < 0) {
            /* Need to resize. We coud realloc, but that copys the buffer, which we don't need. */
            free(buffer);
            r *= -1; /* Make positive number */
            if (r < buffer_len) {
                /* Weird, we were asked to supply a smaller buffer. */
                fprintf(stderr, "Asked to supply a smaller buffer. Gave %lu got %d?!\n", buffer_len, r);
                buffer_len *= 2;
            } else {
                buffer_len = r * 2;
            }
        }
    }
    fprintf(stderr, "fstring.c: Maximum buffer exceeded: %lu\n", buffer_len);
    return NULL;
}


int bfstring(char *buffer, size_t buffer_len, const char *format, fstr_value *first, ...)
{
    /* What's with the "first" arg? It's just to ensure that people don't call
     * bfstring(buff, l, "blah", fstr_list )
     * when the should have called
     * lbfstring(buff, l, "blah", fstr_list )
     */
    int r;
    va_list vl;
    fstr_value **list;
    va_start(vl, first);
    
    list = _va_to_list(first, vl);    
    va_end(vl);
    r = lbfstring(buffer, buffer_len, format, list);
    free(list);
    return r;
}


int vbfstring(char *buffer, size_t buffer_len, const char *format, va_list vl)
{
    int r;
    fstr_value **list = _va_to_list(NULL, vl);    
    r = lbfstring(buffer, buffer_len, format, list);
    free(list);
    return r;
}


int lbfstring(char *buffer, size_t buffer_len, const char *format, fstr_value *values[])
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
