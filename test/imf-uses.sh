#!/bin/sh

# Manual/informational only -- not wired into CTest (2026-07-05): this
# is a per-symbol usage report, not a pass/fail check, and its data
# overlaps imf-diff.sh (also left manual-only; see the dated note
# there about the scale of legitimately-unused bulk image inventory).

cd $1/../lib

for sym in `grep -ah '^(imf ' *.imf | sed -e 's/^(imf "\([^"]*\)".*$/\1/' | sort |uniq` ; do
	grep -ac "image-name \"${sym}\"" *.g | grep -v ':0$' >/tmp/rslt1
	grep -ac "emblem-name \"${sym}\"" *.g | grep -v ':0$' >/tmp/rslt2
	grep -ac "unit-type ${sym}[^A-Za-z-]" *.g | grep -v ':0$' >/tmp/rslt3
	grep -ac "unit-type ${sym}$" *.g | grep -v ':0$' >/tmp/rslt4
	grep -ac "terrain-type ${sym}[^A-Za-z-]" *.g | grep -v ':0$' >/tmp/rslt5
	grep -ac "terrain-type ${sym}$" *.g | grep -v ':0$' >/tmp/rslt6
	rslt=`cat /tmp/rslt[1-6] | sed -e 's/:1$//' | sort | uniq | tr '\012' ' '`
	echo ${sym} '	' ${rslt}
done

rm /tmp/rslt[1-6]
