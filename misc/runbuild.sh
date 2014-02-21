#!/bin/sh

host_type=`uname`
if { echo ${host_type} | grep -i 'mingw32' 2>&1 >/dev/null; } then
    host_type=mingw32
fi

special_target=""
if [ "$1" != "" ]; then
    special_target="-$1"
fi

if [ ! -f Makefile ]; then
    ./runconfig.sh
    exit_status=$?
    [ ${exit_status} -ne 0 ] && exit ${exit_status}
fi

echo ">> Building..."
make all${special_target} 2>&1 | tee Build.log
grep 'error:' Build.log 2>&1 >/dev/null
exit_status=$?
if [ ${exit_status} -eq 0 ]; then
    echo ">>>> Build failed! <<<<"
    exit 1
fi

if [ "${host_type}" != "mingw32" ]; then
echo ">> Installing..."
make install${special_target} 2>&1 | tee Install.log
grep 'error:' Install.log 2>&1 >/dev/null
exit_status=$?
if [ ${exit_status} -eq 0 ]; then
    echo ">>>> Install failed! <<<<"
    exit 1
fi
fi

echo ">> Building and installation completed."
