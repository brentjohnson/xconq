#!/bin/sh

# This script extracts and compares the symbols defined in the syntax chart
# to those defined in the manual.

sed -e '/^@c Syntax/,/^@example/d' -e 's/end example/end_example/' $1/../doc/syntax.texi | tr ' |()[]' '\012' | grep -v '^@[^@]' | sed -e 's/@@/@/' | grep -v '::=' | grep -v '\.\.\.' | sort | uniq >syntax.syms

grep '@deffn' $1/../doc/refman.texi | grep '@code' | sed -e 's/^.*@code{\([^{ }]*\)}.*$/\1/' | sed -e 's/@@/@/' | sort | uniq >doc.syms

echo "Differences between symbols in reference manual and in syntax chart:"
echo "('<' - in manual,  '>' - in syntax chart)"

diff -w doc.syms  syntax.syms | grep -v '^[0-9]' | grep -v '^---'

exit 0
