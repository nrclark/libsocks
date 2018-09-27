#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "nunit.h"
#include "libsocks_dirs.h"

char *getcwd(char *buf, size_t size);

char first_buffer[PATH_MAX + 1] = {0};
char second_buffer[PATH_MAX + 1] = {0};
char third_buffer[PATH_MAX + 1] = {0};

int socks_store_cwd(void);

int directory_test()
{
    label_test();

    getcwd(first_buffer, PATH_MAX);
    assert_success(socks_store_cwd());

    assert_success(chdir("/tmp"));
    getcwd(second_buffer, PATH_MAX);

    assert_success(socks_restore_cwd());
    getcwd(third_buffer, PATH_MAX);

    assert_nonzero(strcmp(first_buffer, second_buffer));
    assert_nonzero(strcmp(third_buffer, second_buffer));
    assert_zero(strcmp(first_buffer, third_buffer));

    return EXIT_SUCCESS;
}

test_t test_suite[] = {directory_test, NULL};

void nunit_config(void) {
    register_suite(test_suite, "test_suite", default_setup, default_teardown);
}
