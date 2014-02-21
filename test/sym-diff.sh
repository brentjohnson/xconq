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

diff -w doc.syms  src.syms | grep -v '^[0-9]' | grep -v '^---'

exit 0
