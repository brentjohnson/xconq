#!/bin/sh

# NOTE (2026-07-05): this script is intentionally NOT wired into CTest.
#
# It cross-references image family (imf) definitions against
# image-name/emblem-name/unit-type/terrain-type uses in the library.
# Run against the current lib/, it reports ~1750 mismatches out of
# ~4500 total entries (39%) -- overwhelmingly imf.defs entries with no
# matching use. That's not drift from a once-in-sync baseline: large
# fractions of the image library are bulk-imported sprite packs (the
# "ang-*" Angband tileset, "kb-*" sci-fi ship sets) and a generic X11
# color-name palette, provided as optional/decorative inventory that
# most games never reference directly. A "some large fraction of imf
# defs go unused" state looks to be the norm here, not a bug, so a
# waiver list at this scale would just be a blanket ignore wearing a
# long list as a disguise. Left as a manual/informational script.
#
# grep -a below (added 2026-07-05) avoids "binary file matches" noise:
# a couple of lib/*.g files (e.g. spec-rules.g) aren't valid UTF-8.

grep -ah '(imf ' $1/../lib/*.imf $1/../lib/*.g |
  sed -e 's/^(imf "\([-a-z0-9]*\)".*$/\1/' |
  sort |
  uniq >imf.defs

grep -ah '(emblem-name ' $1/../lib/*.g |
  grep -av '^;' |
  sed -e 's/^.*(emblem-name "\([-a-z0-9]*\)").*$/\1/'  > /tmp/rslt1

grep -ah '(image-name ' $1/../lib/*.g |
  grep -av '^;' |
  sed -e 's/^.*(image-name "\([-a-z0-9]*\)").*$/\1/' > /tmp/rslt2

grep -ah ' image-name ' $1/../lib/*.g |
  grep -av '^;' |
  sed -e 's/^.* image-name "\([-a-z0-9]*\)").*$/\1/' > /tmp/rslt3

grep -ah '(unit-type ' $1/../lib/*.g |
  grep -av '^;' |
  grep -av image-name |
  sed -e 's/^(unit-type \([-.&*/@a-zA-Z0-9]*\)[ )].*$/\1/' \
      -e 's/^(unit-type \([-.&*/@a-zA-Z0-9]*\)$/\1/' > /tmp/rslt4

grep -ah '(terrain-type ' $1/../lib/*.g |
  grep -av '^;' |
  grep -av image-name |
  sed -e 's/^(terrain-type \([-a-zA-Z0-9]*\)[ )].*$/\1/' \
      -e 's/^(terrain-type \([-a-zA-Z0-9]*\)$/\1/' > /tmp/rslt5

cat /tmp/rslt[1-5] | sort |uniq > imf.uses

echo "Differences between defined and used images"
echo "('<' - definitions only,  '>' - uses only)"

diff -w imf.defs imf.uses

exit 0
