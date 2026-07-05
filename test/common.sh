# Shared helpers for the Xconq test scripts.  POSIX sh; source after
# setting $srcdir:
#
#	srcdir=$1
#	. $srcdir/common.sh
#
# Each script runs skelconq over a set of game modules with run_one and
# calls finish, which exits nonzero if anything failed.  Failure policy:
#
#  - A crash (signal death, e.g. segfault/assertion, or timeout) fails
#    for every module.
#  - A playable game (one listed in lib/game.dir, the list shown in new
#    game dialogs) also fails on a nonzero exit status or any
#    Error:/Warning: output.  Warnings are suppressed with -w so that a
#    warning does not abort the run, and the Xconq.Warnings log (which
#    the kernel keeps even when warnings are suppressed) is inspected
#    instead; the known AI planner-recovery warnings are tolerated
#    (see AI_TOLERATED and MODERNIZATION-PLAN.md).
#  - Other modules (include-fragments, maps whose base game is elsewhere,
#    experiments) are only expected not to crash: run standalone they
#    legitimately produce warnings about types their parent modules
#    define.

# Default assumes cwd is the build's test/ directory (true for aggregate
# and manual runs); the per-module CTest entries run from their own
# scratch subdirectory and pass an absolute $SKELCONQ instead.
SKELCONQ=${SKELCONQ:-../kernel/skelconq}

if [ ! -x $SKELCONQ ] ; then
	echo "Error: $SKELCONQ not found, exiting"
	exit 1
fi

# Keep saves, scores, and preferences out of the user's ~/.xconq.
XCONQHOME=`pwd`/xconq-home
export XCONQHOME
rm -rf "$XCONQHOME"
mkdir -p "$XCONQHOME"

# Bound each run: a hung game dies after 10 minutes instead of eating
# the whole CTest timeout, and a runaway one (e.g. an AI planning loop
# printing forever) hits the 100 MB output cap and dies with SIGXFSZ
# instead of filling the disk. XCONQ_TIMEOUT_SCALE (set by the CTest
# harness, default 1) widens the bound for slow sanitized runs, matching
# the scaled CTest TIMEOUT property.
XCONQ_TIMEOUT_SCALE=${XCONQ_TIMEOUT_SCALE:-1}
RUN_TIMEOUT=`expr 600 \* "$XCONQ_TIMEOUT_SCALE"`
if command -v timeout >/dev/null 2>&1 ; then
	TIMEOUT="timeout $RUN_TIMEOUT"
else
	TIMEOUT=
fi
MAXOUTBLOCKS=204800	# 512-byte blocks = 100 MB

failures=
nrun=0

# Set ALL_STRICT=yes to apply the playable-game policy to every module
# (used for the test/ modules, which should be diagnostic-clean even
# though they are not in game.dir).
ALL_STRICT=

is_full_game () {
	[ -n "$ALL_STRICT" ] && return 0
	grep -q "^\"`basename $1 .g`\"\$" $srcdir/../lib/game.dir
}

# run_skelconq <output-file> <input-file> <args...>
# Run one bounded skelconq under the hermetic home; status is returned.
run_skelconq () {
	out=$1; inp=$2; shift 2
	( ulimit -f $MAXOUTBLOCKS 2>/dev/null
	  exec $TIMEOUT $SKELCONQ "$@" < $inp > $out 2>&1 )
}

# fail <game-file> <why>
fail () {
	echo "FAIL: `basename $1`: $2" | tee -a $logname
	failures="$failures `basename $1`"
}

# The generic AI occasionally issues actions that cannot succeed; the
# kernel recovers by clearing the offending unit's plan, with a runtime
# warning.  Those planner deficiencies are a known follow-up (see
# MODERNIZATION-PLAN.md), so any suite that lets the AI act tolerates
# exactly these warnings and no others.
AI_TOLERATED='trying multiple bad actions|was asked to act'

# run_one <game-file> <input-file> [extra skelconq args...]
# Run one module with a fixed random seed and append its output to
# $logname, applying the failure policy above.
run_one () {
	game=$1; inp=$2; shift 2
	nrun=`expr $nrun + 1`
	echo "" >> $logname
	echo ">>> Running skelconq on: $game <<<" >> $logname
	echo "" >> $logname
	rm -f Xconq.Warnings
	run_skelconq run-one.out $inp "$@" -w -R 1 -f "$game" -L $srcdir/../lib
	status=$?
	cat run-one.out >> $logname
	if [ $status -ge 124 ] ; then
		fail $game "crashed (exit status $status)"
	elif is_full_game "$game" ; then
		if [ $status -ne 0 ] ; then
			fail $game "exit status $status"
		elif grep -q '^Error:' run-one.out ; then
			fail $game "reported errors"
		elif [ -f Xconq.Warnings ] \
		     && grep -Ev "$AI_TOLERATED" Xconq.Warnings | grep -q . ; then
			echo ">>> Unexpected warnings from $game: <<<" >> $logname
			grep -Ev "$AI_TOLERATED" Xconq.Warnings >> $logname
			fail $game "logged unexpected warnings"
		fi
	fi
}

# run_expect_diag <game-file> <input-file> [extra skelconq args...]
# Like run_one, but for modules that exist to exercise error handling:
# they must produce at least one diagnostic (and still must not crash).
# Warnings only land in Xconq.Warnings because of -w.
run_expect_diag () {
	game=$1; inp=$2; shift 2
	nrun=`expr $nrun + 1`
	echo "" >> $logname
	echo ">>> Running skelconq on: $game (diagnostics expected) <<<" >> $logname
	echo "" >> $logname
	rm -f Xconq.Warnings
	run_skelconq run-one.out $inp "$@" -w -R 1 -f "$game" -L $srcdir/../lib
	status=$?
	cat run-one.out >> $logname
	if [ $status -ge 124 ] ; then
		fail $game "crashed (exit status $status)"
	elif grep -q '^Error:' run-one.out || [ -s Xconq.Warnings ] ; then
		: # produced the expected diagnostics
	else
		fail $game "expected diagnostics but produced none"
	fi
}

# finish: write the trailer, report, and set the exit status.
finish () {
	echo "Test finished on `date` ($nrun modules)" >> $logname
	if [ -n "$failures" ] ; then
		echo "FAILED ($nrun modules run):$failures"
		exit 1
	fi
	echo "All $nrun modules passed."
	exit 0
}
