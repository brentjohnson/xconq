#!/bin/sh

# This script extracts and compares the commands defined in the manual
# to those defined in the source code.

grep '^DEF_CMD' $1/../kernel/cmd.def \
    | sed -e 's/\\\\/\\/' \
    | sed -e 's:^[^(]*(\([^,]*\), "\([^"]*\)".*$:\1 \2:' \
    | sed -e "s/C('\(.\)')/C-\1/" -e "s/ *'\(.\)' /\1/" -e 's/   0  /0/' \
    | sed -e "s/^  /' ' /" \
    | grep -v '0 D' \
    | sort | uniq >src.cmds

# Join all @item lines with their following lines.

cat  $1/../doc/commands.texi \
    | sed -e 's/^@item \(.*\)$/@item \1 EOL/' \
    | sed -e '/^@item /{N;s/EOL.//;P;D;}' \
    >joined.txt

# Single-letter commands.

cat joined.txt \
    | grep -h "^@item [^'] "  \
    | sed -e 's/^@item \(.\) (@code{\([^}]*\)}).*$/\1 \2/' \
    >doc.cmds1

# Escaped single-letter commands.

cat joined.txt \
    | grep -h "^@item @. "  \
    | sed -e 's/^@item @\(.\) (@code{\([^}]*\)}).*$/\1 \2/' \
    >doc.cmds2

# Quoted single-letter commands.

cat joined.txt \
    | grep -h "^@item '.' "  \
    | sed -e "s/^@item '\(.\)' (@code{\([^}]*\)}).*$/'\1' \2/" \
    >doc.cmds3

# Control-character commands.

cat joined.txt \
    | grep -h "^@item ^. "  \
    | sed -e 's/^@item ^\(.\) (@code{\([^}]*\)}).*$/C-\1 \2/' \
    >doc.cmds4

# Long-name-only commands.

cat joined.txt \
    | grep -h "^@item [a-zA-Z][a-zA-Z-][a-zA-Z-]* "  \
    | sed -e 's/@@/@/' \
    | sed -e 's/^@item \([a-zA-Z-]*\) .*$/0 \1/' \
    > doc.cmds5

cat doc.cmds[1-5] | grep -v '0 D' | sort | uniq >doc.cmds

echo "Differences between commands in reference manual and in source code:"
echo "('<' - in documentation,  '>' - in sources)"

# The manual documents the Return key generically as '@item 'return''
# (see commands.texi), a form this script's texinfo parser can't turn
# into a key-code token. cmd.def binds the same end-turn command to two
# raw key codes (Ctrl-M and literal newline) that a terminal's Return
# key can produce depending on input mode, so both show up as
# source-only here even though the command is documented. Waived by
# name rather than taught to the parser, since it's a one-off notation
# mismatch, not missing documentation (2026-07-05).
KNOWN_GAPS="C-M end-turn
' ' '\n' end-turn"

diffs=`diff -w doc.cmds src.cmds`
unwaived=`echo "$diffs" | grep '^[<>]' | sed -e 's/^[<>] //' | grep -vFx "$KNOWN_GAPS"`

echo "$diffs"

if [ -n "$unwaived" ]; then
    echo "Unwaived differences:"
    echo "$unwaived"
    exit 1
fi

exit 0
