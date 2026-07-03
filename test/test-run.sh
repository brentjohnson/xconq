#!/bin/sh
# Iterate through all library modules, attempting to start up,
# run for a while, then quit.

srcdir=$1
. $srcdir/common.sh

logname=runtest.log

rm -f $logname
touch $logname
echo Test started on `date` >> $logname
for i in $srcdir/../lib/*.g ; do
	echo $i
	run_one $i $srcdir/runtest.inp
done
finish
