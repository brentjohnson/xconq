#!/bin/sh

# Manual/informational only -- not wired into CTest (2026-07-05): this
# is a per-file usage report (which of game.dir/base-module/
# default-base-module/include references each lib/*.g file), not a
# pass/fail check. Its "unreferenced file" signal is the same data
# game-diff.sh's defined-only column already reports and gates on;
# this script just breaks it down per-file for human triage.

cd $1/../lib

grep -a '^"' game.dir |
  sed -e 's/^"\([^"]*\)"$/G game.dir \1.g/' > /tmp/rslt1

grep -ah '(base-module ' *.g |
  sed -e 's/^\([^:]*\):[^"]*"\([^"]*\)".*$/B \1 \2.g/' >> /tmp/rslt1

grep -ah '(default-base-module ' *.g |
  sed -e 's/^\([^:]*\):[^"]*"\([^"]*\)".*$/D \1 \2.g/' >> /tmp/rslt1

grep -ah '(include ' *.g |
  sed -e 's/^\([^:]*\):[^"]*"\([^"]*\)".*$/I \1 \2.g/' >> /tmp/rslt1

for libfile in *.g ; do
	rslt=`grep "^[^ ]* [^ ]* ${libfile}" /tmp/rslt1 | sed -e 's/^\([^ ]*\) \([^ ]*\) .*$/\1:\2/' | tr '\012' ' '`
	echo ${libfile} '	' ${rslt}
done
