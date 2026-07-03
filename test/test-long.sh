#!/bin/sh
# Iterate through all library modules, attempting to start up,
# do lots and lots of commands, and quit.

srcdir=$1
. $srcdir/common.sh

logname=longtest.log

rm -f $logname
touch $logname
echo Test started on `date` >> $logname
for i in $srcdir/../lib/*.g ; do
	echo $i
	run_one $i $srcdir/longtest.inp
done
finish
