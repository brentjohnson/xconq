#!/bin/sh

# NOTE (2026-07-05): this script is intentionally NOT wired into CTest.
#
# It used to silently compare against nothing: doc/syntax.texi's intro
# comment matched the /^@c Syntax/ start of its range-delete, but the
# migration to Texinfo's @smallexample (from @example) meant the
# `/^@c Syntax/,/^@example/d` pattern never found its end marker and
# deleted the *entire* file, so every symbol in the manual showed up as
# "missing from the chart." That part is fixed below.
#
# With the extraction actually working, ~530 of the manual's ~1700
# @deffn symbols still aren't listed in the syntax chart -- a large,
# real backlog (the chart was evidently never kept in lockstep with
# refman.texi as keywords were added over the years), but far too big
# to triage into per-symbol waivers the way sym-diff.sh's ~50 do. A
# waiver list at this scale would just be a blanket pass, not a useful
# check. Left as a manual/informational script until someone does a
# focused pass syncing doc/syntax.texi with doc/refman.texi, at which
# point it's worth registering in test/CMakeLists.txt like its siblings.

sed -e '/^@c Syntax/,/^@smallexample/d' -e 's/end smallexample/end_smallexample/' $1/../doc/syntax.texi | tr ' |()[]' '\012' | grep -v '^@[^@]' | sed -e 's/@@/@/' | grep -v '::=' | grep -v '\.\.\.' | grep -v '^zz-' | sort | uniq >syntax.syms

grep '@deffn' $1/../doc/refman.texi | grep '@code' | sed -e 's/^.*@code{\([^{ }]*\)}.*$/\1/' | sed -e 's/@@/@/' | grep -v '^zz-' | sort | uniq >doc.syms

echo "Differences between symbols in reference manual and in syntax chart:"
echo "('<' - in manual,  '>' - in syntax chart)"

diff -w doc.syms  syntax.syms | grep -v '^[0-9]' | grep -v '^---'

exit 0
