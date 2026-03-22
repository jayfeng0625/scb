#ifndef TEST_H
#define TEST_H

#include <stdio.h>
#include <string.h>

extern int test_passes;
extern int test_fails;

#define ASSERT(expr) do { \
    if (expr) { test_passes++; } \
    else { test_fails++; printf("  FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); } \
} while (0)

#define ASSERT_EQ(a, b)  ASSERT((a) == (b))
#define ASSERT_STR(a, b) ASSERT(strcmp((a), (b)) == 0)

#define RUN_SUITE(fn) do { \
    int before = test_fails; \
    printf("  %s\n", #fn); \
    fn(); \
    if (test_fails == before) printf("    ok\n"); \
} while (0)

#define TEST_REPORT() do { \
    printf("\n%d passed, %d failed\n", test_passes, test_fails); \
    return test_fails > 0 ? 1 : 0; \
} while (0)

#endif
