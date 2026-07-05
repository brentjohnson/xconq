#!/bin/sh

# Scratch files go in a private per-run directory (not a shared /tmp
# name) since check-consistency-* tests can run concurrently under
# `ctest -j`.
tmpdir=`mktemp -d`
trap 'rm -rf "$tmpdir"' EXIT

'ls' $1/../lib/*.g |
  sed -e 's,^.*/lib/\([-a-z0-9]*\)\.g$,\1,' |
  sort |
  uniq >game.defs

grep -h '(base-module ' $1/../lib/*.g |
  grep -v '^;' |
  sed -e 's/^.*(base-module "\([-a-z0-9]*\)").*$/\1/'  >$tmpdir/rslt1

grep -h '(default-base-module ' $1/../lib/*.g |
  grep -v '^;' |
  sed -e 's/^.*(default-base-module "\([-a-z0-9]*\)").*$/\1/' >$tmpdir/rslt2

grep -h '(include ' $1/../lib/*.g |
  grep -v '^;' |
  sed -e 's/^.*(include "\([-a-z0-9]*\)").*$/\1/' >$tmpdir/rslt3

grep -h '"' $1/../lib/game.dir |
  grep -v '^;' |
  sed -e 's/^"\([-a-z0-9]*\)"/\1/' >$tmpdir/rslt4

cat $tmpdir/rslt[1-4] | sort | uniq > game.uses

echo "Differences between defined and used games"
echo "('<' - defined only,  '>' - used only)"

diffs=`diff -w game.defs game.uses`
echo "$diffs"

# Waiver list of known-legitimate mismatches (2026-07-05). Two flavors:
#
#  - "used only" (base-module/default-base-module names with no
#    matching lib/<name>.g): t-eastmed.g and t-africa.g each prepared a
#    terrain map expecting a ruleset module ("greek", "africa") that
#    was apparently never written -- every *other* t-*.g base-module
#    reference in the library points at a real file, so these two look
#    like abandoned/unfinished content rather than typos with an
#    obvious fix.
#
#  - "defined only" (lib/*.g files nothing references): the library's
#    "u-*"/"t-*"/"ng-*" modules are meant to be combined manually at
#    the command line (terrain + unit-set + namers), not chained
#    automatically via base-module, so most never show up as "used" by
#    this script's mechanical scan. A few others (arena, simple, solar,
#    spec, bolodd3, eur-50km, mod-usa, mod-world) are standalone
#    scenarios simply not wired into lib/game.dir's menu.
KNOWN_GAPS='> africa
< arena
< awls-testbed
< bolodd3
< earth-2deg
< eur-50km
> greek
< mod-usa
< mod-world
< ng-nickname
< ng-sides
< omniterr
< simple
< solar
< spec
< t-africa
< u-battles
< u-battles-2
< u-e1-1998
< u-greek
< u-khan-2
< u-normandy'

unwaived=`echo "$diffs" | grep '^[<>]' | grep -vFx "$KNOWN_GAPS"`

if [ -n "$unwaived" ]; then
    echo "Unwaived differences:"
    echo "$unwaived"
    exit 1
fi

exit 0
