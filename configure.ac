AC_PREREQ([2.69])
AC_INIT([feednix], [0.6.4.2], [jorgemartinezhernandez@gmail.com])
AC_CONFIG_SRCDIR([src/main.cpp])
AM_INIT_AUTOMAKE([no-define foreign])
AC_CONFIG_HEADERS([config.h])
AC_PREFIX_DEFAULT([/usr])

AC_PROG_CXX
AC_PROG_CC

AC_CHECK_LIB([curl], [curl_easy_init])
AC_CHECK_LIB([jsoncpp], [_ZNK4Json5Value4sizeEv])
AC_CHECK_LIB([menu], [free_item])
AC_CHECK_LIB([ncurses], [initscr])
AC_CHECK_LIB([panel], [new_panel])

AC_CHECK_HEADERS([stdlib.h string.h termios.h unistd.h])

AC_CHECK_HEADER_STDBOOL
AC_TYPE_SIZE_T

AC_CHECK_FUNCS([atexit strdup])

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
