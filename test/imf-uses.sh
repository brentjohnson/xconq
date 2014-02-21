#! /bin/bash

cd $1/../lib

for sym in `grep -h '^(imf ' *.imf | sed -e 's/^(imf "\([^"]*\)".*$/\1/' | sort |uniq` ; do
	grep -c "image-name \"${sym}\"" *.g | grep -v ':0$' >/tmp/rslt1
	grep -c "emblem-name \"${sym}\"" *.g | grep -v ':0$' >/tmp/rslt2
	grep -c "unit-type ${sym}[^A-Za-z-]" *.g | grep -v ':0$' >/tmp/rslt3
	grep -c "unit-type ${sym}$" *.g | grep -v ':0$' >/tmp/rslt4
	grep -c "terrain-type ${sym}[^A-Za-z-]" *.g | grep -v ':0$' >/tmp/rslt5
	grep -c "terrain-type ${sym}$" *.g | grep -v ':0$' >/tmp/rslt6
	rslt=`cat /tmp/rslt[1-6] | sed -e 's/:1$//' | sort | uniq | tr '\012' ' '`
	echo ${sym} '	' ${rslt}
done

rm /tmp/rslt[1-6]

