#include <stdio.h>
#include <string.h>

#include "fstring.h"


int main(int argc, char *argv[]) 
{
    static char buffer[1024];
    int r, i, total = 0, fail = 0, success = 0;

    /* Example showing the quick and easy way to call fstring */
    fstring(buffer, sizeof(buffer), "test {blah}", (fstring_value[]) {
        {.name="blah", .value="TEST"}
    });

    /* values to pass to our tests */
    fstring_value params[] = {
        { .name = "LOOKUP", .value="replacement"},
        { .name = "T", .value="long text"},
        { .name = "SHORT", .value="sh" },
        { .name = "LONGLOOKUP", .value = "short"},
        { .name = NULL}        
    };

    /* Success tests */
    struct {
        char *test;
        char *match;
    } tests[] = {
        { "Yet Another {LONGLOOKUP} test", "Yet Another short test" },
        { "{LONGLOOKUP}", "short" },
        { "{T}", "long text" },
        { "a{SHORT}b", "ashb" },
        { "Testing {LOOKUP} this", "Testing replacement this" },
        { "Blah{T}XXXY", "Blahlong textXXXY" },
        { "No such {MACRO} found", "No such {MACRO} found"},
        { NULL, NULL }
    };
    for(i = 0; tests[i].test != NULL; i++) {
        memset(buffer, '*', sizeof(buffer));
        total++;
        r = fstring(buffer, sizeof(buffer), tests[i].test, params);
        if (r < 0) {
            printf("%d: FAIL: Got error code %d, input: %s", total, r, tests[i].test);
            fail++;
        } else if (strcmp(buffer, tests[i].match) != 0) {
            printf("%d: FAIL: Failed match: \"%s\" didn't match \"%s\"\n", total, buffer, tests[i].match);
            fail++;
        } else if (r != strlen(tests[i].match)) {
            printf("%d: FAIL: Returned %d but len should have been %lu\n", total, r, strlen(tests[i].match));
            fail++;
        } else {
            printf("%d: PASS: %s\n", total, buffer);
            success++;
        }
    }

    printf("\nAnd now testing failure conditions:\n\n");
    /* Test failures */
    struct {
        char *test;
        int buff_len;
        int retval;
    } fail_tests[] = {
        { "This shall not fit at all blah", 10, -1 },
        { "{LONGLOOKUP}", 0, -1 },
        { "a", 0, -1 },
        { "x", 1, -1 },
        { "y", 2, 1 },
        { NULL, 0, 0 }
    };    
    for(i = 0; fail_tests[i].test != NULL; i++) {
        memset(buffer, '*', sizeof(buffer));
        total++;
        r = fstring(buffer, fail_tests[i].buff_len, fail_tests[i].test, params);
        if (r != fail_tests[i].retval) {
            printf("%d: FAIL: retval was %d, expecting %d\n", total, r, fail_tests[i].retval);
            fail++;
        } else {
            printf("%d: PASS: %d returned correctly\n", total, r);
            success++;
        }
    }
    printf("\n");

    total++;
    memset(buffer, '*', sizeof(buffer));
    r = fstring(buffer, 8, "{SHORT}", params);
    if (r != 2) {
        total++;
        printf("%d: FAIL: short template overrun returned %d\n", total, r);
        fail++;
    } else if (buffer[8] != '*') {
        printf("%d: FAIL: Wrote past buffer size!", total);
        fail++;        
    } else {
        printf("%d: PASS: short template overrun returned correctly %d\n", total, r);
        success++;
    }

    total++;
    r = fstring(buffer, 2, "{T}", params);
    if (r != -1) {
        total++;
        printf("%d: FAIL: short template overrun returned %d\n", total, r);
        fail++;
    } else {
        printf("%d: PASS: short template overrun returned correctly %d\n", total, r);
        success++;
    }
    total++;
    memset(buffer, '*', sizeof(buffer));
    r = fstring(buffer, 5, "{T}", params);
    if (r != -1) {
        printf("%d: FAIL: template overrun returned %d\n", total, r);
        fail++;
    } else if (buffer[5] != '*') {
        printf("%d: FAIL: Wrote past buffer size!", total);
        fail++;
    } else {
        printf("%d: PASS: template overrun returned correctly %d\n", total, r);
        success++;
    }
    /* End of checks */

    if (fail == 0) {
        printf("\n\n[%d/%d] All tests passed succesfully\n", success, total);
    } else {
        printf("\n\nSome tests failed: Failed=%d passed=%d total=%d\n", fail, success, total);
    }
    return 0;
}
