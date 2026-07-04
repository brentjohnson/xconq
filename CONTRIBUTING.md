# Contributing to Xconq

## What this is

Xconq is a general turn-based strategy game *system* (not a single game). A
portable C/C++ engine ("the kernel") interprets games written in **GDL** (Game
Design Language), a Lisp-like declarative language. The games library
(`lib/*.g`) contains dozens of scenarios (an Empire-style default game,
historical battles, fantasy/space games, and more), and multiple swappable
user interfaces sit on top of the same kernel.

## Building

The build uses CMake (3.20+). Each UI builds only if its dependencies are
found (a missing one just prints a warning and skips that UI); the kernel and
`skelconq` (the headless null-interface binary used for testing) always
build.

```sh
cmake -B build
cmake --build build -j
cmake --install build
```

UI options, all `ON` by default:

- `-DXCONQ_UI_TCLTK=ON|OFF` — Tcl/Tk interface, installed as `xconq`
- `-DXCONQ_UI_CURSES=ON|OFF` — curses interface, `cconq`
- `-DXCONQ_UI_SDL=ON|OFF` — SDL 1.2 interface (via sdl12-compat), `sdlconq`
- `-DXCONQ_UI_X11=ON|OFF` — legacy Xt/Xaw interface, `xtconq`

Other useful cache variables: `XCONQ_DATA_DIR` (installed location of the
games library/images/UI scripts) and `XCONQ_SCORES_DIR` (runtime score file
location). Generated config headers (`acdefs.h`, `version.h`) land in
`build/include/`, produced from templates in `kernel/*.h.in`.

Despite the `.c` extensions on most kernel files, **everything is compiled as
C++** (sources are marked `LANGUAGE CXX` in CMake, built with `-fpermissive
-Wno-write-strings`, pinned to **gnu++98**) — newer C++ standards make
libstdc++ headers clash with the kernel's `min`/`max` macros. Don't be
surprised to see C-style code; treat it as C++ for compiler-flag purposes.

## Testing

```sh
ctest --test-dir build --label-exclude long
```

Execution tests drive real games headlessly through `skelconq`. CTest names
mirror the historical make targets:

```sh
ctest --test-dir build -R check-lib      # load & run every library game module
ctest --test-dir build -R check-actions  # action coverage
ctest --test-dir build -R check-save     # save/restore round-trips
ctest --test-dir build -R check-test     # special/diagnostic games
ctest --test-dir build -R check-ai       # AI self-play (label "long", hours)
```

**What failure means** (the policy lives in `test/common.sh`): a crash
(segfault, assertion, timeout) always fails the test, for any module. A
*playable* game — one listed in `lib/game.dir`, the curated list shown in
the new-game dialog — is held to a higher bar and also fails on any
`Error:`/`Warning:` output, on failing to save, or on a save/restore mismatch.
Everything else under `lib/` is an include-fragment or similar that
legitimately produces warnings when run standalone (it's exercising code that
belongs to some other module), so those are only checked for crashes. Runs
use `-w` so a warning doesn't abort the game in progress; warnings are
collected from the `Xconq.Warnings` log afterward, tolerating only the AI's
known planner-recovery class ("trying multiple bad actions"). Each game run
is bounded (10 minute timeout, 100 MB output cap, fixed random seed `-R 1`)
and executes under a hermetic `XCONQHOME` inside the build directory, so
tests never touch your real `~/.xconq`.

To run one game by hand under `skelconq` the way the test scripts do:

```sh
build/kernel/skelconq -w -R 1 -f lib/germany.g -L lib
```

Full per-game output from a CTest run lands in `build/test/*.log`.

A few consistency-check scripts also live in `test/` but are not wired into
CTest — run them by hand, passing the test directory as the argument (e.g.
`./sym-diff.sh .`): `sym-diff.sh`/`syntax-diff.sh` diff documented symbols
against `.def`-file symbols, and `src-uses.sh`/`lib-uses.sh` check that every
symbol is actually referenced somewhere in the kernel/library.

## The `.def` X-macro system

The kernel's core types, properties, tables, actions, tasks, and commands are
defined once in `kernel/*.def` files and expanded multiple ways via the
X-macro pattern: a header `#define`s a `DEF_*` macro, then `#include`s the
`.def` file to generate an enum, struct fields, a GDL property-accessor
table, etc. — the same list of properties fans out into several different
generated forms depending on which macro was active at `#include` time.

**Consequence:** to add or change a unit property, GDL keyword, action, and
so on, edit the relevant `.def` file. Never hand-edit the generated
enums/tables — they're regenerated from the `.def` file every time it's
included. Search the tree for the `DEF_` macro name (e.g. `DEF_UPROP_I`) to
find every site that consumes it.

Worked example: `kernel/utype.def` lists every unit-type property, e.g.

```c
DEF_UPROP_I("acp-to-move", u_acp_to_move, ...)
```

Adding a new `DEF_UPROP_I(...)` line there is enough to make the property
available as a GDL keyword (readable/writable in game modules), included in
save files, and covered by the `.def`/docs consistency checks — no other file
needs to change. `kernel/types.c` and `kernel/game.h` are two of the sites
that `#include "utype.def"` to expand it into different things (struct layout
vs. GDL symbol table).

## Source conventions

- Everything compiles as C++, per the build note above — new code should be
  valid C++ even in files with a `.c` extension.
- There's no `.clang-format` in the tree yet; match the surrounding file's
  style rather than reformatting wholesale, and if you do introduce
  formatting tooling later, apply it only to lines you actually touch.

## Where game content lives

Game modules are `lib/*.g`, written in GDL. `lib/game.dir` is the curated
list of "real" playable games shown in the new-game dialog — see Testing
above for why membership in that list raises the bar for what counts as a
passing test run. `.inp` files under `test/` are scripted command input used
to drive games in the execution tests, and `.imf` files map game symbols to
bitmap images.
