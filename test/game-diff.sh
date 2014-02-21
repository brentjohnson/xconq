#!/bin/sh

'ls' $1/../lib/*.g |
  sed -e 's,^.*/lib/\([-a-z0-9]*\)\.g$,\1,' |
  sort |
  uniq >game.defs

grep -h '(base-module ' $1/../lib/*.g |
  grep -v '^;' |
  sed -e 's/^.*(base-module "\([-a-z0-9]*\)").*$/\1/'  >/tmp/rslt1

grep -h '(default-base-module ' $1/../lib/*.g |
  grep -v '^;' |
  sed -e 's/^.*(default-base-module "\([-a-z0-9]*\)").*$/\1/' >/tmp/rslt2

grep -h '(include ' $1/../lib/*.g |
  grep -v '^;' |
  sed -e 's/^.*(include "\([-a-z0-9]*\)").*$/\1/' >/tmp/rslt3

grep -h '"' $1/../lib/game.dir |
  grep -v '^;' |
  sed -e 's/^"\([-a-z0-9]*\)"/\1/' >/tmp/rslt4

cat /tmp/rslt[1-4] | sort | uniq > game.uses

echo "Differences between defined and used games"
echo "('<' - defined only,  '>' - used only)"

diff -w game.defs game.uses

exit 0
