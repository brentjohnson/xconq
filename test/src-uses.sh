#!/bin/sh

# This script counts the references to GDL-related symbols in all
# Xconq sources.

# Unlike the GDL keywords, there are no C symbols/functions to confuse
# the shell's wildcard expansion process.

cd $1/..

# Count uses of keywords.

for file in keyword.def action.def history.def plan.def task.def goal.def ; do
	for sym in `grep '^DEF_' kernel/$file | sed -e 's/^[^,]*, \([a-zA-Z0-9_]*\)[,)].*$/\1/'` ; do
		rslt=`grep -c "${sym}[^A-Za-z_]" */*.[hc] | grep -v ':0$' | sed -e 's/:1$//' | tr '\012' ' '`
		# Symbol name says what kind of thing it is.
		echo ${sym} '	' ${rslt}
	done
done

# Count uses of function-based symbols.

for file in utype.def mtype.def ttype.def atype.def table.def gvar.def ; do
	for fun in `grep '^DEF_' kernel/${file} | sed -e 's/^[^,]*, \([a-zA-Z0-9_]*\),.*$/\1/'` ; do
		rslt=`grep -c "${fun}[^A-Za-z_]" */*.[hc] | grep -v ':0$' | sed -e 's/:1$//' | tr '\012' ' '`
		# Symbol name says what kind of thing it is.
		echo ${fun} '	' ${rslt}
	done
done
