#!/bin/csh
# Iterate through all library modules, attempting to start up,
# do lots and lots of commands, and quit.

set srcdir = $1

set logname = longtest.log

/bin/rm -f $logname
echo Test started on `date` > $logname
foreach i ( $srcdir/../lib/*.g )
	echo $i
	echo "" >>$logname
	echo ">>> Running skelconq on: " $i "<<<" >>$logname
	echo "" >>$logname
	../kernel/skelconq -f $i -L $srcdir/../lib <$srcdir/longtest.inp >>&$logname
end
echo Test finished on `date` >> $logname
