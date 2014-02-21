#!/bin/sh
# Test the AI playing itself.

srcdir=$1

logname=aitest.log

rm -f $logname
echo Test started on `date` > $logname
for gd in $srcdir/../lib/*.g ; do
	echo $gd
	echo "" >>$logname
	echo ">>> Running skelconq on: " $gd "<<<" >>$logname
	echo "" >>$logname
	../kernel/skelconq -r ,ai@stdio -f $gd -L $srcdir/../lib <$srcdir/aitest.inp >>$logname
done
