#!/bin/csh
# Iterate through all library modules, attempting to start up,
# run for a while, then quit.

set srcdir = $1

set logname = runtest.log

/bin/rm -f $logname
echo Test started on `date` > $logname
foreach i ( $srcdir/../lib/*.g )
	echo $i
	echo "" >>$logname
	echo ">>> Running skelconq on: " $i "<<<" >>$logname
	echo "" >>$logname
	../kernel/skelconq -A -f $i -L $srcdir/../lib <<Script-EOF >>&$logname
memory
run
run
run
units
save
run
run
run
units
save
memory
quit
Script-EOF
end

echo Test finished on `date` >> $logname
