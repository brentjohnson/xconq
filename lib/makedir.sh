#!/bin/sh

# Create the imf dir file.
# The perl version in makedir.pl detects some errors that this one will
# miss, but otherwise they're identical in behavior.

echo "ImageFamilyName FileName"

grep '(imf ' $* |  \
sed -e 's/\(.*\):(imf "\([-a-z0-9]*\)".*$/\2 \1/' |  \
grep -v : |  \
sort |  \
uniq

echo ". ."
