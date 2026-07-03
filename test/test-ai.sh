#!/bin/sh
# Test the AI playing itself.

srcdir=$1
. $srcdir/common.sh

logname=aitest.log

rm -f $logname
touch $logname
echo Test started on `date` >> $logname
for gd in $srcdir/../lib/*.g ; do
	echo $gd
	run_one $gd $srcdir/aitest.inp -r ,ai@stdio
done
finish
