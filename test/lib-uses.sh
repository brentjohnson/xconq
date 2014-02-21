#!/bin/sh

# This script counts the references to GDL symbols in all Xconq library games.

savedir=`pwd`

/bin/rm -rf /tmp/xc7

mkdir -p /tmp/xc7/lib
mkdir -p /tmp/xc7/test

# Filter out non-symbol stuff in library and test files.

cd $1/..

for libfile in lib/*.g lib/*.imf test/*.g test/*.imf ; do
	sed -e 's/"[^"]*"/ /g' $libfile \
	| sed -e 's/"[^"][^"]*$//' \
	| sed -e 's/^[^"][^"]*"//' \
	| sed -e 's/;.*$//g' \
	| tr '() ' '\012' >/tmp/xc7/$libfile
done

# Count uses of keywords.

cd $savedir

# We need to be in an empty directory so "m*" doesn't get expanded into
# a list of file names.

if [ ! -d ../empty ] ; then
	mkdir ../empty
fi

cd ../empty

for file in keyword.def action.def history.def plan.def task.def goal.def ; do
	for sym in `grep DEF_ $1/../kernel/${file} | sed -e 's/^[^"]*"\([^"]*\)".*$/\1/' | grep -v '^zz-'` ; do
		sym=$sym
		rslt=`fgrep -c -x ${sym} /tmp/xc7/*/*.g /tmp/xc7/*/*.imf | grep -v ':0$' | sed -e 's/:1$//' | sed -e 's,/tmp/xc7/,,' | tr '\012' ' '`
		echo ${sym} '	' ${file} '	' ${rslt}
	done
done

for file in utype.def mtype.def ttype.def atype.def table.def gvar.def ; do
	for sym in `grep DEF_ $1/../kernel/${file} | sed -e 's/^[^"]*"\([^"]*\)".*$/\1/' | grep -v '^zz-'` ; do
		rslt=`fgrep -c -x ${sym} /tmp/xc7/*/*.g /tmp/xc7/*/*.imf | grep -v ':0$' | sed -e 's/:1$//' | sed -e 's,/tmp/xc7/,,' | tr '\012' ' '`
		echo ${sym} '	' ${file} '	' ${rslt}
	done
done



