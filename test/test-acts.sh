#!/bin/sh
# Iterate through all library modules, running the scripted action
# commands in each.  Extra arguments limit the run to the named modules
# (this is how CTest drives one check-actions-<module> test per
# lib/*.g file, in parallel).

srcdir=$1
. $srcdir/common.sh

if [ $# -gt 1 ] ; then
	shift
	list=$@
else
	list=`echo $srcdir/../lib/*.g`
fi

logname=actstest.log

rm -f $logname
touch $logname
echo Test started on `date` >> $logname
for i in $list ; do
	echo $i
	run_one $i $srcdir/actstest.inp
done
finish
