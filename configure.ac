
#------------------------------------------------------------------------------#

m4_include([m4/semantic_version.m4])
SEMANTIC_VERSION_SET_FILE([VERSION])
AC_INIT([libsocks], SEMANTIC_VERSION, [nicholas.clark@gmail.com])
AC_SUBST([LIB_RELEASE],[SEMANTIC_VERSION])

#------------------------ Configure Source Locations  -------------------------#

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])
AC_REQUIRE_AUX_FILE([tap-driver.sh])
AC_CONFIG_SRCDIR([libsocks.h])
AC_CONFIG_HEADERS([config.h])

#------------------------- Init Automake / Autoconf  --------------------------#

AC_PREREQ([2.69])
AM_INIT_AUTOMAKE([foreign subdir-objects])
AC_USE_SYSTEM_EXTENSIONS

#------------------------ Confirm Rand() Requirements  ------------------------#

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL

AM_PROG_AR
AM_PROG_LIBTOOL
LT_INIT

# Checks for header files
AC_CHECK_HEADERS([fcntl.h limits.h stdint.h string.h sys/socket.h \
                  sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_PID_T
AC_C_RESTRICT
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT32_T

# Checks for library functions.
AC_FUNC_STRNLEN
AC_CHECK_FUNCS([select socket])

#------------------------------ Generate Outputs ------------------------------#

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
