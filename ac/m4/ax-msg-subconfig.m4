dnl -------------------------------------------------------- -*- autoconf -*-
dnl ## Autoconf m4 Macro: AX_MSG_SUBCONFIG
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

## AX_MSG_SUBCONFIG(pkgname)
#
# Print a message indicating a bundled package is about to be configured. 
#   pkgname - Name of bundled package.
# ----------------------------------------------------------------------
AC_DEFUN([AX_MSG_SUBCONFIG], [

    dnl # Catch missing package name during 'autoconf' run.
    m4_if($1, , [AC_FATAL([AX_MSG_SUBCONFIG received an empty package name.])])

    # Setup message string to print.
    ax_msg_subconfig_tmp_1="Configuring bundled package: $1."
    ax_msg_subconfig_tmp_2=${#ax_msg_subconfig_tmp_1}
dnl    ax_msg_subconfig_tmp_3=""
dnl     while test ${ax_msg_subconfig_tmp_2} -gt 0; do
dnl 	ax_msg_subconfig_tmp_3="${ax_msg_subconfig_tmp_3}="
dnl 	ax_msg_subconfig_tmp_2=`expr ${ax_msg_subconfig_tmp_2} - 1`
dnl     done
    AX_PATTERN_GEN([ax_msg_subconfig_tmp_3], , [${ax_msg_subconfig_tmp_2}])

    # Print message string.
    AC_MSG_NOTICE([

${ax_msg_subconfig_tmp_3}
${ax_msg_subconfig_tmp_1}
${ax_msg_subconfig_tmp_3}
    ])
    
])

