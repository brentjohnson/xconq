#!/bin/sh
# Iterate through all library modules, attempting to start up,
# do some simple commands, and quit.

srcdir=$1
. $srcdir/common.sh

logname=libtest.log

rm -f $logname
touch $logname
echo Test started on `date` >> $logname
for i in $srcdir/../lib/*.g ; do
	echo $i
	run_one $i $srcdir/libtest.inp
done
finish
