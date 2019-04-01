AC_PREREQ([2.69])
AC_INIT([mdesigner], [1.0], [akatz712@gmail.com])
AM_INIT_AUTOMAKE([subdir-objects])
AC_PROG_CC
AC_PROG_CXX
AC_CONFIG_FILES([Makefile])
CC=g++
CXXFLAGS="$CXXFLAGS `fltk-config --cxxflags` -Wno-narrowing"
LIBS="`fltk-config --use-images --ldflags` -lhpdf"
AC_OUTPUT
