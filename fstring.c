
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>

struct cfstring_parameter {
    const char *name;
    const char *value;
    void (*callback)(void *, const char *, const char *);
    void *callback_data;
};

int cfstring(char *buffer, size_t buffer_len, const char *format, struct cfstring_parameter *parameters);

int main(int argc, char *argv[]) 
{
    static char buffer[1024];
    int r, i;
    struct cfstring_parameter params[] = {
        { .name = "LOOKUP", .value="replacement"},
        { .name = "T", .value="long text"},
        { .name = "LONGLOOKUP", .value = "short"},
        { .name = NULL}        
    };
    struct {
        char *test;
        char *match;
    } tests[] = {
        { "Yet Another {LONGLOOKUP} test", "Yet Another short test" },
        { "{LONGLOOKUP}", "short" },
        { "{T}", "long text" },
        { "Testing {LOOKUP} this", "Testing replacement this" },
        { "Blah{T}XXXY", "Blahlong textXXXY" },
        { NULL, NULL }
    };
    for(i = 0; tests[i].test != NULL; i++) {
        memset(buffer, '*', sizeof(buffer));
        r = cfstring(buffer, sizeof(buffer), tests[i].test, params);
        if (r < 0) {
            printf("Test %d failed, r = %d: Input: %s", i+1, r, tests[i].test);
        } else if (strcmp(buffer, tests[i].match) != 0) {
            printf("Test %d failed match: \"%s\" didn't match \"%s\"\n", i+1, buffer, tests[i].match);
        } else {
            puts(buffer);
        }
    }
}

const char *_param_lookup(const char *name, struct cfstring_parameter *p)
{
    for(int i = 0; p && p[i].name != NULL; i++) {
        if (strcasecmp(name, p[i].name) == 0) {
            return p[i].value;
        }
    }
    return NULL;
}

int cfstring(char *buffer, size_t buffer_len, const char *format, struct cfstring_parameter *parameters)
{
    char *sp, *dp, *name;
    const char *value;
    size_t buffer_remaining, value_len, remaining_len;
    buffer_remaining = buffer_len;
    strncpy(buffer, format, buffer_len);
    sp = buffer;
    dp = buffer;
    while(*sp != 0 && buffer_remaining > 0) {
        if (*sp == '{' && *(sp+1) == '{') {
            // If it's a curly brace follow by another curlly brace, it's considered
            // escaping, so "blah {{ blah" becomes "blah { blah"
            sp++;
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
            value = _param_lookup(name, parameters);
            if (value == NULL) {
                // Lookup failed, not there, we will simply output
                // the name {NAME} (so restore that closing brace)
                *sp++ = '}';
            } else {
                
                // If the value is smaller than name, we only need
                // to move sp to past the name.
                // But if value is longer than name, we need to move
                // the remaining text so it can fit. 
                value_len = strlen(value);
                //name_len = strlen(name);
                remaining_len = strlen(sp+1);

                if (buffer_remaining < value_len + remaining_len + 1) {
                    // We can't fit the value and the remaining text
                    return -1;
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
    *dp = 0;
    return 0;
}
