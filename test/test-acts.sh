#!/bin/sh

srcdir=$1

if [ -r ../kernel/skelconq ] ; then
	echo "skelconq found"
else
	echo "Error: skelconq not found, exiting"
	exit 1
fi

logname=actstest.log

/bin/rm -f $logname
touch $logname
echo Test started on `date` >> $logname
for i in $srcdir/../lib/*.g ; do
	echo $i
	echo "" >> $logname
	echo ">>> Running skelconq on: " $i "<<<" >> $logname
	echo "" >> $logname
	../kernel/skelconq -f $i -L $srcdir/../lib <$srcdir/actstest.inp >> $logname
done
echo Test finished on `date` >> $logname
