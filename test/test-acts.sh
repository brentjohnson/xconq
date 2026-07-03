#!/bin/sh
# Iterate through all library modules, running the scripted action
# commands in each.

srcdir=$1
. $srcdir/common.sh

logname=actstest.log

rm -f $logname
touch $logname
echo Test started on `date` >> $logname
for i in $srcdir/../lib/*.g ; do
	echo $i
	run_one $i $srcdir/actstest.inp
done
finish
