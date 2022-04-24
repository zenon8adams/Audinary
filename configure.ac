dnl Process this file with autoreconf to produce a configure script
dnl ------------------------------------------------------------------------
dnl configure.ac
dnl ------------------------------------------------------------------------
dnl Copyright (C) 2022 by Adesina Meekness. All Rights Reserved.
dnl ------------------------------------------------------------------------
dnl Author: Adesina Meekness
dnl ------------------------------------------------------------------------
dnl GNU General Public License (GPL) Rights pursuant to Version 3, June 2007
dnl ------------------------------------------------------------------------

AC_PREREQ([2.69])
AC_INIT([audinary], [1.0], [zenon8adams@gmail.com])

# Checks for programs.
AC_PROG_CXX
AC_PROG_INSTALL

AC_LANG([C++])

AC_CONFIG_SRCDIR([driver.cpp])
AC_CONFIG_HEADERS([config.h])
AC_COPYRIGHT([Copyright (C) 2022 by Adesina Meekness. All Rights Reserved.])

# Checks for custom headers
AC_CHECK_HEADER([cAudio/cAudio.h], [AC_DEFINE([HAVE_CAUDIO_H], [1], [found cAudio.h])])
AC_CHECK_HEADER([httpserver.hpp], [AC_DEFINE([HAVE_HTTPSERVER_H], [1], [found httpserver.h])])

# Checks for libraries.
AC_CHECK_LIB([cAudio], [ov_open], [LIBCAUDIO=-lcAudio])
AC_SUBST(LIBCAUDIO)

AC_CHECK_LIB([fontconfig], [FcConfigGetCurrent])

AC_CHECK_LIB([httpserver], [_ZN10httpserver15string_response16get_raw_responseEv], [LIBHTTPSERVER=-lhttpserver], [
    AC_MSG_ERROR([libhttpserver is required!])
])
AC_SUBST(LIBHTTPSERVER)

AC_CHECK_LIB([pthread], [pthread_create])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h syslog.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT8_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_CHECK_FUNCS([munmap setlocale strchr])

AC_CONFIG_FILES([Makefile
                 source/Makefile])
AC_OUTPUT
