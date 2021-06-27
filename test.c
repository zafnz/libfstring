/*
 *  Copyright Nick Clifford, 2021
 * 
 *  Nick Clifford (nick@crypto.geek.nz)
 */
#include <stdio.h>
#include <string.h>

#include "fstring.h"

char data_check[4] = "Boo";

const char *test_callback1(void *data, const char *name)
{
    if (strcmp(name, "CALLBACK") != 0) {
        return "Wrong name passed";
    }
    if (data != data_check) {
        return "Bad data passed";
    }
    return "PASS";
}


int main(int argc, char *argv[]) 
{
    static char buffer[1024];
    int r, i, total = 0, fail = 0, success = 0;

    /* Example showing the quick and easy way to call fstring */
    fstring(buffer, sizeof(buffer), "test {blah}", (fstring_value[]) {
        {.name="blah", .value="TEST"},
        {.name=NULL}
    });

    /* values to pass to our tests */
    fstring_value params[] = {
        { .name = "LOOKUP", .value="replacement"},
        { .name = "T", .value="long string"},
        { .name = "SHORT", .value="sh" },
        { .name = "LONGLOOKUP", .value = "short"},
        { .name = "CALLBACK", .value = NULL, .callback = test_callback1, .callback_data = data_check },
        { .name = "CB2", .value = NULL, .callback = test_callback1, .callback_data = NULL },
        { .name = NULL}        
    };

    /* Success tests */
    struct {
        char *test;
        char *match;
        int retval;
    } tests[] = {
        { "Yet Another {LONGLOOKUP} test", "Yet Another short test" },
        { "{LONGLOOKUP}", "short" },
        { "{T}", "long string" },
        { "Testing {{ blah", "Testing { blah", 13 },
        { "a{SHORT}b", "ashb" },
        { "Testing {LOOKUP} this", "Testing replacement this" },
        { "Blah{T}XXXY", "Blahlong stringXXXY" },
        { "No such {MACRO} found", "No such {MACRO} found"},
        { "Callback test: {CALLBACK}", "Callback test: PASS"},
        { "CB: {CALLBACK}", "CB: PASS"},
        { "CB: {CB2}", "CB: Wrong name passed"},
        { NULL, NULL }
    };
    for(i = 0; tests[i].test != NULL; i++) {
        memset(buffer, '*', sizeof(buffer));
        total++;
        r = fstring(buffer, sizeof(buffer), tests[i].test, params);
        if (r < 0) {
            printf("%d: FAIL: Got error code %d, input: %s\n", total, r, tests[i].test);
            fail++;
        } else if (strcmp(buffer, tests[i].match) != 0) {
            printf("%d: FAIL: Failed match: \"%s\" didn't match \"%s\"\n", total, buffer, tests[i].match);
            fail++;
        } else if (tests[i].retval != 0 && r != tests[i].retval) {
            printf("%d: FAIL: Returned %d but len should have been %u: %s\n", total, r, tests[i].retval, buffer);
            fail++;            
        } else if (tests[i].retval == 0 && r != strlen(tests[i].match)) {
            printf("%d: FAIL: Returned %d but strlen should have been %lu: %s\n", total, r, strlen(tests[i].match), buffer);
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
        { "This shall not fit at all blah", 10, -31 }, // 30 characters
        { "{LONGLOOKUP}", 0, -13 }, // 12 characters
        { "a", 0, -2 },
        { "x", 1, -2 },
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

    printf("\nAnd now testing some edge cases\n");

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
    if (r != -4) {
        total++;
        printf("%d: FAIL: short template overrun returned %d\n", total, r);
        fail++;
    } else {
        printf("%d: PASS: short template overrun returned correctly %d\n", total, r);
        success++;
    }
    total++;
    memset(buffer, '*', sizeof(buffer));
    /* {T} is 10 charcters long */
    r = fstring(buffer, 5, "{T}", params);
    /* So we should need 11 bytes to store it */
    if (r != -11) {
        printf("%d: FAIL: template overrun returned %d\n", total, r);
        fail++;
    } else if (buffer[5] != '*') {
        printf("%d: FAIL: Wrote past buffer size!", total);
        fail++;
    } else {
        printf("%d: PASS: template overrun returned correctly %d\n", total, r);
        success++;
    }

    total++;
    r = fstring(buffer, sizeof(buffer), "test {XYZ}", (fstring_value[]) {
        {.name="blah", .value="TEST"},
        {.name="*", .value="Wildcard"},
        {.name=NULL}
    });   
    if (r < 0) {
        printf("%d: FAIL: Wildcard returned error: %d\n", total, r);
        fail++;
    } else if (r != 13) {
        printf("%d: FAIL: Wildcard returned bad length %d: %s\n", total, r, buffer);
        fail++;
    } else if (strcmp(buffer, "test Wildcard") != 0) {
        printf("%d: FAIL: Wildcard string doesn't match '%s' vs '%s'\n", total, buffer, "test Wildcard");
        fail++;
    } else {
        printf("%d: PASS: Wildcard test passed: %s\n", total, buffer);
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
