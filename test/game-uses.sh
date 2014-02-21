#! /bin/bash

cd $1/../lib

grep '^"' game.dir |
  sed -e 's/^"\([^"]*\)"$/G game.dir \1.g/' > /tmp/rslt1

grep '(base-module ' *.g |
  sed -e 's/^\([^:]*\):[^"]*"\([^"]*\)".*$/B \1 \2.g/' >> /tmp/rslt1

grep '(default-base-module ' *.g |
  sed -e 's/^\([^:]*\):[^"]*"\([^"]*\)".*$/D \1 \2.g/' >> /tmp/rslt1

grep '(include ' *.g |
  sed -e 's/^\([^:]*\):[^"]*"\([^"]*\)".*$/I \1 \2.g/' >> /tmp/rslt1

for libfile in *.g ; do
	rslt=`grep "^[^ ]* [^ ]* ${libfile}" /tmp/rslt1 | sed -e 's/^\([^ ]*\) \([^ ]*\) .*$/\1:\2/' | tr '\012' ' '`
	echo ${libfile} '	' ${rslt}
done

