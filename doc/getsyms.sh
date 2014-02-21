#!/bin/sh

# This script extracts GDL symbols defined in the input file.

grep '@deffn' $1 | grep '@code' | grep -v '@c ' | sed -e 's/^.*@deffn \([^ ]*\) @code{\([^{ }]*\)}.*$/\2 \1/' | sed -e 's/@@/@/' | grep -v '^zz-' | sort | uniq

exit 0
