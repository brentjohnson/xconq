#!/bin/sh

grep -h '(imf ' $1/../lib/*.imf $1/../lib/*.g |
  sed -e 's/^(imf "\([-a-z0-9]*\)".*$/\1/' |
  sort |
  uniq >imf.defs

grep -h '(emblem-name ' $1/../lib/*.g |
  grep -v '^;' |
  sed -e 's/^.*(emblem-name "\([-a-z0-9]*\)").*$/\1/'  > /tmp/rslt1

grep -h '(image-name ' $1/../lib/*.g |
  grep -v '^;' |
  sed -e 's/^.*(image-name "\([-a-z0-9]*\)").*$/\1/' > /tmp/rslt2

grep -h ' image-name ' $1/../lib/*.g |
  grep -v '^;' |
  sed -e 's/^.* image-name "\([-a-z0-9]*\)").*$/\1/' > /tmp/rslt3

grep -h '(unit-type ' $1/../lib/*.g |
  grep -v '^;' |
  grep -v image-name |
  sed -e 's/^(unit-type \([-.&*/@a-zA-Z0-9]*\)[ )].*$/\1/' \
      -e 's/^(unit-type \([-.&*/@a-zA-Z0-9]*\)$/\1/' > /tmp/rslt4

grep -h '(terrain-type ' $1/../lib/*.g |
  grep -v '^;' |
  grep -v image-name |
  sed -e 's/^(terrain-type \([-a-zA-Z0-9]*\)[ )].*$/\1/' \
      -e 's/^(terrain-type \([-a-zA-Z0-9]*\)$/\1/' > /tmp/rslt5

cat /tmp/rslt[1-5] | sort |uniq > imf.uses

echo "Differences between defined and used images"
echo "('<' - definitions only,  '>' - uses only)"

diff -w imf.defs imf.uses

exit 0
