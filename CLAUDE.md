# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

Xconq is a general turn-based strategy game *system* (not a single game). A portable
C/C++ engine ("the kernel") interprets games written in **GDL** (Game Design Language),
a Lisp-like declarative language. The games library (`lib/*.g`) contains dozens of
scenarios (Empire-style default game, historical battles, fantasy/space, etc.). Multiple
swappable user interfaces sit on top of the same kernel.

Despite the `.c` extensions on many files, **everything is compiled as C++** (source
files are marked `LANGUAGE CXX`, with `-fpermissive -Wno-write-strings`, pinned to
**gnu++98** — newer standards make libstdc++ headers clash with the kernel's `min`/`max`
macros).

## Build & test

The build uses CMake (3.20+). Each UI builds only if its dependencies are found
(missing ones warn and skip); the kernel and `skelconq` always build.

```sh
cmake -B build                       # all UIs default ON
cmake --build build -j
ctest --test-dir build --label-exclude long   # 'long' tests play full games
cmake --install build
```

UI options (`-DXCONQ_UI_TCLTK/CURSES/SDL/X11=ON|OFF`) gate the four interfaces.
Executables: `xconq` (Tcl/Tk, the primary UI), `cconq` (curses), `sdlconq` (SDL 1.2 via
sdl12-compat), `xtconq` (legacy Xt/Xaw). Other knobs: `XCONQ_DATA_DIR`,
`XCONQ_SCORES_DIR`. Generated config headers (`acdefs.h`, `version.h`) land in
`build/include/`, from templates in `kernel/*.h.in`.

CI (`.github/workflows/c-cpp.yml`, Ubuntu) installs `tcl-dev tk-dev libncurses-dev
libsdl1.2-compat-dev libxaw7-dev libxmu-dev libxpm-dev`, builds all UIs, and runs
ctest without the `long` label.

### Running tests

Execution tests drive games headlessly through **`skelconq`** (kernel + null interface;
built as part of the normal build). CTest names mirror the old targets:

```sh
ctest --test-dir build -R check-lib     # or check-actions, check-save, check-test, check-ai
```

The scripts fail honestly (July 2026 rework; policy in `test/common.sh`): any game
that crashes fails the test, and a *playable* game (one listed in `lib/game.dir`) also
fails on any `Error:` output or logged warning, on failure to save, or on a
save/restore mismatch. Runs use `-w` so a warning doesn't abort the game; warnings
are collected from the `Xconq.Warnings` log instead, with the AI planner-recovery
class ("trying multiple bad actions") tolerated (`AI_TOLERATED`). Other modules are
include-fragments that legitimately warn when run standalone, so they are only
crash-checked. Each game run is bounded (10 min, 100 MB output, fixed random seed
`-R 1`) and runs under a hermetic `XCONQHOME` in the build dir. Ten modules with
known save-fidelity bugs are waived by name in `test-save.sh` (`KNOWN_UNFAITHFUL`).
Full per-game output still lands in `build/test/*.log`.

Consistency-check scripts also live in `test/` (`*-diff.sh`, `*-uses.sh`), runnable by hand:
- `check-lib` / `test-lib.sh` — load & run every library game module
- `check-actions`, `check-save`, `check-test` — action coverage, save/restore, special games
- `*-diff.sh` / `*-uses.sh` — consistency checks between docs, source `.def` symbols, and library games
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
  supply, GDL reader/writer `read.c`/`write` + `lisp.c`, save/restore, synthesis
  `mk*.c`, scoring, history). This is the bulk of the game logic. `run.c`/`run2.c` drive
  turns; `kernel.h` is the internal header, `conq.h`/`kpublic.h` the public API to UIs.
- **`kernel/ai*.c` / `ai*.cc`, `plan.c`, `mplayer.c`** — the AIs. GDL games are analyzed by
  a generic AI that infers how to play arbitrary rulesets.
- **UIs** (`tcltk/`, `curses/`, `sdl/`, `x11/`) — each provides a `main` and implements the
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
via `read.c` + `lisp.c`; the same machinery writes save files. When editing kernel code that
touches game state, remember that nearly every field is both GDL-readable and save/restore-able.
