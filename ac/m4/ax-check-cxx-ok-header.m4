dnl -------------------------------------------------------- -*- autoconf -*-
dnl ## Autoconf m4 Macro: AX_CHECK_CXX_OK_HEADER
dnl ## Copyright (C) 2005	Eric A. McDonald

dnl #This program is free software; you can redistribute it and/or modify
dnl #it under the terms of the GNU General Public License as published by
dnl #the Free Software Foundation; either version 2 of the License, or
dnl #(at your option) any later version.
dnl #
dnl #This program is distributed in the hope that it will be useful,
dnl #but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl #MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl #GNU General Public License for more details.
dnl #
dnl #You should have received a copy of the GNU General Public License
dnl #along with this program; if not, write to the Free Software
dnl #Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

dnl # Dependencies: M4 sug4r with 'AS_SET' and friends.

## AX_CHECK_CXX_OK_HEADER(C-HEADER)
#
# Checks whether a C++ compiler can extern "C" the given C header.
# C-HEADER: The name of a C header file.
# ----------------------------------------------------------------------
AC_DEFUN([AX_CHECK_CXX_OK_HEADER], [
    AC_LANG_PUSH([C++])
    AS_VAR_PUSHDEF([ac_Header], [ac_cv_header_cxx_okay_$1])dnl
    AC_CACHE_CHECK([if C++ compiler can use $1 with extern "C" wrapper], ac_Header,
		   [AC_COMPILE_IFELSE([
[${ac_includes_default}]
extern "C" {
@%:@include <$1>
}],
				      [AS_VAR_SET(ac_Header, yes)],
				      [AS_VAR_SET(ac_Header, no)])])
    AS_IF([test AS_VAR_GET(ac_Header) = yes], [
AC_DEFINE_UNQUOTED(AS_TR_CPP(HAVE_CXX_OK_$1))])[]dnl
    AS_VAR_POPDEF([ac_Header])dnl
    AC_LANG_POP([C++])
])

