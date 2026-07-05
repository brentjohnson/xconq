#!/bin/sh
# Iterate through all library modules, attempting to start up,
# do some simple commands, and quit.  Extra arguments limit the run to
# the named modules (this is how CTest drives one check-lib-<module>
# test per lib/*.g file, in parallel).

srcdir=$1
. $srcdir/common.sh

if [ $# -gt 1 ] ; then
	shift
	list=$@
else
	list=`echo $srcdir/../lib/*.g`
fi

logname=libtest.log

rm -f $logname
touch $logname
echo Test started on `date` >> $logname
for i in $list ; do
	echo $i
	run_one $i $srcdir/libtest.inp
done
finish
