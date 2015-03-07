#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT(libimgutil, 0.1, chris.stones@gmail.com)
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_SRCDIR(src/libimgutil.c)
AC_CONFIG_HEADERS(src/config.h)

AC_CONFIG_MACRO_DIR([m4])

LT_INIT
AM_INIT_AUTOMAKE([subdir-objects])

#AX_PTHREAD(
#  [
#    LIBS="$PTHREAD_LIBS $LIBS"
#    CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
#    CXXFLAGS="$CXXFLAGS $PTHREAD_CFLAGS"
#    CC="$PTHREAD_CC"
#  ],
#  []
#)

AC_SEARCH_LIBS(
  [squish_Compress],[csquish],[],
  AC_MSG_ERROR([Unable to find csquish library]))

AC_CHECK_HEADERS([GL/gl.h],[],[AC_MSG_ERROR([Missing GL/gl.h])])

AC_SEARCH_LIBS(
  [imgAllocAndRead],[img],[],
  AC_MSG_ERROR([Unable to find libimg library]))
  
AC_CHECK_HEADERS([libimg.h],[],[AC_MSG_ERROR([Missing libimg.h])])

CXXFLAGS="$CXXFLAGS -std=c++11"

AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL

AC_CONFIG_FILES([
	Makefile
	src/Makefile
])
AC_OUTPUT


