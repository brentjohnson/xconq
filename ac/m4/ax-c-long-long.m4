dnl -------------------------------------------------------- -*- autoconf -*-
dnl ## Autoconf m4 Macro: AX_C_LONG_LONG_INT
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

## AX_C_LONG_LONG_INT
#
# Checks whether platform and C or C++ compiler support 'long long int'.
# This macro is heavily inspired by 'AC_LONG_DOUBLE' in 
# Autoconf's 'c.m4'.
# ----------------------------------------------------------------------
AC_DEFUN([AX_C_LONG_LONG_INT], [
    AC_CACHE_CHECK(
	[for long long int that is twice the width of long int],
	[ac_cv_c_long_long_int], [
	AC_COMPILE_IFELSE([
	    AC_LANG_BOOL_COMPILE_TRY(
		[long long int i;],
		[(sizeof(long long int) / sizeof(long int)) == 2]
	    )],
	    ac_cv_c_long_long_int=yes,
	    ac_cv_c_long_long_int=no)
    ])
    if test "${ac_cv_c_long_long_int}" = "yes"
    then
	AC_DEFINE(HAVE_LONG_LONG_INT, 1, 
		[Define to 1, if 'long long int' is available and is 
		 twice as wide as 'long int'.])
    fi
])

