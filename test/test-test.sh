#!/bin/csh
# Iterate through all test modules, attempting to start up,
# do some simple commands, and quit.

set srcdir = $1

set logname = test.log

/bin/rm -f $logname
echo Test started on `date` > $logname
foreach i ( $srcdir/../test/*.g )
	echo $i
	echo "" >>$logname
	echo ">>> Running skelconq on: " $i "<<<" >>$logname
	echo "" >>$logname
	../kernel/skelconq -f $i -L $srcdir/../lib <$srcdir/libtest.inp >>&$logname
end
echo Test finished on `date` >> $logname
