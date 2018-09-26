ACLOCAL_AMFLAGS = -I m4 --install

lib_LTLIBRARIES = libsocks.la
libsocks_la_SOURCES = libsocks.c libsocks_dirs.c libsocks_debug.h eintr_wrappers.c
include_HEADERS = libsocks.h libsocks_dirs.h
libsocks_la_LDFLAGS = -release @LIB_RELEASE@

#------------------------------------------------------------------------------#

check_LTLIBRARIES = libnunit.la
libnunit_la_SOURCES = test/nunit/nunit.h test/nunit/_nunit_pvt.h
libnunit_la_SOURCES += test/nunit/nunit.c

check_PROGRAMS = test/server test/client

test_client_CFLAGS = -I@srcdir@ -I@srcdir@/test/nunit
test_client_SOURCES = test/socks_client.c
test_client_LDADD = libsocks.la libnunit.la

test_server_CFLAGS = -I@srcdir@ -I@srcdir@/test/nunit
test_server_SOURCES = test/socks_server.c
test_server_LDADD = libsocks.la libnunit.la

TEST_LOG_DRIVER = env AM_TAP_AWK='$(AWK)' $(SHELL) $(top_srcdir)/build-aux/tap-driver.sh
TEST_LOG_DRIVER_FLAGS = --comments

TESTS_ENVIRONMENT = PATH=@srcdir@/test:$(PATH)
TESTS = test/sample.test test/test-basic.sh

EXTRA_DIST = $(TESTS)
