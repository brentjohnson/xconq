dnl -------------------------------------------------------- -*- autoconf -*-
dnl ## Autoconf m4 Macro: AX_PATTERN_GEN
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

## AX_PATTERN_GEN(pattern-variable, [pattern], [multiplier])
#
# Generate a repeating pattern and save it in a shell variable.
#   pattern-variable - Shell variable to store repeating pattern in.
#   pattern - Pattern to be repeated. 
#   multiplier - How many times to repeat pattern.
# By default, if 'pattern' is missing, then '=' is used as the pattern.
# By default, if 'multiplier' is missing, then the 'COLUMNS' environment 
# variable is used.  If that variable is undefined, then '80' is used.
# ----------------------------------------------------------------------
AC_DEFUN([AX_PATTERN_GEN], [

    dnl # Catch missing 'pattern-variable' during 'autoconf' run.
    m4_if($1, , [AC_FATAL([AX_PATTERN_GEN received an empty variable name.])])

    # If 'pattern' is missing, then choose default pattern.
    ax_pattern_gen_pat="$2"
    if test "$2" = ""
    then
	ax_pattern_gen_pat="="
    fi

    # If 'multiplier' is missing, then choose default multipler.
    ax_pattern_gen_mult=$3
    if test "$3" = ""
    then
	if test "$COLUMNS" != "" && test $COLUMNS -ne 0; then
	    ax_pattern_gen_mult=$COLUMNS
	else
	    ax_pattern_gen_mult=80
	fi
    fi

    # Generate the pattern.
    $1=""
    while test ${ax_pattern_gen_mult} -gt 0
    do
        $1="${$1}${ax_pattern_gen_pat}"
        ax_pattern_gen_mult=`expr ${ax_pattern_gen_mult} - 1`
    done

])

