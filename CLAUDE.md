# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

Xconq is a general turn-based strategy game *system* (not a single game). A portable
C/C++ engine ("the kernel") interprets games written in **GDL** (Game Design Language),
a Lisp-like declarative language. The games library (`lib/*.g`) contains dozens of
scenarios (Empire-style default game, historical battles, fantasy/space, etc.). Multiple
swappable user interfaces sit on top of the same kernel.

Everything is compiled as C++: the kernel sources use `.cc` extensions
(renamed from `.c` 7/2026), pinned to **gnu++17**.

## Build & test

The build uses CMake (3.20+). Each UI builds only if its dependencies are found
(missing ones warn and skip); the kernel and `skelconq` always build.

```sh
cmake -B build                       # all UIs default ON
cmake --build build -j
ctest --test-dir build --label-exclude long   # 'long' tests play full games
cmake --install build
```

`-DXCONQ_UI_SDL=ON|OFF` gates the sole UI. Executables: `sdlconq` (SDL3) and
`skelconq` (headless) both find `lib/`/`images/` from the source checkout
(`kernel/unix.cc`'s `default_library_pathname()`, compiled-in `XCONQ_SRCDIR`
fallback) even before `cmake --install`, so e.g. `build/sdl/sdlconq` run from
the repo root works uninstalled. `-L <path>` or `XCONQLIB`/`XCONQIMAGES` env
vars still override this for a non-default checkout layout. The Tcl/Tk
(`xconq`) and legacy Xt/Xaw (`xtconq`) UIs were removed 7/2026 — see
MODERNIZATION-PLAN.md's Step 2 note; the curses UI (`cconq`) was removed
7/2026 as well. Other knobs: `XCONQ_DATA_DIR`,
`XCONQ_SCORES_DIR`. Generated config headers (`acdefs.h`, `version.h`) land in
`build/include/`, from templates in `kernel/*.h.in`.

CI (`.github/workflows/c-cpp.yml`) is a matrix: on Ubuntu, GCC and Clang
each build Debug and Release (4 legs), installing `libxmu-dev` plus SDL3's
X11 build deps and building SDL3 from source (no `libsdl3-dev` package on
`ubuntu-latest` yet); the UI builds and the quick ctest lane
(`--label-exclude long`) runs on every leg. A macOS job (`brew install sdl3`)
builds the kernel and `skelconq` and runs the same quick ctest lane;
`sdlconq` needs X11 (it links Xlib/Xmu/Xext directly, not just SDL3 — see
`sdl/CMakeLists.txt`), which isn't present on the runner, so the top-level
`X11_FOUND` gate just skips it there. The macOS job is `continue-on-error:
true` pending a longer green track record. A separate `sanitizers` job
builds the kernel + `skelconq` with GCC on RelWithDebInfo and
`-DXCONQ_SANITIZE=address,undefined` (the toggle wires `-fsanitize` into
compile and link via `xconq_common`) and runs the quick ctest lane under
`ASAN_OPTIONS=detect_leaks=0` (leak checking is deferred — the kernel frees
almost nothing by design) and `UBSAN_OPTIONS=halt_on_error=1`; it configures
`-DXCONQ_TEST_TIMEOUT_SCALE=3` to widen the test timeouts (CTest `TIMEOUT`
and `test/common.sh`'s per-game bound) for the 2–5× sanitizer slowdown. All
legs build with `-j` and skip the `long` label.

### Running tests

Execution tests drive games headlessly through **`skelconq`** (kernel + null interface;
built as part of the normal build). `check-lib`, `check-actions`, `check-save`, and
`check-test` are split one CTest per game module (`check-lib-<module>`, etc., generated
by globbing `lib/*.g`/`test/*.g` at configure time in `test/CMakeLists.txt`), so a
failure names the exact `.g` file and `ctest -j` runs modules in parallel. `ctest -R
check-lib` still matches all of them (substring regex):

```sh
ctest --test-dir build -j$(nproc) --label-exclude long   # full quick lane, parallel
ctest --test-dir build -R check-lib                       # or check-actions, check-save, check-test
ctest --test-dir build -R check-ai                        # aggregate sweep (label "long")
```

The scripts fail honestly (July 2026 rework; policy in `test/common.sh`, shared by the
aggregate and per-module tests alike): any game that crashes fails the test, and a
*playable* game (one listed in `lib/game.dir`) also fails on any `Error:` output or
logged warning, on failure to save, or on a save/restore mismatch. Runs use `-w` so a
warning doesn't abort the game; warnings are collected from the `Xconq.Warnings` log
instead, with the AI planner-recovery class ("trying multiple bad actions") tolerated
(`AI_TOLERATED`). Other modules are include-fragments that legitimately warn when run
standalone, so they are only crash-checked. Each game run is bounded (10 min, 100 MB
output, fixed random seed `-R 1`) and runs under a hermetic `XCONQHOME`. Ten modules
with known save-fidelity bugs are waived by name in `test-save.sh` (`KNOWN_UNFAITHFUL`).
Each per-module test gets its own scratch working directory
(`build/test/scratch/check-<suite>-<module>/`) so concurrent runs can't collide on
`XCONQHOME`/saves/logs; that's where its `*.log` lands. Running a script with no module
argument (e.g. `sh test-lib.sh <srcdir>`) still sweeps every module from `build/test/`
directly, for manual use.

`test/unit/` holds `unittests` (CTest `unittests`, label `unit`, `ctest -L unit`), a small
C++ binary of pure-kernel unit tests (hand-rolled `CHECK` macro, no external framework)
that links the kernel directly instead of driving a whole game through skelconq: GDL
reader/writer round-trips and malformed-input handling (`kernel/lisp.cc`), the `.def`
type/table/gvar machinery via `interp_form(NULL, ...)` on inline GDL fragments, and dice/
pm-scale utility helpers (`kernel/misc.h`). It links `kernel/skelconq_stubs.cc`, which
holds the required-interface callback stubs every UI must supply (factored out of
`skelconq.cc`, which keeps only its interactive command loop); a `tolerate_warnings` flag
in that shared file lets `unittests` inspect a deliberately provoked reader warning
instead of exiting the way skelconq's warning handling otherwise always does.

Consistency-check scripts also live in `test/` (`*-diff.sh`, `*-uses.sh`):
- `check-lib` / `test-lib.sh` — load & run every library game module (per-module, or all via no module arg)
- `check-actions`, `check-save`, `check-test` — action coverage, save/restore, special games (same split)
- `*-diff.sh` — cross-check docs, source `.def` symbol tables, and library games; three are wired into
  CTest as `check-consistency-<name>` (label `consistency`, `ctest -L consistency`): `cmd-diff.sh`
  (manual commands vs `cmd.def`), `game-diff.sh` (defined vs. used library games), `sym-diff.sh` (manual
  keywords vs. `.def` symbols). Each exits nonzero only on *unwaived* differences — known-legitimate
  gaps (dead references from the original 20-year-old import, tables/properties the manual documents as
  removed, etc.) are listed by name in a `KNOWN_GAPS` block at the top of the script, same house style as
  `test-save.sh`'s `KNOWN_UNFAITHFUL`. `imf-diff.sh` and `syntax-diff.sh` are excluded from CTest — a
  dated comment in each explains why (the real drift they find, ~1750 and ~530 symbols respectively, is
  too large to waive item-by-item and would just be a disguised blanket ignore); they're still runnable
  by hand.
- `*-uses.sh` (`game-uses.sh`, `imf-uses.sh`, `lib-uses.sh`, `src-uses.sh`) — usage-count reports for
  manual triage, not pass/fail checks; not wired into CTest.
- `.g` files are GDL games, `.inp` files are scripted command input, `.imf` are image families

### Old build system

The historical autoconf build (`configure.in`, `Makefile.in` per directory) and an
abandoned automake rework in `src/` were removed in the CMake migration — they exist
only in git history.

## Architecture

### The `.def` X-macro system (read this before touching kernel data structures)

The kernel's core types, properties, tables, actions, tasks, and commands are defined once
in `kernel/*.def` files and expanded multiple ways via the X-macro pattern. Each `.def` file
is a list of `DEF_*(...)` invocations; a header `#define`s `DEF_*` then `#include`s the file
to generate enums, struct fields, property-accessor tables, GDL symbol tables, etc.

Examples: `utype.def` (unit-type properties), `mtype.def` (materials), `ttype.def` (terrain),
`gvar.def` (global game variables), `table.def` (interaction tables), `action.def`, `task.def`,
`plan.def`, `goal.def`, `cmd.def` (UI commands), `keyword.def` (GDL syntax).

**Consequence:** to add a unit property, GDL keyword, action, etc., edit the relevant `.def`
file — do not hand-edit the generated enums/tables. Search for a `DEF_` name to find every
site that consumes it. `test/*-diff.sh` verify that `.def` symbols, docs, and library usage
stay in sync.

### Directory map

- **`kernel/`** — the portable engine (world model, units, combat, movement, economy,
  supply, GDL reader/writer `read.cc`/`write` + `lisp.cc`, save/restore, synthesis
  `mk*.cc`, scoring, history). This is the bulk of the game logic. `run.cc`/`run2.cc` drive
  turns; `kernel.h` is the internal header, `conq.h`/`kpublic.h` the public API to UIs.
- **`kernel/ai*.cc`, `plan.cc`, `mplayer.cc`** — the AIs. GDL games are analyzed by
  a generic AI that infers how to play arbitrary rulesets.
- **UIs** (`sdl/`) — each provides a `main` and implements the
  interface callbacks the kernel expects. They link the kernel as `libconq.a` / `libconqlow.a`.
- **`lib/`** — the GDL games library (`*.g`). This is data, but it is where most "content"
  changes happen and is exercised by the test suite.
- **`images/`, `bitmaps/`, `sounds/`** — media assets; image families (`.imf`) map game
  symbols to bitmaps.
- **`doc/`** — Texinfo manuals (user guide + game designer's manual describing all GDL
  properties/tables) and `INSTALL*` files.

### GDL in one paragraph

GDL syntax resembles Lisp but is declarative: you `define` unit/material/terrain/advance
types and set properties, then fill in interaction tables (e.g. hit/damage between unit
types, movement cost by terrain). A game module can `include` others. The kernel reads GDL
via `read.cc` + `lisp.cc`; the same machinery writes save files. When editing kernel code that
touches game state, remember that nearly every field is both GDL-readable and save/restore-able.
