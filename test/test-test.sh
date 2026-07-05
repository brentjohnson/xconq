#!/bin/sh
# Iterate through all test modules, attempting to start up,
# do some simple commands, and quit.  Extra arguments limit the run to
# the named modules (this is how CTest drives one check-test-<module>
# test per test/*.g file, in parallel).
#
# The test modules are the suite's own inputs, so they are all held to
# the strict no-diagnostics policy, except that the error*.g and warn.g
# modules exist to exercise error handling and must produce diagnostics,
# and extreme2.g (the terrain-type limit test) legitimately warns while
# synthesizing terrain for its 130 percentile-less terrain types.

srcdir=$1
. $srcdir/common.sh

ALL_STRICT=yes

if [ $# -gt 1 ] ; then
	shift
	list=$@
else
	list=`echo $srcdir/../test/*.g`
fi

logname=test.log

rm -f $logname
touch $logname
echo Test started on `date` >> $logname
for i in $list ; do
	echo $i
	case `basename $i` in
	error*.g|warn.g|extreme2.g)
		run_expect_diag $i $srcdir/libtest.inp ;;
	*)
		run_one $i $srcdir/libtest.inp ;;
	esac
done
finish
