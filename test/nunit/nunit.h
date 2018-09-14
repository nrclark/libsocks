/*******************************************************************************
 * @file nunit.h
 * @brief Definition of basic function types and assert macros. Should be
 * included in each file with tests to run.
 *
 * This file contains the prototypes and macros needed to use the nunit
 * unit-test framework.
 *
 * @copyright This file is in the public domain. Modify it, distribute it,
 * or do anything you want with it.
 ******************************************************************************/

#ifndef _NUNIT_H_
#define _NUNIT_H_

#include "_nunit_pvt.h"

typedef int (*test_t)(void);
typedef int (*setup_t)(void);
typedef int (*teardown_t)(void);

extern void nunit_config(void);

int check_zero(void *ptr, unsigned int length);
int check_mem_const(void *ptr, unsigned char value, unsigned int length);

void hexdump(const void *ptr, unsigned int len);
void hexdump_flat(const void *ptr, unsigned int len);

void register_suite(test_t *suite, char *name, setup_t setup,
                    teardown_t teardown);

#define label_test() __label_test()

#define assert_true(x) __test(x, "\n    ["xstr(x)"] not true", == 1)
#define assert_false(x) __test(x, "\n    ["xstr(x)"] not false", == 0)

#define assert_success(x) __test(x, "\n    ["xstr(x)"] failed", == 0)
#define assert_failure(x) __test(x, "\n    ["xstr(x)"] succeeded", != 0)

#define assert_nonnegative(x) __test((x), "\n    ["xstr(x)"] is negative", >= 0)
#define assert_negative(x) __test((x), "\n    ["xstr(x)"] is positive", < 0)
#define assert_nonzero(x) __test((x), "\n    ["xstr(x)"] is zero", != 0)
#endif
