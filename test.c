#include <stdio.h>
#include <string.h>

#include "fstring.h"


int main(int argc, char *argv[]) 
{
    static char buffer[1024];
    int r, i, total = 0, fail = 0, success = 0;
    struct cfstring_parameter params[] = {
        { .name = "LOOKUP", .value="replacement"},
        { .name = "T", .value="long text"},
        { .name = "LONGLOOKUP", .value = "short"},
        { .name = NULL}        
    };
    struct {
        char *test;
        char *match;
        int retval;
    } tests[] = {
        { "Yet Another {LONGLOOKUP} test", "Yet Another short test", 0 },
        { "{LONGLOOKUP}", "short", 0 },
        { "{T}", "long text", 0 },
        { "Testing {LOOKUP} this", "Testing replacement this", 0 },
        { "Blah{T}XXXY", "Blahlong textXXXY", 0 },
        { "No such {MACRO} found", "No such {MACRO} found", 0},
        { NULL, NULL }
    };
    for(i = 0; tests[i].test != NULL; i++) {
        memset(buffer, '*', sizeof(buffer));
        total++;
        r = cfstring(buffer, sizeof(buffer), tests[i].test, params);
        if (r < 0) {
            printf("%d: FAIL: Got error code %d, input: %s", i+1, r, tests[i].test);
            fail++;
        } else if (strcmp(buffer, tests[i].match) != 0) {
            printf("%d: FAIL: Failed match: \"%s\" didn't match \"%s\"\n", i+1, buffer, tests[i].match);
            fail++;
        } else if (r != strlen(tests[i].match)) {
            printf("%d: FAIL: Returned %d but len should have been %lu\n", i, r, strlen(tests[i].match));
            fail++;
        } else {
            printf("%d: PASS: %s\n", i + 1, buffer);
            success++;
        }
    }
    printf("\nAnd now testing failure conditions:\n\n");
    // Now test failures
    struct {
        char *test;
        int buff_len;
        int retval;
    } fail_tests[] = {
        { "This shall not fit at all blah", 10, -1 },
        { "{LONGLOOKUP}", 0, -1 },
        { NULL, 0, 0 }
    };    
    for(i = 0; fail_tests[i].test != NULL; i++) {
        memset(buffer, '*', sizeof(buffer));
        total++;
        r = cfstring(buffer, fail_tests[i].buff_len, fail_tests[i].test, params);
        if (r != fail_tests[i].retval) {
            printf("%d: FAIL: retval was %d, expecting %d\n", i + 1, r, fail_tests[i].retval);
            fail++;
        } else {
            printf("%d: PASS: %d returned correctly\n", i + 1, r);
            success++;
        }
    }
    if (fail == 0) {
        printf("\n[%d/%d] All tests passed succesfully\n", success, total);
    } else {
        printf("\nSome tests failed: Failed=%d passed=%d total=%d\n", fail, success, total);
    }
    return 0;
}
