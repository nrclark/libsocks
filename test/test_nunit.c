#include <stdio.h>
#include "nunit.h"

static int basic_test(void)
{
    assert_true(5 == 5);
    assert_false(5 == 4);
    return EXIT_SUCCESS;
}

test_t basic_suite[] = {basic_test, basic_test, NULL};

void nunit_config(void) {
    register_suite(basic_suite, "basic_suite", default_setup, default_teardown);
}
