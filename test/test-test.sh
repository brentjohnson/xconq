#!/bin/sh
# Iterate through all test modules, attempting to start up,
# do some simple commands, and quit.
#
# The test modules are the suite's own inputs, so they are all held to
# the strict no-diagnostics policy, except that the error*.g and warn.g
# modules exist to exercise error handling and must produce diagnostics,
# and extreme2.g (the terrain-type limit test) legitimately warns while
# synthesizing terrain for its 130 percentile-less terrain types.

srcdir=$1
. $srcdir/common.sh

ALL_STRICT=yes

logname=test.log

rm -f $logname
touch $logname
echo Test started on `date` >> $logname
for i in $srcdir/../test/*.g ; do
	echo $i
	case `basename $i` in
	error*.g|warn.g|extreme2.g)
		run_expect_diag $i $srcdir/libtest.inp ;;
	*)
		run_one $i $srcdir/libtest.inp ;;
	esac
done
finish
