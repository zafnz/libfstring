/*
 * Copyright Nick Clifford, 2021
 * 
 * Nick Clifford (nick@crypto.geek.nz)
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
 * 
 */
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <stdarg.h>

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

#define S_PASS "\x1b[32mPASS\x1b[0m"
#define S_FAIL "\x1b[31mFAIL\x1b[0m"

#define TEST_DECLARE()    int total = 0, pass = 0, fail = 0; char *test_name
#define TEST_NAME(NAME) do { test_name = NAME; total++; } while(0)

#define TEST_ASSERT(TEST)       do {  \
                    if (TEST) { \
                        pass++; \
                        printf("%02d: " S_PASS ": %s\n", total, test_name); \
                    } else { \
                        fail++; \
                        printf("%02d: " S_FAIL ": %s assertion failed: %s\n", total, test_name, #TEST); \
                    } } while(0)

#define TEST_RESULTS()     do { \
                    if (fail == 0) { \
                        printf("\n\n[%d/%d] \x1b[32mAll tests passed succesfully\x1b[0m\n", pass, pass); \
                    } else { \
                        printf("\n\n\x1b[31mSome tests failed\x1b[0m: Failed=%d passed=%d total=%d\n", fail, pass, total); \
                    } } while(0)


#define PERF_TEST_COUNT     50000000L

void performance_test()
{
    static char buffer[1024];
    int i;
    struct timeval tv_start, tv_end;
    u_int64_t usec;

    printf("Starting perf test\n");
    gettimeofday(&tv_start, NULL);
    for(i = PERF_TEST_COUNT; --i > 0;) {
       lbfstring(buffer, sizeof(buffer), "test {blah} thing {BLAH} {thing} testing one two three", fstr_values_cast {
            fstr_nstr("blah", "TEST"),
            fstr_end
        });
    }
    gettimeofday(&tv_end, NULL);

    usec = (1000000 * (tv_end.tv_sec - tv_start.tv_sec)) + (tv_end.tv_usec - tv_start.tv_usec);
    printf("%ld interations. Elapsed time: %lu us (%lu.%06lu seconds)\n", PERF_TEST_COUNT, usec,
            usec / 1000000, usec % 1000000);
    return;
}


int calling_test1(char *vl_match, char *str, ...);
int calling_test2(char *vl_match, char *testing, va_list vl);


/* Called with fstr_val_str("boo", "arg"), fstr_val_int("agent", 99), fstr_val_end */
int calling_test() {
    return calling_test1(
        "boom arg 99",
        "boom {boo} {agent}", 
        fstr_nstr("boo", "arg"), fstr_nint("agent", 99), fstr_end);
}
int calling_test1(char *vl_match, char *str, ...)
{
    va_list vl;
    int r;
    va_start(vl, str);
    r = calling_test2(vl_match, str, vl);
    va_end(vl);
    return r;
}

int calling_test2(char *vl_match, char *testing, va_list vl)
{
    /* There are many ways to call fstring, lets try them all! 

extern char *fstring(const char *format, fstr_value *, ...);
extern char *vfstring(const char *format, va_list vl);
extern char *lfstring(const char *format, fstr_value *values[]);

extern int bfstring(char *buffer, size_t buffer_len, const char *format, fstr_value *, ...);
extern int vbfstring(char *buffer, size_t buffer_len, const char *format, va_list vl);
extern int lbfstring(char *buffer, size_t buffer_len, const char *format, fstr_value *values[]);

     */

    char *str1 = "Testing";
    char *str2 = "Blah";
    int x = 10;
    int y = 12;
    float f = 3.1415;
    double d = 3.333333;
    int r;
    va_list vl_test1, vl_test2;
    fstr_value *variables[] = {
        fstr_int(x),
        fstr_str(str2),
        fstr_str(str1),
        fstr_ndouble("blah", 1.123456),
        fstr_end
    };
    static char buffer[1024];
    TEST_DECLARE();

    char *result;


    TEST_NAME("fstring()");
    result = fstring("Testing {str1}", fstr_str(str1), fstr_str(str2), fstr_end);
    TEST_ASSERT(result != NULL);
    TEST_ASSERT(strcmp(result, "Testing Testing") == 0);

    TEST_NAME("vfstring()");
    va_copy(vl_test1, vl);
    result = vfstring(testing, vl_test1);
    TEST_ASSERT(result != NULL);
    TEST_ASSERT(strcmp(result, vl_match) == 0);

    TEST_NAME("lfstring()");
    result = lfstring("Another {f} test {y}", fstr_values_cast {
        fstr_str(str2), fstr_float(f), fstr_int(y), fstr_double(d), fstr_end
    });
    TEST_ASSERT(result != NULL);
    TEST_ASSERT(strcmp(result, "Another 3.141500 test 12") == 0);

    TEST_NAME("bfstring()");
    r = bfstring(buffer, sizeof(buffer), "Testing {x} {str2}", fstr_str(str2), fstr_int(x), fstr_end);
    TEST_ASSERT(r > 0);
    TEST_ASSERT(strcmp(result, "Testing 12 str2"));

    TEST_NAME("vbfstring()");
    va_copy(vl_test2, vl);
    r = vbfstring(buffer, sizeof(buffer), testing, vl_test2);
    TEST_ASSERT(r > 0);
    TEST_ASSERT(strcmp(result, vl_match));

    TEST_NAME("lbfstring()");
    r = lbfstring(buffer, sizeof(buffer), "Testing {blah} {str2} {str1} {x} {missing}", variables);
    TEST_ASSERT(r > 0);
    TEST_ASSERT(strcmp(result, "Testing 1.123456 Testing Blah 12 "));

    TEST_RESULTS();

    return total;
}


int main(int argc, char *argv[]) 
{
    static char buffer[1024], compare[1024];
    int r, i, total = 0, fail = 0, success = 0;
    char *thing = "magic thingy";

    total++;

    /* Example showing the quick and easy way to call lbfstring */
    r = lbfstring(buffer, sizeof(buffer), "test {total}: {blah} {thing} {boo} blah", fstr_values_cast {
        fstr_int(total),
        fstr_nstr("blah", "TEST"),
        fstr_str(thing),
        fstr_end
    });
    if (r < 0) {
        printf("%d: "S_FAIL": Quick test failed :(, return: %d\n", total, r);
        fail++;
    } else {
        printf("%d: "S_PASS": Sanity check passed: %s\n", total, buffer);
        success++;
    }
    /* values to pass to our tests */
    fstr_value *params[] = {
        fstr_nstr("LOOKUP", "replacement"),
        fstr_nstr("T", "long string"),
        fstr_nstr("SHORT", "sh"),
        fstr_nstr("LONGLOOKUP", "short"),
        fstr_ncb("CALLBACK", test_callback1, data_check),
        fstr_ncb("CB2", test_callback1, NULL),
        fstr_end       
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
        { "Testing {{ blah", "Testing { blah", 14 },
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
        r =lbfstring(buffer, sizeof(buffer), tests[i].test, params);
        if (r < 0) {
            printf("%d: "S_FAIL": Got error code %d, input: %s\n", total, r, tests[i].test);
            fail++;
        } else if (strcmp(buffer, tests[i].match) != 0) {
            printf("%d: "S_FAIL": Failed match: \"%s\" didn't match \"%s\"\n", total, buffer, tests[i].match);
            fail++;
        } else if (tests[i].retval != 0 && r != tests[i].retval) {
            printf("%d: "S_FAIL": Returned %d but len should have been %u: %s\n", total, r, tests[i].retval, buffer);
            fail++;            
        } else if (tests[i].retval == 0 && r != strlen(tests[i].match)) {
            printf("%d: "S_FAIL": Returned %d but strlen should have been %lu: %s\n", total, r, strlen(tests[i].match), buffer);
            fail++;
        } else {
            printf("%d: "S_PASS": %s\n", total, buffer);
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
        r =lbfstring(buffer, fail_tests[i].buff_len, fail_tests[i].test, params);
        if (r != fail_tests[i].retval) {
            printf("%d: "S_FAIL": retval was %d, expecting %d\n", total, r, fail_tests[i].retval);
            fail++;
        } else {
            printf("%d: "S_PASS": %d returned correctly\n", total, r);
            success++;
        }
    }

    printf("\nAnd now testing some edge cases\n");

    total++;
    memset(buffer, '*', sizeof(buffer));
    r =lbfstring(buffer, 8, "{SHORT}", params);
    if (r != 2) {
        total++;
        printf("%d: "S_FAIL": short template overrun returned %d\n", total, r);
        fail++;
    } else if (buffer[8] != '*') {
        printf("%d: "S_FAIL": Wrote past buffer size!", total);
        fail++;        
    } else {
        printf("%d: "S_PASS": short template overrun returned correctly %d\n", total, r);
        success++;
    }

    total++;
    r =lbfstring(buffer, 2, "{T}", params);
    if (r != -4) {
        total++;
        printf("%d: "S_FAIL": short template overrun returned %d\n", total, r);
        fail++;
    } else {
        printf("%d: "S_PASS": short template overrun returned correctly %d\n", total, r);
        success++;
    }
    total++;
    memset(buffer, '*', sizeof(buffer));
    /* {T} is 10 charcters long */
    r = lbfstring(buffer, 5, "{T}", params);
    /* So we should need 11 bytes to store it */
    if (r != -11) {
        printf("%d: "S_FAIL": template overrun returned %d\n", total, r);
        fail++;
    } else if (buffer[5] != '*') {

        printf("%d: "S_FAIL": Wrote past buffer size!", total);
        fail++;
    } else {
        printf("%d: "S_PASS": template overrun returned correctly %d\n", total, r);
        success++;
    }

    total++;
    r =lbfstring(buffer, sizeof(buffer), "test {XYZ}", (fstr_value *[]) {
        fstr_nstr("blah","TEST"),
        fstr_nstr("*", "Wildcard"),
        fstr_end
    });   
    if (r < 0) {
        printf("%d: "S_FAIL": Wildcard returned error: %d\n", total, r);
        fail++;
    } else if (r != 13) {
        printf("%d: "S_FAIL": Wildcard returned bad length %d: %s\n", total, r, buffer);
        fail++;
    } else if (strcmp(buffer, "test Wildcard") != 0) {
        printf("%d: "S_FAIL": Wildcard string doesn't match '%s' vs '%s'\n", total, buffer, "test Wildcard");
        fail++;
    } else {
        printf("%d: "S_PASS": Wildcard test passed: %s\n", total, buffer);
        success++;
    }

    /* And now the different ways of calling fstring */

    total++;
    snprintf(compare, sizeof(compare), "We've had %d successful tests, and %d failures with this %s.", success, fail, thing);
    r = bfstring(buffer, sizeof(buffer), "We've had {success} successful tests, and {fail} failures with this {thing}.", 
        fstr_str(thing), 
        fstr_int(success),
        fstr_int(fail),
        fstr_end
    );
    if (r <= 0) {
        printf("%d: "S_FAIL": bfstring call returned error %d", total, r);
        fail++;
    } else if (strcmp(compare, buffer) != 0) {
        printf("%d: "S_FAIL": bfstring call failed. Returned unexpected.\nExpected:%s\nGot:%s\n", total, compare, buffer);
        fail++;
    } else {
        printf("%d: "S_PASS": bfstring call passed: %s", total, buffer);
        success++;
    }

    /* End of checks */

    if (fail == 0) {
        printf("\n\n[%d/%d] \x1b[32mAll tests passed succesfully\x1b[0m\n", success, total);
    } else {
        printf("\n\n\x1b[31mSome tests failed\x1b[0m: Failed=%d passed=%d total=%d\n", fail, success, total);
    }

    printf("\n\nCalling tests\n\n");
    calling_test();

    if (argc > 1 && strcmp(argv[1], "performance") == 0) {
        performance_test();
    }
    return 0;
}
