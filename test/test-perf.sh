#!/bin/sh
# Calculate how long it takes to do a standard set of games.

srcdir=$1

if [ -r ../kernel/skelconq ] ; then
	echo "skelconq found"
else
	echo "Error: skelconq not found, exiting"
	exit 1
fi

logname=perftest.log

tottime=0

/bin/rm -f $logname
touch $logname
echo Test started on `date` >> $logname

for options in \
    "-g standard -M 20 -r ,ai@stdio" \
    "-g classic -M 20 -r ,ai@stdio" \
    "-g gettysburg -r ,ai@stdio" \
    "-g magnusvew -r ,ai@stdio" \
    "-g ww2-eur-42 -r ,ai@stdio" \
    ; do
    starttime=`date +%s`
    echo Running with $options
    echo "" >> $logname
    echo ">>> Running skelconq with: " $options "<<<" >> $logname
    echo "" >> $logname
    ../kernel/skelconq $options -R 1 -L $srcdir/../lib < $srcdir/perftest.inp >> $logname
    endtime=`date +%s`
    runtime=`expr $endtime - $starttime`
    echo "    " $runtime secs
    echo Execution time with $options is $runtime secs >> $logname
    tottime=`expr $tottime + $runtime`
done

echo Execution time total is $tottime secs
echo Execution time total is $tottime secs >> $logname

echo Test finished on `date` >> $logname


