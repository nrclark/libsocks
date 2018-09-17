/*******************************************************************************
 * @file nunit.c
 * @brief Implementation of nunit functions, and nunit main().
 *
 * This file contains the functions used for running unit-tests. Most of
 * the functions in here don't need to be called manually (except for
 * register_suite(), which should be alled from within the externally-defined
 * 'nunit_config' function.
 *
 * @copyright This file is in the public domain. Modify it, distribute it,
 * or do anything you want with it.
 ******************************************************************************/

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "nunit.h"

typedef struct full_suite_t {
    test_t *suite;
    teardown_t teardown;
    setup_t setup;
    char name[];
} full_suite_t;

static full_suite_t **suites = NULL;
static volatile unsigned int suite_count = 0;

__attribute__ ((weak)) void nunit_config(void)
{
    fprintf(stderr, "nunit_config() not defined! No tests were executed.\n");
}

int check_zero(void *ptr, unsigned int length)
{
    return check_mem_const(ptr, 0, length);
}

int check_mem_const(void *ptr, unsigned char value, unsigned int length)
{
    unsigned char *temp = (unsigned char *) ptr;

    while (length-- != 0) {
        if (*(temp++) != value) {
            return -1;
        }
    }
    return 0;
}

void hexdump_flat(const void *ptr, unsigned int len)
{
    const unsigned char *data = (const unsigned char *)ptr;
    printf("{");
    for (unsigned int x = 0; x < len; x++) {
        printf("0x%02X,", data[x]);
    }
    printf("}\n");
}

void hexdump(const void *ptr, unsigned int len)
{
    const unsigned char *data = (const unsigned char *)ptr;

    for (unsigned int x = 0; x < len; x++) {
        printf("%02X", data[x]);

        if (((x + 1) % 8) == 0) {
            printf(" | ");
            for (unsigned int k = x - 7; k <= x; k++) {
                unsigned char output = data[k];
                if ((output < 0x20) || (output > 0x7E)) {
                    output = '.';
                }
                printf("%c", output);
            }
            printf("\n");
        } else if (x != (len - 1)) {
            printf(" ");
        }
    }

    int remainder = (len % 8);

    if (remainder != 0) {
        for (unsigned short x = 8; x > remainder; x--) {
            printf("   ");
        }
        printf(" | ");
        for (unsigned int k = len - (len % 8); k < len; k++) {
            unsigned char output = data[k];
            if ((output < 0x20) || (output > 0x7E)) {
                output = '.';
            }
            printf("%c", output);
        }
        printf("\n");
    }
}

void register_suite(test_t *suite, const char *name, setup_t setup,
                    teardown_t teardown)
{
    if (name == NULL) {
        name = "";
    }

    unsigned int name_length = (unsigned int) strnlen(name, 256);

    suites = (full_suite_t **)
             realloc(suites, (suite_count + 1) * sizeof(full_suite_t *));

    suites[suite_count] = (full_suite_t *)
                          malloc(sizeof(full_suite_t) + name_length + 1);

    suites[suite_count]->suite = suite;
    suites[suite_count]->setup = setup;
    suites[suite_count]->teardown = teardown;
    strcpy(suites[suite_count]->name, name);

    suite_count++;
}

static void cleanup(void)
{
    if (suites == NULL) {
        return;
    }

    for (unsigned int x = 0; x < suite_count; x++) {
        if (suites[x] != NULL) {
            free(suites[x]);
        }
    }

    free(suites);
}

static int suite_runner(test_t *suite, const char *name, setup_t setup,
                        teardown_t teardown)
{
    int result = EXIT_SUCCESS;
    int successes = 0;
    int failures = 0;

    if (name == NULL) {
        name = "";
    }

    for ((void) suite; *suite != NULL; suite++) {
        bool failed = false;

        if (setup != NULL) {
            result = setup();

            if (result != EXIT_SUCCESS) {
                fprintf(stderr, "setup failed\n");
                failures += (failed == false);
                failed = true;
            }
        }

        if (failed == false) {
            result = (*suite)();

            if (result != EXIT_SUCCESS) {
                failures += (failed == false);
                failed = true;
            }
        }

        if (teardown != NULL) {
            result = teardown();

            if (result != EXIT_SUCCESS) {
                fprintf(stderr, "teardown failed\n");
                failures += (failed == false);
                failed = true;
            }
        }

        successes += (failed == false);
    }

    if (failures != 0) {
        if (name[0] != 0) {
            fprintf(stderr, "Test set [%s] FAILED ", name);
        } else {
            fprintf(stderr, "Test set FAILED ");
        }

        fprintf(stderr, "(%d successes, %d failure)\n", successes, failures);
        return -failures;
    }

    if (name[0] != 0) {
        fprintf(stdout, "Test set [%s] OK ", name);
    } else {
        fprintf(stdout, "Test set OK ");
    }

    fprintf(stdout, "(%d successes, %d failure)\n", successes, failures);
    return EXIT_SUCCESS;
}

int main(void)
{
    nunit_config();
    int errcode = 0;
    int passes = 0;
    int fails = 0;

    for (unsigned int x = 0; x < suite_count; x++) {
        int result = suite_runner(suites[x]->suite, suites[x]->name,
                                  suites[x]->setup, suites[x]->teardown);

        passes += (result == EXIT_SUCCESS);
        fails += (result != EXIT_SUCCESS);
    }
    printf("Test suites run: %u. (suites passed: %d, suites failed: %d)\n",
           suite_count, passes, fails);

    cleanup();
    return errcode;
}
