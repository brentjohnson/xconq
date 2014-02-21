#!/bin/sh
# Iterate through library modules, attempting to start up,
# do some simple commands, save, then restore, then save again,
# and report on the differences.

srcdir=$1

if [ $# -gt 1 ] ; then
	shift
	list=$@
else
	list=`echo $srcdir/../lib/*.g`
fi

if [ -r ../kernel/skelconq ] ; then
	echo "skelconq found"
else
	echo "Error: skelconq not found, exiting"
	exit 1
fi

logname=savetest.log
slogname=svtest.log

/bin/rm -f $logname
/bin/rm -f $slogname
touch $logname
touch $slogname
echo Test started on `date` >> $logname
for i in $list ; do
	echo $i
	echo "" >> $logname
	echo ">>> Running skelconq on: " $i "<<<" >> $logname
	echo "" >> $logname
	echo ">>> Running skelconq on: " $i "<<<" >> $slogname
	../kernel/skelconq -f $i -L $srcdir/../lib <$srcdir/savetst1.inp >> $logname
	if [ -f ~/.xconq/save.xcq ] ; then
		mv ~/.xconq/save.xcq save1.xcq
		echo ">>> Restoring skelconq; <<<" >> $logname
		../kernel/skelconq -f save1.xcq -noai <$srcdir/savetst2.inp >> $logname
		if [ -f ~/.xconq/save.xcq ] ; then
			sed -e '/run-serial-number/d' \
			    -e '/elapsed-real-time/d' \
			    -e '/ai-type-name/d' \
			    -e '/game-saved/d' \
			    -e '/log-started/d' \
			    -e '/game-restarted/d' \
			    save1.xcq >save1f.xcq
			mv ~/.xconq/save.xcq save2.xcq
			sed -e '/run-serial-number/d' \
			    -e '/elapsed-real-time/d' \
			    -e '/ai-type-name/d' \
			    -e '/game-saved/d' \
			    -e '/log-started/d' \
			    -e '/game-restarted/d' \
			    save2.xcq >save2f.xcq
			if cmp save1f.xcq save2f.xcq >/dev/null ; then
				echo ">>> Savefiles match <<<" >> $logname
			else
				gamename=`echo $i | sed -e 's,.*/lib/,,'`
				mv save1.xcq save1.$gamename.xcq
				mv save1f.xcq save1f.$gamename.xcq
				mv save2f.xcq save2f.$gamename.xcq
				echo ">>> diff -u save1f.$gamename.xcq save2f.$gamename.xcq <<<" >> $logname
				diff -u save1f.$gamename.xcq save2f.$gamename.xcq >> $logname
				echo ">>> diff -u0 save1f.$gamename.xcq save2f.$gamename.xcq <<<" >> $slogname
				diff -u0 save1f.$gamename.xcq save2f.$gamename.xcq | grep -v '^@' >> $slogname
				echo `diff -u0 save1f.$gamename.xcq save2f.$gamename.xcq | grep -v '^[0-9]' | wc -l` lines of difference
			fi
		else
			echo Second savefile missing!
		fi
	fi
done
echo Test finished on `date` >> $logname
