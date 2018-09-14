ACLOCAL_AMFLAGS = -I m4 --install

lib_LTLIBRARIES = libsocks.la
libsocks_la_SOURCES = libsocks.c
include_HEADERS = libsocks.h
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

TESTS = test/test-basic.sh
TESTS_ENVIRONMENT = PATH=@srcdir@/test:$(PATH)

#EXTRA_DIST = test/chpst.check test/chpst.dist test/runit.check test/runit.dist
#CLEANFILES = $(TESTS) runit.stdout runit-init.stdout runsv.stdout
