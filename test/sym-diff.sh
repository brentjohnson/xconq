#!/bin/sh

# This script extracts and compares the symbols defined in the manual
# to those defined in the source code.

grep DEF_ $1/../kernel/*.def | grep -v DEF_CMD | sed -e 's/^[^"]*"\([^ ]*\)".*$/\1/' | grep -v '^zz-' | sort | uniq >src.syms

grep '@deffn' $1/../doc/refman.texi | \
  grep '@code' | \
  sed -e 's/^@deffn [a-zA-Z]* @code{\([^{ }]*\)}.*$/\1/' | \
  sed -e 's/@@/@/' | \
  grep -v '^zz-' | \
  sort | uniq >doc.syms

echo "Differences between symbols in reference manual and in source code:"
echo "('<' - in documentation,  '>' - in sources)"

diffs=`diff -w doc.syms  src.syms | grep -v '^[0-9]' | grep -v '^---'`
echo "$diffs"

# Waiver list of known-legitimate mismatches (2026-07-05):
#
#  - "capacity-negation" and "occupant-can-have-occupants" are
#    TableUT/TableUU entries the manual keeps on purpose, each marked
#    "This table is no longer available" -- historical documentation
#    for a removed table, not an oversight.
#
#  - Everything else here is source-only: keyword/property/table/gvar
#    additions (mostly newer AI worth-tuning variables, attack/defense
#    modifier tables, and cursor/color rendering hooks) that were
#    never written up in refman.texi, plus the internal "none" task
#    type (an idle placeholder, never meant to be written into GDL).
#    Writing the missing manual prose is real doc work belonging to a
#    separate pass, not part of wiring this check into CI.
KNOWN_GAPS='< capacity-negation
< occupant-can-have-occupants
> ai-base-worth
> ai-base-worth-for
> ai-depot-worth
> ai-distributor-worth
> ai-enemy-alert-range
> ai-explorer-worth
> ai-mover-worth
> ai-prod-enhancer-worth
> ai-producer-worth
> ai-seer-worth
> attack-cannot-hit
> attack-cursor
> construct
> consumption-per-change-type
> consumption-per-toolup
> could-develop
> could-research
> extract-no-source
> feature-boundaries
> feature-color
> filename
> fire-blocked
> fire-cannot-hit
> fore-color
> full-cxp-affects-attack
> full-cxp-affects-defense
> generic-cursor
> hexgrid
> indepside-has-economy
> insufficient-ammo
> insufficient-tooling
> mask-color
> meridians
> neighbour-affects-attack
> neighbour-affects-defense
> none
> no-path
> occ-cannot-do
> occupant-affects-attack
> occupant-affects-defense
> overrun-cannot-hit
> repair-range
> shorelines
> size-max
> size-min
> survey-cursor
> tech-to-change-type-to
> terrain-affects-attack
> terrain-affects-defense
> terrain-transitions
> text-color
> transport-affects-attack
> transport-affects-defense
> unit-died-from-attrition
> unit-separation-min
> unit-wrecked-from-attrition
> window-color'

unwaived=`echo "$diffs" | grep '^[<>]' | grep -vFx "$KNOWN_GAPS"`

if [ -n "$unwaived" ]; then
    echo "Unwaived differences:"
    echo "$unwaived"
    exit 1
fi

exit 0
