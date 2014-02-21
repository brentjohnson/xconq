#!/bin/sh

host_type=`uname`
if { echo ${host_type} | grep -i 'mingw32' 2>&1 >/dev/null; } then
    host_type=mingw32
fi

target_path_prefix=$HOME/build/cvs/xconq
target_date=`date --utc +%Y%m%d`

target_user=`id -u`
target_group=`id -g`

scoresdir=$HOME/.xconq/scores

config_vars="CXXFLAGS=\"\""
if [ "${host_type}" = "mingw32" ]; then
    config_flags="--disable-freetypetest --disable-paraguitest"
else
    config_flags="--disable-freetypetest"
fi
while [ $# -gt 0 ]; do
    if [ "$1" = "optimize" ]; then
	config_flags="${config_flags} --enable-build-optimizations"
	config_flags="${config_flags} --disable-debugging"
    elif [ "$1" = "profile" ]; then
	config_flags="${config_flags} --enable-profiling"
    elif [ "$1" = "release" ]; then
	config_flags="${config_flags} --enable-release-build"
	config_flags="${config_flags} --disable-debugging"
    elif [ "$1" = "nodebug" ]; then
	config_flags="${config_flags} --disable-debugging"
    elif [ "$1" = "activetcl" -a "${host_type}" = "mingw32" ]; then
	config_flags="${config_flags} --with-tclconfig=/c/Tcl/lib"
	config_flags="${config_flags} --with-tkconfig=/c/Tcl/lib"
	config_flags="${config_flags} --x-includes=/c/Tcl/include/X11"
    else
	echo "Unrecognized flag: $1"
	exit 1
    fi
shift 1
done

config_flags="${config_flags} --prefix=${target_path_prefix}/${target_date}"
config_flags="${config_flags} --enable-build-warnings"
config_flags="${config_flags} --enable-alternate-gameuser=${target_user}"
config_flags="${config_flags} --enable-alternate-gamegroup=${target_group}"
config_flags="${config_flags} --enable-alternate-scoresdir=$scoresdir"

[ -f Makefile ] && make distclean >/dev/null 2>&1 && echo ">> Cleaning..."
rm -f configure && autoconf && echo ">> Rebuilding configure script..."
if [ -e configure ]; then
    echo ">> Configuring..."
    echo "${config_vars} ./configure ${config_flags}"
    eval ${config_vars} ./configure ${config_flags}
fi
[ -f Makefile ] && echo ">> Configuration completed."
