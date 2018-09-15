
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

#--------------------- Create Custom Configuration Options --------------------#

AX_CREATE_ENABLE_HELP_SECTION([Features to enable])
AX_MAKE_ENABLE_OPT([sanitizers], [no], [Build with GCC sanitizers enabled])
AX_MAKE_ENABLE_OPT([lint], [no], [Build with every warning GCC can emit])
AX_MAKE_ENABLE_OPT([warnings], [yes], [Build with -Wall -Wextra -pedantic])
AX_MAKE_ENABLE_OPT([werror], [yes], [Build with -Werror])

#---------------------- Configure For Optional Sanitizers  --------------------#

AS_IF([test "x$enable_lint" = xno], [], [
  AX_ENABLE_CFLAGS([-std=c99])
  AX_ENABLE_EVERY_WARNING([
    -Wtraditional -Wformat-nonliteral
    -Wtraditional-conversion -Wpadded -Wunused-macros -Wabi
  ])
])

AS_IF([test "x$enable_warnings" = xno], [], [
  AX_ENABLE_CFLAGS([
    -Wall
    -Wextra
    -pedantic
  ])
])

AS_IF([test "x$enable_sanitizers" = xno], [], [
  AX_ENABLE_CFLAGS([
    -O0
    -g
    -fstack-protector-strong
    -fstack-protector-all
    -fsanitize=shift
    -fsanitize=undefined
    -fsanitize=address
    -fsanitize=alignment
    -fsanitize=bool
    -fsanitize=bounds
    -fsanitize=bounds-strict
    -fsanitize=enum
    -fsanitize=float-cast-overflow
    -fsanitize=float-divide-by-zero
    -fsanitize=integer-divide-by-zero
    -fsanitize=null
    -fsanitize=object-size
    -fno-sanitize-recover=all
    -fsanitize=return
    -fsanitize=returns-nonnull-attribute
    -fsanitize=signed-integer-overflow
  ])
])

AS_IF([test "x$enable_werror" = xno], [], [
  AS_VAR_APPEND([CFLAGS],[" -Werror"])
])

#------------------------------ Generate Outputs ------------------------------#

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
