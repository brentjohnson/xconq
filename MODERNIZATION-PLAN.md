# Xconq Modernization Plan

A roadmap for bringing a 1987–2005 codebase up to modern standards. Items are
grouped by theme; each carries a rough effort tag (**S**mall: hours, **M**edium:
days, **L**arge: weeks). Within a section, items are ordered by suggested
sequence — earlier items unblock or de-risk later ones.

Step 0 (done, July 2026): replaced autoconf with CMake, removed the abandoned
`src/` automake rework, wired the test suite into CTest, fixed the bit-rot
needed to get all four UIs + tools compiling again, CI green on Ubuntu.

Step 1 (done, July 2026): test honesty + the §2 bug fixes. The `test-*.sh`
scripts now fail on crashes for every module and on any diagnostic/save
mismatch for playable games (those in `lib/game.dir`); runs are seeded,
bounded (time + output size), and use a hermetic `XCONQHOME` (see
`test/common.sh`). `check-auto` and `check-ai` are fixed and re-enabled.
Honest tests immediately exposed and led to fixes for: a NULL-`dside` crash
in `update_research_display` (skelconq/xcscribe) that killed every
advances-based game under skelconq, a segfault on unit forms naming
nonexistent sides (`read.c`/`set_unit_side`), a segfault probing terrain
before any terrain is defined (`init.c`/`patch_object_references`),
`goal_truth` panicking on `no-goal`/`keep-formation`, the skelconq free-run
infinite loop on games that never start (the `check-ai` hang), an AI-test
crash indexing the `acp-retreat` table with `NONUTYPE` when rescuing
occupants (`combat.c`), a broken save-file comparison that had made
`check-save` a no-op, and a pile of stale GDL in `test/*.g` and `lib/`
(valhalla's unplaceable Listening Post, ancient `by-name` syntax, removed
properties/goals in `all.g`).

The revived `check-save` then exposed that save/restore was deeply broken,
all fixed: reloading a save silently loaded the *original library game*
instead of the saved state (the save's `(filename ...)` property clobbered
the real filename and the module was re-resolved by name); `fprintlisp`
dumped a table's entire contents after any symbol naming a table, corrupting
every saved module-variant clause (and growing the save each cycle); table
clauses were written as unreadable glued symbols like `u*0` (missing space
in `add_num_or_dice_to_form`); type objects inside stored forms were written
as unreadable `u#n`/`a#n` debug notation (now printed as type names, via
hooks installed by `init_types`); unit views were written with literal
`"(null)"` names and read positionally as garbage; restored unit views were
never linked to their units and so were flushed and re-dated on first
sight; restoring re-rolled probabilistic see-chances while placing units
(views now stay authoritative during pre-game init of a restore); and the
AI-computed `ai-*-worth` properties declared a lower bound of -1 although
the AI stores negative pm-scale values, making every save of an AI game
unreadable. Also fixed along the way: sides defined before their base
module's types left every per-type side array (and the terrain subtype
chains) undersized — a memory-corruption class that made `spec.g` hang in
terrain generation; a one-byte heap overflow in the GDL reader's token
buffer (`read_delimited_text` wrote the NUL terminator past `BIGBUF`); the
help system warned on empty or quoted `notes`; `execute_action` now
rejects invalid type arguments and inactive actors instead of tripping
assertions (scripted, network, and saved actions are untrusted).

Remaining known save-fidelity gaps, waived explicitly in `test-save.sh`
(`KNOWN_UNFAITHFUL`) and to be burned down later: some views of a side's
own stacked units get re-dated/re-ordered on restore (dates and per-cell
order are normalized in the comparison); ten modules (awls-*, opal-*,
mod-usa, omniterr, spec, u-e1-1938, voyages) still fail the equality check
for deeper reasons — weather-view re-derivation on restore, the indepside
in-game decision being recomputed from tables rather than restored, and
layer drift in some experimental modules.

Step 2 (done, July 2026): removed the `tcltk/` (`xconq`, the primary GUI, plus
`imfapp`) and `x11/` (legacy Xt/Xaw `xtconq`, its `x2imf`/`imf2x`/`xshowimf`
image tools, and the bundled 1990 `SelFile` widget) UI clients, including the
vendored `BWidget` Tcl toolkit copy and the `xconq.desktop`/`xconq.png`
launcher (which only launched the now-gone `xconq` binary). The Tcl/Tk UI
only built against Tcl/Tk 8.x and was already skipped on Tcl-9-only distros;
the Xt/Xaw UI had been dropped from the old build for decades before the
CMake migration briefly revived it. `curses` (`cconq`) and `sdl` (`sdlconq`)
remain, and `sdlconq` is now the primary/default graphical client and the
sole X11 consumer. Build options, CI packages, and docs (`CLAUDE.md`,
`CONTRIBUTING.md`, `README`, `doc/INSTALL`, `doc/commands.texi`) were updated
accordingly; the §4 items about the Tcl/Tk 9 port, the Xt/Xaw fate decision,
and the BWidget vendor-audit are resolved by this removal (see §4 below).

Step 3 (done, July 2026): removed the `curses/` UI client (`cconq`), the last
UI besides `sdlconq`. `sdlconq` is now the sole UI; `-DXCONQ_UI_SDL` is the
only remaining UI build option. Removed `HAVE_CURSES_LIB`/`HAVE_NCURSES_LIB`/
`HAVE_PDCURSES_LIB` from `kernel/acdefs.h.in` and the `find_package(Curses)`
detection from the top-level `CMakeLists.txt`, along with the curses-only
`impl_build` `x == -1 && y == -1` special case in `kernel/ui.cc` (only
`curses/ccmd.cc` ever passed that sentinel; `sdl/sdlcmd.cc` always passes a
real position). Build options, CI packages (`libncurses-dev` dropped), and
docs (`CLAUDE.md`, `CONTRIBUTING.md`, `README`, `doc/INSTALL`,
`doc/commands.texi`, `doc/refman.texi`, `doc/hacking.texi`,
`test/fuzz/README.md`) were updated accordingly. §4's curses/PDCurses items
below are stale as a result and should be read as historical.

Step 4 (planned, adopted 2026-07-11): the **client/server rearchitecture** —
the kernel becomes server-only (`xconqd`), the SDL client and a new browser
client become thin clients over a new WebSocket protocol, and the legacy
lockstep protocol (`kernel/tp.cc`/`socket.cc`) is removed. The decision record
and target architecture live in **ARCHITECTURE.md**; the task-by-task work
plan is **§10** below. This supersedes §9.2 (the successor-UI design study)
and changes the disposition of several §6 items — each carries a dated note.

## 1. Language & toolchain

> **All of §1 is complete (7/2026)** — every task below is done, so the
> per-task execution prompts have been removed. The prompt convention they
> established is still used by the open items in §3 and §5–§9: each carries a
> ready-to-run fenced prompt written so a fresh session of a smaller model can
> execute it without extra context (CLAUDE.md loads automatically and covers
> build/test basics), labeled with a recommended model — **Opus** for tasks
> needing judgment about intent or real-bug triage, **Sonnet** for mechanical
> error-driven sweeps.

- ~~**[M] Make the code valid without `-fpermissive`.**~~ *(done 7/2026)*: Fix
  what the flag papers over (implicit conversions, old-style casts of function
  pointers, etc.) and drop it. This is a prerequisite for everything below and
  for building with Clang, which lacks an equivalent. In the event, the prior
  modernization steps had already cleaned up the offending idioms: dropping the
  `-fpermissive` generator expression from `xconq_common` left the whole tree
  (kernel, curses, SDL) building clean under GCC 15.2.1 with the full test
  suite green, so no source changes were needed.

- ~~**[M] Fix string-literal constness and drop `-Wno-write-strings`.**~~
  *(done 7/2026)*: The CMake migration already fixed ~20 `const char *`
  signature mismatches; this pass finished the job. In the event, forcing the
  tree through `-Werror=write-strings` surfaced only one remaining offender:
  the `char *default_player_spec` global, which each UI's
  `make_default_player_spec()` set to a string literal. That global cannot be
  `const` — `sdl/sdlunix.cc` builds it in place with `strcpy`/`strcat`, and
  `parse_player_spec()` chops it destructively — so the literal assignments in
  `cconq`, `skelconq`, `xcscribe`, and `sdlwin32` were changed to
  `copy_string(...)` instead. With that, `-Wno-write-strings` was removed and
  the whole tree (kernel, curses, SDL, skelconq) builds clean with the test
  suite green.

- ~~**[S] Replace the `min`/`max` macros in `kernel/misc.h`** with inline
  functions (or `std::min`/`std::max`).~~ *(done 7/2026)*: replaced with
  `template<typename T> inline T min/max(T x, T y)` under the same lowercase
  names, so no call site needed renaming. That surfaced ~65 mixed-type call
  sites (e.g. `min(int, long)`, `min(int, short)`) that compiled under the
  macro's implicit promotion but failed template deduction; each got an
  explicit template argument (`min<long>(...)`, etc.) matching the type the
  macro's usual-arithmetic-conversion would have picked, preserving value
  ranges. A scratch `-DCMAKE_CXX_STANDARD=17` build confirmed this was the
  sole header clash blocking C++17 — it now builds clean end to end with no
  other errors.

- ~~**[M] Bump the C++ standard incrementally: gnu++98 → 11 → 17.**~~ *(done
  7/2026)*: both bumps were nearly free. C++11 (`CMAKE_CXX_STANDARD_REQUIRED`
  now `ON`) built with zero source changes — no narrowing-conversion or
  literal-concatenation fallout. C++17 also built clean; the only expected
  breakage (`register`, deprecated since C++11 and slated for removal in
  C++20) was still just a `-Wregister` warning under GCC 15, not an error,
  but the keyword was stripped anyway from the 4 compiled files that used it
  (`obstack.c`, `snprintf.c`, `unit.c`, `gif.c`) since it's dead weight ahead
  of C++20. No `throw(...)` specs or `auto_ptr`/`binder1st`-era usage existed
  to clean up. Both UIs build and the quick ctest suite passes at each stage.

- ~~**[M] Rename `.c` files that are compiled as C++ to `.cc`** (or `.cpp`) and
  drop the `LANGUAGE CXX` override in CMake.~~ *(done 7/2026)*: renamed all 53
  live .c files (50 in kernel/, 3 in curses/) with `git mv` to preserve
  history; `kernel/win32.c` (dead on non-Windows builds) and the already-unused
  `kernel/path.c` were left as `.c`, per plan. Updated both CMakeLists.txt
  source lists to the new names, deleted the `xconq_cxx_sources()` helper and
  its call sites, fixed `test/src-uses.sh`'s `*/*.[hc]` glob (it was silently
  missing the pre-existing `.cc` AI/SDL files, and would have missed
  everything once the rename landed) to also match `*/*.cc`, and updated
  stale `.c` filenames in CLAUDE.md, doc/hacking.texi, doc/refman.texi, and a
  test/all.g comment. Fresh configure + build of both UIs, the quick ctest
  suite, and all `test/*-diff.sh` / `*-uses.sh` scripts run clean.

- ~~**[M] Turn on real warnings (`-Wall -Wextra`) and burn the backlog down.**~~
  *(done 7/2026)*: Added `-Wall -Wextra` to `xconq_common` (informational) plus a
  curated hard-error subset — `-Werror=return-type,format,format-security,`
  `implicit-fallthrough,uninitialized` and GCC's `-Werror=maybe-uninitialized`
  (the last needs an `-O2`/Release build to fire). `-Werror=format` is scoped
  back with `-Wno-error=format-overflow,-truncation` so only format-string /
  security correctness is fatal, not buffer sizing. Burned the subset to zero in
  both Debug and Release; both UIs build and `ctest --label-exclude long` is green
  in both configs. Roughly 90 `maybe-uninitialized` sites were fixed — mostly
  false positives quieted with a value-init + comment, but several were real
  bugs (see below). Remaining non-subset warnings (a future pass): ~736 Debug /
  ~773 Release, dominated by `-Wunused-parameter` (453), `-Wunused-but-set-`
  `variable` (116), and `-Wmissing-field-initializers` (58).
  Genuine bugs found and fixed while burning down:
  - `curses/cconq.cc` `play_movies`: the `movie_nuke` case read `unit->x/y` off
    an uninitialized `unit`; nukes carry the target cell in `args[0..1]` (per the
    SDL UI), so it now uses those directly.
  - `kernel/read.cc` `interp_unit`: the default-tooling copy loop iterated `u2`
    but indexed with `u` (only set on the type-symbol path) — fixed to `u2`.
  - `kernel/read.cc` `interp_unit_plan`: `plan` was only set when the unit had no
    plan yet; a unit with an existing plan wrote through an uninitialized pointer.
    Now always works on `unit->plan`.
  - `kernel/task.cc` compare-fn: the `control_sides_defined()` branch computed
    `cs0/cs1` but tested `ps0/ps1` (people sides, uninitialized here) — fixed to
    `cs0/cs1`.
  - `kernel/write.cc` `write_game`: the advance-type loop never captured `name`,
    so its redundancy check compared against a stale/uninitialized value — now
    captures `name` like the terrain loop.
  - `kernel/imf.cc` `add_hex_mask`/`remove_hex_mask`: `numbytes` was used in
    `xmalloc`/`interp_bytes` without ever being computed (garbage-sized alloc);
    now `numbytes = img->h * computed_rowbytes(img->w, 1)` like every other site.
  - `kernel/mkroads.cc`: road endpoints from `find_adj_inside_area` were used
    even when it failed; now the road is laid only when both endpoints resolve.
  - `kernel/ui.cc` `unit_could_attack`: combat-model-0 fell through to the
    unconditional `return TRUE`, making its attack-possibility checks dead; added
    the missing `break` (UI attack-cursor hint only). *Behavior change.*
  - `kernel/world.cc` terrain-change loop: `hevttype` leaked across stacked
    units, so a unit that neither wrecked nor vanished could inherit the prior
    unit's event; now reset to `H_UNDEFINED` each iteration.
  - `kernel/nlang.cc`, `kernel/history.cc`: four `sprintf(buf, runtime_string)`
    calls (format-security) now use an explicit `"%s"`.

- ~~**[S] Remove bundled compat shims that modern platforms make dead code:**~~
  *(done 7/2026)* All three were unconditionally compiled but effectively dead
  on any platform this project actually builds for:
  - `kernel/snprintf.c/.h` deleted. Its header unconditionally forced
    `HAVE_SNPRINTF`/`PREFER_PORTABLE_SNPRINTF` regardless of platform (a
    long-standing "XCONQ HACK" per its own comment), so the portable fallback
    was always compiled and always shadowed libc's `snprintf`/`vsnprintf` via
    macro — never actually conditional. `asprintf`/`vasprintf`/`asnprintf`/
    `vasnprintf` were declared but `NEED_*` was never defined anywhere, so
    those bodies never even compiled; removing the header drops the unused
    declarations too. Callers now get libc's `snprintf`/`vsnprintf` directly
    via `<stdio.h>`.
  - `kernel/timestuff.c/.h` deleted along with the `NEED_STRUCT_TIMEVAL/`
    `TIMEZONE/GETTIMEOFDAY` CMake checks and `acdefs.h.in` defines that fed
    it. `NEED_GETTIMEOFDAY` only ever fired for `_MSC_VER`, and the checked-in
    fallback body was a `clock()`-based approximation (CPU time, not wall
    time — already wrong semantics for `gettimeofday`). The only in-tree
    consumer of that fallback is `kernel/win32.c`, which per the Windows
    assessment above is already dead/unbuilt/untested — reviving Windows
    support is an explicit future maintainer decision and would need a
    correct `gettimeofday` shim of its own regardless. `kernel/config.h` now
    includes `<sys/time.h>` directly in its `UNIX` block (glibc/BSD libc
    always provide `gettimeofday`/`struct timeval`).
  - `kernel/obstack.c/.h` (bundled GNU obstack) deleted. Its only consumer,
    `kernel/help.cc`'s `TextBuffer`, used it as a plain growable append
    buffer (`obstack_begin` → repeated `obstack_grow` via `tbcat`/`tbcat_si`
    → `obstack_finish`/`obstack_free`) — a clean fit for `std::string` now
    that the file is compiled as C++. Replaced the `struct obstack ostack`
    field with `std::string content`; `tbcat`/`tbcat_si` now call
    `.append()`, and `get_help_text` reads `.c_str()` instead of
    `obstack_finish`. Verified by running the full in-game help system
    (`?a` at the skelconq prompt, which walks every help node including the
    long copyright/warranty text) end-to-end with clean output.
  - Also removed `doc/INSTALL-snprintf`, a leftover install guide for the
    vendored snprintf package from the original CVS import. Left
    `doc/INSTALL-win.txt` alone — its two mentions of `timestuff.c`/
    `snprintf.c` are inside an already wholesale-obsolete 2004 Tcl/Tk-on-Windows
    build guide (references the removed `tcltk/`/`missing/` trees and
    `Makefile.in`), not worth partially patching.
  - Also deleted (7/2026): `lib/snprintf/`, a second, entirely unreferenced
    copy of the same vendored snprintf package (present since the initial CVS
    import, never wired into any build — old autoconf or current CMake). It
    was a self-contained autoconf project of its own (23 tracked files: its
    own `configure`/`Makefile.in`/`aclocal.m4`, `snprintf.c`, `test.c`), dead
    weight in a different location; `git rm -r`'d wholesale.

## 2. Known bugs (pre-existing, found while migrating)

- ~~**[M] `check-auto` assertion crash**~~ *(fixed 7/2026)*: the autotest
  called `do_toolup_action` on an incomplete, unplaced unit; the test setup
  now creates a complete on-map actor. Re-enabled in CTest.
- ~~**[M] `check-ai` infinite loop**~~ *(fixed 7/2026)*: not an ACP loop —
  `cil-rules.g` (a rules fragment with no units) never passes
  `test_for_game_start`, and skelconq's free-run loop spun forever waiting
  for a turn to advance. skelconq now cancels a `run` when the game has not
  started. Re-enabled; each game run is also bounded by a 10-minute timeout
  and a 100 MB output cap (`ulimit -f`), so this class of hang can no longer
  eat the disk.
- ~~**[S] Audit other library games for the same loop class**~~ *(done
  7/2026)*: full `check-ai` sweeps over all 174 modules run to completion
  under the per-game bounds (10 min, 100 MB output). Findings for the
  future AI work: `battles.g` and `conquest.g` drive the AI into runaway
  attack/capture-retry loops (contained by the output cap); a dozen big
  games (galaxy, advances, fantasy, insects, 3rd-age, spec, several
  ww2-*) exceed 10 minutes of AI self-play and fail on the per-game
  timeout; `classic.g` produces "unit occupying cell (x,y), was at
  (-x y)" consistency warnings. `check-ai` is enabled but carries the
  `long` CTest label, since an honest AI sweep does not fit a quick CI
  lane.

## 3. Testing & CI

- ~~**[M] Make the test scripts actually fail.**~~ *(done 7/2026)*: shared
  policy in `test/common.sh` — crashes fail everywhere; playable games
  (`lib/game.dir`) also fail on diagnostics and save/restore mismatches;
  runs are seeded, time- and output-bounded, and hermetic. The csh scripts
  (`test-run`, `test-long`) were converted to sh. The first honest run
  exposed eleven crashing library games (all fixed).
> Execution prompts for the remaining §3 items follow the same convention
> as §1/§5–§9 (fenced block, recommended model), numbered 3.x. They are
> mostly independent; prerequisites are stated per prompt.

- ~~**[S] Split `check-lib`/`check-actions` into one CTest per game module**~~
  *(done 7/2026)*: `check-save` and `check-test` got the same split, since the
  mechanism (a module argument on the scripts, reusing `test/common.sh`
  unchanged) dropped out for free. `test/CMakeLists.txt` globs `lib/*.g` and
  `test/*.g` at configure time and adds one `check-<suite>-<module>` test per
  file, each with its own scratch working directory under `build/test/scratch/`
  so `XCONQHOME`/saves/logs can't collide under `ctest -j`; skelconq's path is
  passed as an absolute `$SKELCONQ` since these tests don't run from
  `build/test/`. `ctest -R check-lib` still matches all of them. Verified:
  `ctest --label-exclude long -j$(nproc)` green (555 tests) twice, matching a
  serial run; a deliberately broken module failed only its own three tests.

- ~~**[M] Add a CI matrix:** GCC + Clang, Debug + Release, and a macOS job
  (the kernel and SDL/curses UIs should be close to working there).~~
  *(done 7/2026)*: Ubuntu is now a 4-leg matrix (gcc/clang x Debug/Release,
  via `CC`/`CXX` and `-DCMAKE_BUILD_TYPE`) and all 4 legs are green,
  confirmed in an actual CI run. Along the way this surfaced and fixed
  three real, previously-latent bugs: the SDL3-from-source build (added
  with the SDL2→3 port) had been silently broken on every CI run since,
  missing `libxtst-dev` (SDL's own CMake needs XTest to configure) —
  reproduced and fixed in a plain `ubuntu:24.04` container; two switch
  statements had comment-only `/* fall through */` markers that only
  GCC's `-Wimplicit-fallthrough` honors (Clang requires the
  `[[fallthrough]];` attribute — `kernel/nlang.cc`, matching the
  precedent in `read.cc`); and Release's optimizer surfaced a genuine
  `-Wmaybe-uninitialized` in `curses/ccmd.cc` that Debug doesn't reach.
  Also fixed two actual missing `break;`s found while auditing every
  `default: break;` in the tree for the same fallthrough class
  (`kernel/help.cc`, `sdl/sdlscreen.cc`; harmless in both cases since the
  `default` did nothing but break anyway, but not annotated as
  intentional).
  The macOS job builds the kernel, `skelconq`, and `cconq` — but as of
  this writing its `cmake -B build` configure step fails for an
  undiagnosed reason; two fix attempts (plain configure, then hinting
  `-DCMAKE_PREFIX_PATH=$(brew --prefix)` for `find_package(SDL3)`)
  didn't resolve it, and the actual error text couldn't be retrieved in
  this session (GitHub's logs API rejects anonymous/unauthenticated
  requests even for public repos, and no macOS machine was available to
  reproduce locally). `sdlconq` is expected to be skipped there regardless
  — it links Xlib/Xmu/Xext directly (not just SDL3, see
  `sdl/CMakeLists.txt`), which needs XQuartz. The job is kept
  `continue-on-error: true`; a future session with either an authenticated
  `gh`/API token or a real Mac should pull the actual configure error and
  finish this leg.
- ~~**[M] Add ASan/UBSan CI jobs** running the quick test suite.~~ *(done
  7/2026)*: added a `-DXCONQ_SANITIZE=address,undefined` CMake toggle (wires
  `-fsanitize=…` + `-fno-omit-frame-pointer -g` into compile *and* link of
  every kernel/UI target via `xconq_common`) and a GCC RelWithDebInfo CI leg
  that builds the kernel + skelconq and runs `ctest --label-exclude long` under
  `ASAN_OPTIONS=detect_leaks=0:abort_on_error=1` and
  `UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1`. Leak checking is
  deferred (`detect_leaks=0`): the kernel allocates via `xmalloc` and frees
  almost nothing by design, so LSan would drown the signal until §9
  de-globalization gives it a teardown path. Sanitized runs are ~2–5× slower,
  so a `XCONQ_TEST_TIMEOUT_SCALE` CMake knob (×3 on this leg) widens the CTest
  `TIMEOUT` properties and — passed through as `XCONQ_TIMEOUT_SCALE` in the
  test environment — `test/common.sh`'s per-game bound, for this leg only. **No
  suppressions were needed**; every finding was a real bug, all fixed (the
  quick lane, including `check-save`, is green in both the sanitized and normal
  builds, so no save/restore changed):

  ASan (memory safety):
  - `write.cc` `find_name`: `spare_file_name[PATH_SIZE] = '\0'` wrote one byte
    past a `PATH_SIZE` buffer — an off-by-one heap overflow on the save path,
    so it fired for essentially every module. The BIGBUF-class relative step 1
    predicted. Copy at most `PATH_SIZE-1` and terminate at the last valid index.
  - `side.cc` `create_side`: the per-side `trusts`/`trades` arrays were sized
    `g_sides_max()+1` at creation, but a later module can raise `sides-max`
    (clamped to `MAXSIDES`) after early sides exist, leaving a side-id index
    past the array — heap overflow, the same undersized-side-array class as
    step 1. Sized to `MAXSIDES+1`, the convention `config.h` documents.
  - `help.cc`: the cached `u_histogram` scratch buffer is shared by ~8
    table-description helpers that each walk a different type dimension, but
    every site sized it by `numutypes`; a game with more terrain (or advance)
    types overran it. Sized by the largest dimension via a `u_histogram_size()`
    helper.
  - `supply.cc` `init_supply_system`: a `realloc` meant to trim the `mstats`
    block discarded its return value and could move the block, dangling
    `first_mstat` and every `next`/`next_class` link into it — use-after-free
    in `process_supply_class`. Dropped the one-time trim (the kernel never frees
    it anyway).
  - `unit.cc`/`read.cc` `init_unit_opinions`: the opinions array is indexed by
    `side_number` (0..numsides), but two of the three callers sized it
    `numsides` (the third already passed `numsides+1`) — off-by-one when a unit
    with opinions is saved/restored. Fixed both callers.
  - `generic.cc`: a module defining types *after* the tables were allocated
    (tolerated with a warning) left every table indexed by that kind
    undersized — the accessor uses the new, larger stride/row-count on an
    old-sized block (hit via `tt_coat_max` in `spec`). Added
    `regrow_tables_for_index`, called from all four `disallow_more_*_types`, to
    reallocate and re-lay-out affected tables when a type kind grows late.

  UBSan (undefined behavior):
  - `util.cc` `isqrt`: `k <<= 1` with `k` a negative bit mask (-1, -2, -4, …) —
    left shift of a negative value, UB before C++20. Shift as unsigned and
    convert back; bit-identical result.
  - `economy.cc`: `border_at(…,dir,…)` does `1 << dir`, but `approx_dir` returns
    -1 when the from/to cells coincide → `1 << -1`. Skip the border
    contributions when `dir < 0` (there is no border to a cell from itself).
  - `misc.h` `normalize_on_pmscale` + `ai.cc` `basic_worth`: signed int overflow
    in the AI worth math — `n * 10000` in the pm-scale normalize, and the
    geometric `worth += worth*capture/150` term that explodes past `INT_MAX`
    (past 64 bits even, in `battles.g`) and used to wrap negative (the code
    already warned on the resulting "negative basic worth"). Widened the
    normalize intermediate to 64-bit; accumulate worth in 64-bit and saturate at
    `INT_MAX`. Behavior-relevant in principle (worth drives AI valuation) but
    only for the degenerate games that previously wrapped — normal games are
    unchanged and `check-save` is clean.
  - `unit.h`: `ParamBoxUnitSideSeers`/`ParamBoxUnitUnitSeers` carried a virtual
    destructor (making them polymorphic) yet are built with `xmalloc`, which
    never sets the vptr — every later member access/downcast tripped UBSan's
    vptr check. The destructors were no-ops; removed them so the boxes are
    non-polymorphic like their siblings.
  - `util.cc`: the `DEBUGGING` allocation-accounting counters (`overflow`,
    `totmalloc`, `grandtotmalloc`, `copymalloc`) are cumulative byte totals that
    overflow `int` on a long run; widened to `size_t` (unsigned, correct for a
    byte count, no UB).

  Also fixed two RelWithDebInfo-only `-Wmaybe-uninitialized` errors (that build
  type is new to CI): `economy.cc` `newtype` and `move.cc` `acpcost`, both
  false positives value-inited with a comment.
- ~~**[M] Fuzz the GDL reader** (`kernel/read.cc`, `kernel/lisp.cc`) with
  libFuzzer/AFL.~~ *(done 7/2026)*: added a libFuzzer harness
  (`test/fuzz/fuzz_gdl.cc`) driving `read_form_from_string()` — the lowest
  reader entry, the same tokenizer/parser that module loads, save restore, and
  the network transfer protocol all feed untrusted bytes through — over the
  whole input buffer one form at a time. It links a dedicated copy of the
  low-level reader objects with `-fsanitize=fuzzer,address,undefined` plus a
  stub file mirroring `imf2imf.cc`'s standalone-reader link closure; the target
  is opt-in (`-DXCONQ_FUZZ=ON`, Clang only, `test/fuzz/CMakeLists.txt`) and out
  of default builds. Interpreting parsed forms (`interp_form` in `read.cc`) is
  deliberately *not* fuzzed — it needs full world state and does file I/O
  (include/image/default-game loading), so a harness over it would surface
  init-order artifacts, not parser bugs; the untrusted-bytes-to-parser surface
  is `lisp.cc`. Corpus (`test/fuzz/corpus/`) seeds from the reader's own
  regression cases plus representative GDL; `gdl.dict` is ~1000 GDL tokens
  extracted from `kernel/*.def` by `gen_dict.sh`. A `fuzz` CI leg (Clang) builds
  the harness, replays the corpus (the `fuzz`-labelled CTest), and runs ~5 min
  of live fuzzing under ASan+UBSan; longer runs and OSS-Fuzz are noted as future
  options in `test/fuzz/README.md`. Two rounds of fuzzing (a shallow-bug burst,
  then a ~5 CPU-hour post-fix campaign that found nothing new; coverage
  saturated at ~325 edges) surfaced **five memory-safety / UB bugs, all fixed in
  `kernel/lisp.cc`**, every one reachable from a truncated or hostile module /
  save / network message and every one a real BIGBUF-relative the prompt
  predicted:
  - **Infinite allocation loop (DoS)** on an unclosed list ending in a symbol
    (`(foo`): `strmgetc` returns EOF at a string's terminating NUL *without*
    consuming, but `strmungetc` decremented the position unconditionally, so
    pushing that EOF back re-exposed the previous char and `read_list` re-read
    it forever, consing without bound. Fixed by making `strmungetc(EOF, …)` a
    no-op, as in stdio's `ungetc`.
  - **Signed overflow accumulating a long digit run** (`num*10 + digit`) — UB on
    `999999999999`; saturates at `INT_MAX` now, with `(ch - '0')` parenthesized
    so operator precedence can't overflow the intermediate near `INT_MAX`.
  - **Signed overflow in the decimal `factor * num` step** — large value times
    the ×100 decimal factor; saturates now.
  - **Left shift of a negative value in dice bit-packing** (`(dice - 2) << 7`,
    `(numdice - 1) << 11`) — UB for out-of-range dice like `1d`/`1d1` (already
    warned about, then computed anyway); packs the fields as unsigned now,
    bit-identical for valid dice.
  - **Signed overflow accumulating a long octal escape** (`8*octch + digit`) —
    UB on `"\04444444444"`; only the low byte is kept, so it accumulates as
    unsigned (defined wrap, same result) now.
  All fixes are behavior-preserving for valid input: the full quick ctest lane
  (555 tests, including `check-save`'s save/restore fidelity comparison) stays
  green in the normal build.
- ~~**[S] Port the `test/*-diff.sh` / `*-uses.sh` consistency checks into
  CTest** (docs ↔ `.def` symbols ↔ library usage) so they can't silently
  rot.~~ *(done 2026-07-05)*: converted `game-uses.sh`/`imf-uses.sh` from
  csh-flavored `#!/bin/bash` to `#!/bin/sh` and gave `cmd-diff.sh`,
  `game-diff.sh`, and `sym-diff.sh` real exit-code discipline (nonzero only
  on *unwaived* differences), registered as `check-consistency-{cmd,game,sym}`
  (label `consistency`, `TIMEOUT 60`, own scratch dir per test). Along the
  way fixed the actual drift each surfaced: a stale `^D detach` doc entry
  for a command that no longer exists, a dangling `(include "libimf")`
  debug leftover and a dead `"battle-test"` `game.dir` entry, six `PlanType`
  doc entries with the wrong case (`defensive` vs. source's `Defensive`,
  etc.) plus three genuine keyword renames (`hit-position`→`hit-pos`,
  `detonate-on-attack`→`detonate-with-attack`,
  `unit-consumption-per-cp`→`consumption-per-cp`), and half a dozen doc
  entries describing features removed from the kernel entirely
  (`autobuild`/`autoplan`, the `colonizing`/`random` plan types, three empty
  `GoalType` stubs, `indepside-can-build`, `units-may-go-into-reserve`).
  Remaining known-legitimate gaps (two unfinished-content module
  references in `game-diff.sh`, ~50 undocumented recent keywords plus two
  intentionally-retained "no longer available" table entries in
  `sym-diff.sh`) are waived by name, `KNOWN_UNFAITHFUL`-style. Two scripts
  are left unwired, each with a dated comment explaining why: `imf-diff.sh`
  (~1750/4500 symbols differ — mostly bulk-imported sprite packs and an
  X11 color palette that are legitimately unused by most games, not drift)
  and `syntax-diff.sh` (its `@example`→`@smallexample` Texinfo-migration
  sed bug was fixed, but the now-working comparison finds ~530/1700
  symbols missing from the syntax chart, too large to waive item-by-item).
  The four `*-uses.sh` scripts are usage-count reports with no pass/fail
  signal and stay manual-only.
- ~~**[S] Add unit tests for pure-kernel logic** (GDL read/write round-trip,
  save/restore equality, movement/combat table lookups) — the current tests are
  end-to-end only.~~ *(done 2026-07-05)*: added `test/unit/` (`unittests`
  binary, CTest `unittests`, label `unit`, `TIMEOUT 60`), linking the kernel
  directly rather than driving a whole game through skelconq. Factored
  skelconq's required-interface callback stubs (the ones every UI must
  supply per conq.h) out into `kernel/skelconq_stubs.cc`/`.h`, shared by
  both `skelconq` and `unittests`; skelconq itself keeps only its
  interactive command loop, unchanged in behavior (a `tolerate_warnings`
  flag added to the shared stub lets `unittests` inspect a deliberately
  provoked reader warning instead of dying the way skelconq always has).
  Covers, in one process/session: the Lisp reader/writer
  (`read_form_from_string`/`fprintlisp` round-trips for integers, escaped
  and octal-escaped and empty strings, plain and `|quoted|` symbols, nested
  lists, dice notation, and the lack of dotted-pair support; malformed
  input — unterminated list, unterminated string, and a token at the
  BIGBUF-1000 boundary — via the two real bugs the fuzzer found, without
  crashing or hanging); the `.def` type/table/gvar machinery
  (`interp_form(NULL, ...)` on inline `unit-type`/`terrain-type`/`table`/
  `set` forms, checking a u-by-t table's default and both lookup
  orientations, and a gvar default/override); and the dice1 roll min/mean/
  max/roll-range helpers plus the `normalize_on_pmscale` macro. Save/restore
  equality stayed out of scope (already covered end-to-end by `check-save`).

## 4. User interfaces

- ~~**[L] Port the Tcl/Tk UI (the primary one) to Tcl/Tk 9.**~~ *(resolved by
  removal, 7/2026)*: rather than porting, the Tcl/Tk UI (`tcltk/`, `xconq` +
  `imfapp`) was deleted outright — see Step 2. It only built against Tcl/Tk
  8.x and was already skipped on Tcl-9-only distros (Fedora); porting ~57
  `Tk_Offset`/`Tk_ConfigureWidget` call sites was judged not worth it with
  the SDL UI available as the graphical successor.
- ~~**[M] Port the SDL UI from SDL 1.2 to SDL2 or SDL3.**~~ *(done, 7/2026)*:
  ported to native SDL2 first (window-surface API: `SDL_CreateWindow` +
  `SDL_GetWindowSurface`/`SDL_UpdateWindowSurfaceRects`, replacing
  `SDL_SetVideoMode`/`SDL_Flip`; `SDL_WINDOWEVENT`/`SDL_TEXTINPUT` replace
  `SDL_VIDEORESIZE`/`SDL_ACTIVEEVENT` and the removed per-key unicode field),
  then straight on to SDL3 the same month: the window-surface model carried
  over unchanged, with the API churn confined to renames
  (`SDL_FillRect`→`SDL_FillSurfaceRect`, `SDL_FreeSurface`→`SDL_DestroySurface`,
  `SDL_MapRGB(surf->format,…)`→`SDL_MapSurfaceRGB(surf,…)`,
  `SDL_CreateRGBSurface`→`SDL_CreateSurface` with an `SDL_PixelFormat` enum),
  the `SDL_WINDOWEVENT` compound event splitting into individual
  `SDL_EVENT_WINDOW_*` types, and `evt->key.keysym.sym`→`evt->key.key`.
  `sdlconq` now links `libSDL3` directly; CI builds SDL3 from source (no
  `libsdl3-dev` package on `ubuntu-latest` yet — see the workflow file) since
  no shim exists at that vintage to fall back through.
- ~~**[S] Decide the fate of the Xt/Xaw UI (`xtconq`).**~~ *(resolved by
  removal, 7/2026)*: deleted outright rather than kept deprecated — see
  Step 2. This also removed its `x2imf`/`imf2x`/`xshowimf` image tools and
  the bundled 1990 K&R `SelFile` widget.
- ~~**[S] Vendor-audit the bundled Tcl `BWidget` toolkit copy**~~ *(moot,
  7/2026)*: `tcltk/` (and its vendored `BWidget` copy) was deleted along
  with the Tcl/Tk UI in Step 2.
- **[S] Ship an AppStream metainfo file** for `sdlconq` so desktop stores
  list the game properly. (The old `xconq.desktop`/`xconq.png`, which
  launched the now-removed `xconq` binary, was deleted in Step 2; a fresh
  desktop entry pointing at `sdlconq` is part of this item.)
- ~~**[S] Decide the fate of the curses UI (`cconq`).**~~ *(resolved by
  removal, 7/2026)*: deleted outright — see Step 3. `sdlconq` is now the
  sole UI, so the §5.1 PDCurses-parity question this used to motivate is
  moot too.

## 5. Platform behavior

> Execution prompts for §5–§8 follow the same convention as §1 (fenced
> block, recommended model). Unlike §1 these are mostly **independent** of
> each other — prerequisites are stated per prompt. File extensions are
> given as `.c`; after §1's rename task they may be `.cc` — adapt.

- ~~**[M] Use XDG base directories.**~~ *(done 7/2026)* Per-user data (saves,
  checkpoints, scores) now lives under `$XDG_DATA_HOME/xconq` (default
  `~/.local/share/xconq`); preferences under `$XDG_CONFIG_HOME/xconq`
  (default `~/.config/xconq`), both created 0700 on demand via a shared
  `xdg_xconq_dir`/`mkdir_p` helper in `kernel/unix.cc`. `$XCONQHOME`, when
  set, still overrides both exactly as before (test/common.sh's hermeticity
  is unaffected). If the new dirs are absent but `~/.xconq` still exists, its
  contents are read as a fallback (saved-game lookup, preferences, scores),
  with a one-time stderr note; nothing is ever written back there. Scores
  default to a per-user `scores` subdir of the data dir; `XCONQ_SCORES`
  (env, unchanged) and the compiled-in `XCONQ_SCORES_DIR` (now empty by
  default instead of `/var/lib/xconq/scores`) remain as overrides for shared
  installs. The setgid-era `games_uid`/`setuid` dance around scorefile
  writes (never wired up by the CMake install rules anyway) was removed from
  `kernel/unix.cc` and `sdl/sdlunix.cc`.
- ~~**[M] Decide on Windows/macOS support.**~~ *(Classic Mac half done
  7/2026, Windows half left open)* The tree carried Classic Mac OS
  (`kernel/mac.c`, `sdl/sdlmac.cc`) and Win32 (`kernel/win32.c`, RC files)
  code that hadn't compiled in 20 years. Classic Mac support is now deleted
  outright (see the dated note below); the Windows question is deliberately
  left to the maintainer. (The Tcl/Tk-side Classic Mac/Win32 files this
  item used to name, `tcltk/tkmac.c`/`tkxmac.c`/`iappmac.c`/`tkwin32.c`/
  `iappwin32.c`, were already removed with the rest of `tcltk/` in Step 2.)

**Windows assessment (7/2026).** Classic Mac support (`kernel/mac.c`,
`sdl/sdlmac.cc`, and all `#ifdef MAC`/`MACOSX`/`__MWERKS__`-as-Mac blocks
across `kernel/`, `curses/`, `sdl/`) is deleted. The Win32 files were left
in place and inventoried:

- `kernel/win32.c` (447 lines): a Win32/Cygwin counterpart to `unix.c` —
  `game_homedir`/`game_filename`/path handling, `open_file`/library-path
  search, signal handlers, `gettimeofday`-based timers. Targets Cygwin
  (`__CYGWIN32__`), MSVC (`_MSC_VER`), and CodeWarrior-for-Windows
  (`__MWERKS__`, for `CreateDirectory`) — i.e. 1999-2003-era Win32, no
  modern MinGW-only path. It **is** already wired into
  `kernel/CMakeLists.txt:63-67` (`if(WIN32) ... win32.c ... else() ...
  unix.c`), but untested for at least 20 years — no CI leg, no MinGW/MSVC
  box has built it since the CMake migration or before.
- `sdl/Xconq.RC`: a trivial 2-line `.rc` script associating `Xconq.ico` as
  the executable's icon. Not referenced by any CMakeLists.txt — dead
  weight, but harmless and tiny. (`curses/Xconq.RC`/`Xcdoc.ico` no longer
  exist — `curses/` was deleted in Step 3, 7/2026.)
- Not previously inventoried, but decision-relevant: `sdl/CMakeLists.txt`
  unconditionally links `X11::Xext`/`X11::Xmu`/`X11::X11` and always
  builds `sdlunix.cc`, whose `main()` (sdlconq's actual entry point) calls
  `setuid`/`geteuid`/X11 geometry parsing directly — i.e. the SDL UI's
  platform glue is Unix/X11-coupled, not just "missing a WIN32 branch."
  There is no `sdlwin32.cc` or equivalent, and `sdlconq` is now the sole UI
  (curses, which had no such X11/setuid coupling, was deleted in Step 3).

Recommendation: reviving Windows support is more than flipping the
existing `if(WIN32)` switch on. The plausible modern path is an SDL2/3
port of the SDL UI (already the top §4 UI priority, and now the only UI)
with a real `sdlwin32.cc`-equivalent main/platform file, `winsock2`/
`ws2_32` linkage for networking (`socket.c` already has partial `WIN32`/
`winsock2.h` handling, untested), and a MinGW or MSVC CI leg. (A parallel
PDCurses build for `curses` was a candidate cheaper first step, but it no
longer applies — `curses` was deleted in Step 3, 7/2026.) `kernel/win32.c`
contributes a plausible starting point for the kernel-level path/signal
glue (it's small and already CMake-wired) but is unverified and pre-dates
modern Win32/UTF-8 path conventions; everything UI-side would be new
work. This is a maintainer call, not made here — the Win32 files are left
in place either way pending that decision. *(Note 2026-07-11: the §10 web
client will become the practical no-install play story on Windows/macOS —
any browser against a hosted server — which lowers the stakes of the native
Win32 question considerably; revisit after §10 phase B.)*
- ~~**[S] Replace kernel/config.h's homegrown size detection**, which checks
  `SIZEOF_INT`/`SIZEOF_LONG` for its `Z16`/`Z32` typedefs — replace with
  `<cstdint>` fixed width types and delete the checks from `acdefs.h`.~~ (done 7/2026)

## 6. Security & robustness

- **[M] Unsafe string call audit:** the kernel had ~300 `sprintf` and ~70
  `strcpy` calls into fixed buffers (the bundled `snprintf.c` existed because
  the code predates C99). *(Kernel PRIORITY 1 done 2026-07-05; UIs remain.)*
  All kernel sites whose format/source carries attacker-influenced data (GDL
  type/unit/side names — capped at 999 chars by the reader but far larger than
  the BUFSIZE=255 display buffers — plus save/network data and player names)
  were converted to bounded equivalents across four committed slices:
  - **GDL reader/writer** (`lisp.cc`, `read.cc`, `module.cc`): the acknowledged
    `escaped_string`/`escaped_symbol` overflow (untrusted names into a fixed
    `escapedthingbuf`) now grows on demand; `sprintf_context` (raw reader input)
    and `read_warning`/`module_and_line` bounded (added `vtnprintf`).
  - **Network** (`tp.cc`, `socket.cc`): all ~45 outgoing-packet formats bounded;
    fixed a real 1-byte receive-path overflow (packet-reassembly guard used `>`
    against a buffer of exactly PACKETBUFSIZE); clamped the reverse-DNS hostname
    copy.
  - **Name formatting** (`nlang.cc`): ~485 sites (sprintf/strcpy/strcat and the
    `tprintf` append wrapper) bounded via BUFSIZE-contract wrappers + `sizeof`
    for local scratch; `name_or_number` given a size parameter.
  - **Remaining kernel** (`ui.cc`, `unit.cc`, `side.cc`, `history.cc`,
    `plan.cc`, `task.cc`, `ai.cc`, `cmd.cc`, `actions.cc`, `mknames.cc`,
    `score.cc`, `ps.cc`, `write.cc`, `help.cc`, `gif.cc`, `skelconq.cc`,
    `cmdline.cc`): bounded via shared `bounded_strcpy`/`bounded_strcat`
    (`util.cc`). Also fixed a real overflow in `help.cc`'s submodule-tree
    printer (per-nesting-level `strcat`/`strncat` with an unchecked, sometimes
    negative bound).

  Provably-safe sites were left with a why-class documented in the commit
  messages (exact-`xmalloc`'d path builders in `unix.cc`, self-bounded
  `make_pathname`, purely-numeric formats into ample buffers, string-literal
  copies, `sprintlisp`'s internal maxlen guards). Each slice left the tree
  green (`ctest --label-exclude long`, 559/559, including `check-save`
  fidelity — no truncation regressed a save).

  **Still open:** `sdl/` (the sole remaining UI as of Step 3, 7/2026) still
  has unbounded calls formatting the same already-bounded kernel handle
  strings into local buffers; it should get the same pass. (`curses/` — the
  bulk of the original ~130-call count — was deleted in Step 3, so its share
  of this item is moot.) The fuzzers from §3 cover the kernel reader, not UI
  formatting. *(Note 2026-07-11: the §10 SDL client port — tasks 10.14/10.15 —
  rewrites those display paths wholesale against `libclientmodel`, so don't
  invest in a standalone `sdl/` string pass unless the port stalls; fold the
  bounded-formatting discipline into the port instead.)*
- **[M] Review the homegrown networking layer** (`kernel/tp.cc`,
  `kernel/socket.cc`): it's a custom peer-to-peer protocol whose messages feed
  the GDL reader. At minimum, fuzz the message decoder; longer term consider
  whether multiplayer survives at all and behind what transport.
  *(Security review done 2026-07-05 — see `doc/net-security-review.md`.)* The
  hostile-peer audit fixed seven local memory-safety bugs in place, two of them
  Critical: a heap overflow de-escaping an oversized packet body into the
  255-byte `rsltbuf` (`tp.cc` receive path) and the undersized `send_packet`
  framing buffer it re-broadcast through; plus wire-controlled array indices
  (`rid`→`online[]`/`numremotes`, assignment/variant indices), a `receive_command`
  NULL-deref, and an unbounded accept `nextrid`. Deferred (documented, not
  patched): a peer can steer this host into opening arbitrary files (`f`/`g`/`w`
  handlers) and into the un-hardened GDL reader, and there is no auth/integrity.
  ~~**This item stays open** pending the §3-style *decoder* fuzzing~~ (the
  report's recommendation: fuzz-and-harden the decoder + contain the file-open
  surface now, and long term run multiplayer only over a trusted transport
  rather than re-securing the protocol). Build clean, quick ctest 559/559.
  *(Superseded 2026-07-11 by §10 / ARCHITECTURE.md: `tp.cc`/`socket.cc` are
  slated for outright removal — task 10.17 — so the planned decoder fuzzing is
  dropped as investment in code being deleted. The "what transport does
  multiplayer survive behind" question this item posed is now answered:
  server-authoritative WebSocket, kernel never parses peer input. Until 10.17
  lands, legacy multiplayer remains trusted-LAN-only per
  `doc/net-security-review.md`.)*
- ~~**[S] Review the custom image decoders** (`kernel/gif.cc`, `kernel/imf.cc`)~~
  — **done 2026-07-07.** Audited the homegrown GIF/LZW reader and the
  image-family interpreter (the X11 XBM/XPM readers this item used to name were
  removed with the Xt/Xaw UI in Step 2; SDL renders from the kernel's raw
  buffers rather than decoding files itself, so it inherits the kernel fixes).
  Found and fixed six bounds/overflow bugs, two critical:
  - `gif.cc:ReadImage` — `malloc(len*height)` with unchecked, unbounded
    image-descriptor dimensions: the `int` product overflows (so does the
    `ypos*len+xpos` write index), the classic decompression-bomb → heap
    overflow. Now validates dims (`>0`, `≤16384`, product `≤64M px`) and
    checks the allocation before use.
  - `imf.cc:make_raw_palette` — a GDL palette longer than 256 entries
    overflowed the `int ipal[4][256]` stack array. Now truncates at 256.
  - `imf.cc:copy_from_file_image` — `fimg->data[k]` indexed by GDL-supplied
    offsets/sizes with no check against the decoded buffer (heap over-read
    leaking adjacent memory into the rendered image). Now bounds-checked via a
    new `FileImage.datasize`; also guards non-positive target dims.
  - `gif.cc:LWZReadByte` — LZW prefix-chain walk could run the decode stack
    off its end on a crafted cyclic table (the self-reference check misses
    longer cycles). Now bounded against the stack end.
  - `gif.cc:GetCode` / `get_gif_from_file` — `buf[last_byte-2]` underflow read
    and use of uninitialized `rawdata` / over-read of the transparent-color
    probe on tiny images. Guarded.
  - `imf.cc:interp_image` — a hostile `numsubs` (from `x`/hexgrid counts) could
    overflow the `xmalloc(numsubs * sizeof(Image*))` size (`xmalloc` takes an
    `int`); now clamped to 4096.
  Verified: all UIs build clean; quick ctest 559/559; a standalone harness
  decodes every real library GIF correctly and rejects the crafted malicious
  ones gracefully, clean under ASan+UBSan.
  **Delegation recommendation:** GIF support is load-bearing — `images/` ships
  ~200 `.gif` files and many `lib/*.g` families reference them via `(file
  ...gif)`, so dropping the format is not viable. The right long-term move is
  to **replace the hand-rolled `gif.cc` with a maintained decoder** (giflib, or
  stb_image for a header-only zero-dep option) behind the existing
  `get_gif(FileImage*)` seam — the decoder is self-contained and has a single
  entry point, so the swap is localized. The `copy_from_file_image` bounds
  check should stay regardless, since it guards the GDL-supplied
  offset/size fields independent of which decoder produced the buffer. Until
  then the hardening above contains the known memory-safety bugs. A residual,
  lower-severity surface remains in the GDL-driven imf path (attacker-chosen
  `pixel-size`/dimensions still size large allocations and drive palette-index
  reads) — worth a follow-up pass with global clamping of GDL numeric image
  fields, but not memory-unsafe after the fixes above.
- ~~**[S] Audit file-path handling** in save/restore and module loading
  (`fopen` of names derived from GDL input or network messages — check for
  directory traversal out of the library dir).~~ *(done 2026-07-11)* Added one
  shared containment helper, `valid_untrusted_filename()` (`kernel/util.cc`,
  declared in `misc.h`): a name from GDL/save/network input is safe only if it
  is a *relative* path built from the allowlist `[A-Za-z0-9._/-]` with no
  leading slash, no backslash, and no `..` component — so it can never escape
  the dir it resolves against. Verified against the whole library first (every
  `include`/base name and all 1468 `imf.dir` FileName entries pass, so no
  legitimate layout — including subdir includes like `lib/foo` — is rejected).
  Site-by-site audit of every `open_file`/`open_module`/`open_library_file`
  caller:
  - **UNTRUSTED, now contained:**
    - `read.cc` `include_module` — `(include "name")` from GDL content →
      resolved by name against the library search path. Rejected traversal
      names are skipped with a `read_warning`.
    - `read.cc` `interp_game_module` — the `base-module`, `default-base-module`,
      `base-game`, and `filename` module properties (GDL/save content, each
      later re-resolved and loaded). Empty values still clear the property as
      before; only non-empty unsafe names warn and are ignored. (`filename` was
      already narrowed in step 1 to not clobber the real file, and the writer's
      `find_name` always records a bare basename, so no real save regresses.)
    - `tp.cc` network handlers `f` (saved-game filename), `g` (library module
      name), and `w` (coordinated-save filename → arbitrary *write*) — all from
      a peer; rejected with a `run_warning`, packet not acted on. (These live in
      code slated for removal by §10/10.17, but the fix is cheap and legacy
      multiplayer stays enabled until then.)
    - `score.cc` — the `scorefile-name` GDL gvar is joined onto the per-user
      scores dir on both the read and write paths; an unsafe value is ignored
      (`run_warning`) and the default `SCOREFILE` used instead.
    - `gif.cc` `get_gif` and `imf.cc` `get_generic_images`/`load_image_families`
      — image-family file names from GDL `(file ...)`/`imf.dir`, resolved
      against the images/library dirs; rejected names leave the family unloaded
      with an `init_warning`.
  - **TRUSTED, left unrestricted (the user's own explicit paths):** the
    command-line/UI game selection that creates `mainmodule`; `save_game`/
    `write_game_module` targets typed by the player; `statistics_filename`
    (`XCONQSTATSFILE` env or hardcoded default); `ps.cc` PostScript-dump target
    (UI print command); the `imf2imf` tool's output paths. Hardcoded names
    (`game.dir`, `imf.dir`, `Xconq.Warnings`, debug/trace files, `news`) are
    trusted by construction.
  Tests: `test/unit/unittests.cc` gains `test_untrusted_filename` (accepts real
  library names incl. subdirs and dotted names; rejects `..`/absolute/backslash/
  whitespace/metachar/empty/NULL); `test/error-traversal.g` drives five hostile
  `(include ...)` names through the real loader under `check-test`, asserting the
  containment warnings fire with no crash and no file opened. Full quick lane
  green (560/560, incl. all 174 `check-save` modules); both UIs build clean.

## 7. Code quality & developer experience

- **[S] Add `.clang-format` + `.editorconfig`** matching the existing GNU-ish
  style; format only files you touch (no big-bang reformat under active
  history).

**⚙ PROMPT 7.1 — recommended model: Sonnet.** *(Config-file authoring
against measurable existing style.)*

```text
Task: add a .clang-format and .editorconfig to Xconq that MATCH the
existing code style. Do not reformat any existing file.

Method:
1. Measure the dominant style empirically from kernel/ sources (they are
   the most consistent): indent width and tabs-vs-spaces (beware: this era
   of GNU code often mixes 4-space indent with 8-column tab compression —
   check actual bytes with cat -A or an od sample, not just appearance),
   brace placement for functions vs control flow (GNU-ish: function return
   type and name on separate lines, opening brace in column 0), line
   length, pointer-star binding, switch/case indent.
2. Write .clang-format starting from BasedOnStyle: GNU and override to
   match the measurements. Calibrate by round-tripping: clang-format a few
   representative kernel files to stdout and diff against the originals;
   iterate until the diff is mostly noise-level (perfect fidelity is not
   achievable — aim for "a formatted new function would look native").
   Record the residual known deviations as comments in the file.
3. Write .editorconfig consistent with it (charset, indent, trailing
   whitespace trim = false for existing files' sake, final newline) with
   sections for *.c/*.cc/*.h, *.g (GDL), CMakeLists.txt, *.sh.
4. Do NOT commit any reformatting of existing files, and do NOT add a CI
   format check (the tree doesn't conform; a check would fail everything).
   Add a short "Formatting" paragraph to CLAUDE.md (or CONTRIBUTING.md if
   it exists by now): format only lines you touch, e.g. via
   git-clang-format.
Verify: clang-format --dry-run on a couple of files runs clean (config
parses); nothing else to test.
Commit; mark this item done (strikethrough + date) in MODERNIZATION-PLAN.md.
```
- **[M] Stand up `clang-tidy`/`cppcheck` with a minimal profile** (bugprone-*,
  clang-analyzer-*) and fix incrementally.

**⚙ PROMPT 7.2 — recommended model: Opus.** *(The setup is mechanical, but
the value is in triaging analyzer findings — separating true 20-year-old
bugs from false positives without silencing real ones.)*

```text
Task: stand up clang-tidy for Xconq with a minimal high-signal profile,
triage the findings, and fix the clear true positives.

PREREQUISITE: the §1 -fpermissive removal must be committed (clang-tidy
uses Clang's frontend, which has no equivalent of -fpermissive). If SDL/X11
headers still trip Clang, scope this task to kernel/ and say so in the
plan note.
Method:
1. Configure with -DCMAKE_EXPORT_COMPILE_COMMANDS=ON. Note the LANGUAGE CXX
   arrangement: if the §1 .cc rename has NOT landed yet, .c entries in
   compile_commands.json compile as C++ — pass --extra-arg or verify
   clang-tidy honors the -x c++ flags in the database before trusting any
   results.
2. Add a .clang-tidy file enabling ONLY:
       bugprone-*, clang-analyzer-*
   minus checks that are noise on this codebase — expect to disable at
   least bugprone-narrowing-conversions and similar style-adjacent ones;
   list every disabled check with a one-line reason in the file itself.
3. Run over kernel/ first (run-clang-tidy with the build dir). Record the
   baseline count per check in a note. Then triage: for each finding class,
   sample several instances and decide true/false positive.
4. Fix TRUE positives in kernel/: null-deref paths, use-after-free,
   dead stores that hide logic bugs, suspicious sizeof/precedence,
   copy-paste index bugs. Rules: smallest fix that addresses the defect;
   anything with gameplay-behavior implications gets called out in the
   commit message. False-positive classes get the check disabled (with
   reason) rather than hundreds of NOLINTs; genuinely rare false positives
   get targeted NOLINT(check-name) with a short comment.
5. Wire a convenience target or script (e.g. misc/run-tidy.sh or a CMake
   custom target) documenting how to reproduce the run. CI integration is
   optional; if added, run it on kernel/ only and non-blocking initially.
6. cppcheck: run once (--enable=warning,portability), harvest anything
   real that clang-tidy missed, but do not build permanent infrastructure
   for it unless it found enough to justify it — note the outcome either
   way.
Verify after fixes: all UIs build, ctest --label-exclude long green.
Commit(s); update this plan item with a dated note: baseline count, fixed
count, remaining backlog, and whether cppcheck earned a permanent place.
```
- **[S] Wire up Doxygen.** The kernel headers already carry extensive `\file`/
  `\brief` comments; a `docs` CMake target + CI artifact makes them useful.

**⚙ PROMPT 7.3 — recommended model: Sonnet.** *(Standard tooling wiring
with a well-defined output.)*

```text
Task: wire up Doxygen for the Xconq kernel as a CMake target and CI
artifact.

Context: kernel headers already carry \file/\brief-style comments from a
past documentation effort (see kernel/*.h). The X-macro .def system will
confuse Doxygen wherever macros generate declarations.
Method:
1. Add a Doxyfile.in configured via CMake (@PROJECT_VERSION@ etc.):
   INPUT = kernel/ (headers and sources), RECURSIVE off beyond that,
   EXTRACT_ALL = YES (much of the code is uncommented; the extracted
   cross-reference is still valuable), GENERATE_LATEX = NO, HTML output
   under the build dir, WARN_IF_UNDOCUMENTED = NO (the backlog is huge;
   keep logs readable). Enable MACRO_EXPANSION with PREDEFINED tuned just
   enough that the .def-generated enums/tables don't produce garbage —
   check the output for the DEF_ macros and either expand or exclude them
   deliberately; note the choice in the Doxyfile.
2. Add an optional CMake target `docs` (find_package(Doxygen); absent
   Doxygen must not break configure — follow the existing pattern used
   for optional UI dependencies).
3. CI: extend .github/workflows/c-cpp.yml with a docs job (install
   doxygen, build the target, upload the HTML as an actions artifact).
   Do not fail CI on Doxygen warnings.
4. Fix only Doxygen syntax errors that produce broken output in existing
   comments (mismatched \param names etc.) if they are few; do not start
   a documentation-writing effort.
5. Skim the generated HTML for a few core headers (kernel.h, unit.h,
   conq.h) to confirm the output is actually navigable — that is the
   acceptance bar.
Verify: cmake --build build --target docs succeeds locally; CI job green.
Commit; mark this item done (strikethrough + date) in MODERNIZATION-PLAN.md.
```
- ~~**[S] Repo hygiene:** archive or delete `cvs_hist/`, `changelogs/`, the
  monolithic `ChangeLog`, and the stale packaging in `pkg/` (`xconq.spec.in`,
  NSIS script reference the deleted autoconf build). Add `CONTRIBUTING.md`
  documenting the CMake workflow and the `.def` X-macro system.~~ *(done
  7/2026)*: deleted `cvs_hist/`, `changelogs/`, `ChangeLog`, and all of
  `pkg/` (the spec/NSIS files were autoconf templates, `install` was the
  X11R5 script CMake's own `install()` replaces — nothing else referenced
  any of it); added `CONTRIBUTING.md` at the repo root.

- **[S] Add SPDX license headers** (GPL-2.0-or-later) to source files that
  carry only prose notices.

**⚙ PROMPT 7.5 — recommended model: Sonnet.** *(Mechanical sweep with one
strict rule: never guess a license — skip and report instead.)*

```text
Task: add SPDX license identifiers to Xconq source files, without ever
overriding what a file's existing notice actually says.

Rules (strict):
1. Only add "SPDX-License-Identifier: GPL-2.0-or-later" to files whose
   existing prose notice clearly says GPL v2-or-later (the standard Xconq
   header says the file "may be distributed under the terms of the GNU
   General Public License" or names v2+ explicitly — read a sample first
   and check COPYING is GPLv2).
2. KEEP the existing prose notice; add the SPDX line as the first comment
   line above it (comment style matching the file: /* */ for C/C++, # for
   shell/CMake/Tcl, ; for .g GDL files if licensing them is established
   practice — check whether lib/*.g carry notices at all; if they don't,
   leave the games library alone entirely and note that).
3. Files with a DIFFERENT or unclear license get NO SPDX line — collect
   them into a report instead. Known/expected cases to check rather than
   assume: kernel/obstack.c and obstack.h (GNU libc origin — LGPL era,
   if the file still exists — the §1 shim-removal task may have deleted
   it), kernel/snprintf.c (third-party, same caveat), anything in images/,
   bitmaps/, sounds/ (media provenance is murky — skip media entirely).
   (x11/SelFile/* and tcltk/BWidget/*, previously listed here as vendored
   third-party code with its own licensing, no longer exist — both dirs
   were removed in Step 2.)
4. Implementation: script the sweep (the scratchpad is fine for the
   script), but review the diff for oddballs — files with no notice at
   all get SPDX only if they are clearly original Xconq code (kernel/*.def
   etc.); when in doubt, skip and report.
5. Deliverable alongside the commit: a LICENSES-NOTES.md (or a section in
   CONTRIBUTING.md if it exists) listing files intentionally left without
   SPDX and why (third-party, unclear provenance, media).
Verify: build still green (headers added inside comments cannot break the
build, but the .g files ARE parsed — run ctest quick suite if any lib/
files were touched; ideally touch none).
Commit; mark this item done (strikethrough + date) in MODERNIZATION-PLAN.md.
```

## 8. Documentation & release management

- **[M] Modernize the manuals' delivery.** The Texinfo sources (user guide,
  designer's manual, hacking guide) are good content with no modern rendering.
  Build HTML in CI and publish via GitHub Pages; consider converting to
  Markdown/Sphinx only if the Texinfo toolchain becomes a burden.

**⚙ PROMPT 8.1 — recommended model: Sonnet.** *(Toolchain + CI wiring; the
content itself is not being edited.)*

```text
Task: build Xconq's Texinfo manuals as HTML in CI and publish them via
GitHub Pages. Do NOT convert formats or rewrite manual content.

Context: doc/ holds the Texinfo sources. Top-level documents vs included
fragments must be determined from the sources: xconq.texi (user guide),
xcdesign.texi (game designer's manual), hacking.texi (hacking guide) are
the likely roots; check @include graphs and any @setfilename to identify
what builds standalone (commands.texi, gdlref.texi etc. are probably
includes). The old build had doc rules — check git history of
doc/Makefile.in only if structure is unclear.
Method:
1. Fix what makeinfo --html reports as ERRORS in the sources (decades-old
   texinfo drift is likely); leave mere warnings unless output is visibly
   broken. Keep fixes markup-only.
2. CMake: optional docs-html target (find_program makeinfo; skip with a
   warning if absent — follow the existing optional-dependency pattern).
   One HTML directory per manual, built under the build dir.
3. Landing page: a small hand-written doc/index.html (copied into the
   published tree) linking the manuals, plus a pointer to the GitHub repo.
4. CI: a workflow job (extend .github/workflows/c-cpp.yml or add
   docs.yml) that installs texinfo, builds the manuals, and deploys to
   GitHub Pages via the standard actions/upload-pages-artifact +
   actions/deploy-pages pair, on pushes to master only. Note in the PR/
   commit message that Pages must be enabled in repo settings (Source:
   GitHub Actions) — that part is manual for the maintainer.
5. Add the published URL to README once known (githubusername.github.io/
   xconq — derive from the origin remote).
Verify: local docs-html build succeeds and the HTML is navigable
(spot-check internal links and the index page); CI workflow syntax passes
(actionlint if available, else careful review); normal build + quick ctest
untouched and green.
Commit; mark this item done (strikethrough + date) in MODERNIZATION-PLAN.md,
noting the manual Pages-settings step if the deploy has not run yet.
```
- **[S] Update the prose docs** (`README`, `doc/INSTALL-*`): dead URLs
  (sources.redhat.com, SourceForge mailing lists), references to Mac OS 9 /
  Windows ME, and the "7.5 Prerelease" framing from 2005.

**⚙ PROMPT 8.2 — recommended model: Sonnet.** *(Prose rewrite against
facts that are all checkable in-tree.)*

```text
Task: bring Xconq's prose docs (README, doc/INSTALL-*, NEWS framing) out
of 2005. Accuracy bar: every technical claim must match the CURRENT tree —
verify against CMakeLists.txt, CLAUDE.md, and the plan; do not carry
forward old text unchecked.

Method:
1. README: rewrite as the front door. What Xconq is (game system + GDL +
   games library — keep/adapt the existing good prose), current status
   (CMake build, one UI: native SDL3 (sdlconq, the sole and primary
   graphical client, ported from SDL 1.2 through SDL2 to SDL3, 7/2026 — see
   §4) — the Tcl/Tk and Xt/Xaw UIs were removed in Step 2, 7/2026, and the
   curses UI (cconq) was removed in Step 3, 7/2026; a minimal UI-section
   update already landed with each removal, but re-verify against the
   current tree rather than trusting it), quick build instructions
   (cmake -B build &&
   cmake --build build -j, dependency list matching the CI workflow's
   apt packages), how to run tests, pointer to MODERNIZATION-PLAN.md and
   CONTRIBUTING.md (if it exists), license note. Delete dead URLs
   (sources.redhat.com, SourceForge lists) — replace with the GitHub repo
   (derive from the origin remote); if no live community forum exists,
   say "use GitHub issues" rather than inventing links.
2. doc/INSTALL-*: read each variant (there are several per-platform ones).
   Fold anything still true into ONE current INSTALL (or a section of
   README) describing the CMake build with per-distro dependency lists
   (the Ubuntu list exists in .github/workflows/c-cpp.yml; doc/INSTALL
   already got a minimal Fedora dnf line as part of Step 2's removal —
   re-verify it rather than trusting it). The Tcl-9-vs-Tk-UI caveat this
   step used to note is moot: the Tcl/Tk UI no longer exists (Step 2,
   7/2026), so there is nothing left to block on Fedora. git rm the
   obsolete per-platform files (Mac OS 9, Windows ME era); mention they
   remain in git history.
3. Sweep remaining prose for the "7.5 Prerelease" framing and 2005-era
   status claims (grep -ri "prerelease\|7.5pre\|sourceforge\|redhat.com"
   across README*, NEWS, doc/*.txt, doc/*.texi top matter, man pages).
   NEWS: add a dated modern entry summarizing the 2026 modernization
   (CMake, test overhaul, fixes — source it from this plan's step 0/1
   intro) above the historical entries; do not rewrite history below.
4. Do not touch manual CONTENT (the .texi bodies) beyond top-matter
   status/URL corrections — the manuals task (8.1) owns their delivery.
Verify: builds/tests unaffected (docs-only change; run quick ctest anyway
if any file consumed by the build was touched — man-page templates are).
Commit; mark this item done (strikethrough + date) in MODERNIZATION-PLAN.md.
```
- **[S] Start tagging releases.** The version has been 7.5.0pre for 20 years.
  Both original gates are now clear: the Tcl/Tk UI was removed rather than
  ported (Step 2), and the SDL UI's SDL3 port landed 7/2026 (see §4). Cut a
  7.5.0 with release notes and a source tarball from CI (replaces the old
  `make distcheck`) — the actual tagging call is the maintainer's.

**⚙ PROMPT 8.3 — recommended model: Sonnet.** *(Release machinery only —
the decision to actually cut 7.5.0 stays with the maintainer.)*

```text
Task: build Xconq's release machinery so that pushing a git tag produces a
GitHub release with a source tarball and notes. Do NOT tag or cut the
release itself — both of the plan's original 7.5.0 gates are now clear
(the Tcl/Tk UI was removed in Step 2 rather than ported; the SDL UI's SDL3
port landed 7/2026, see §4), but the actual tagging call is the
maintainer's.

Method:
1. Version plumbing: find where the version is set (top-level
   CMakeLists.txt project() / PROJECT_VERSION feeding kernel/version.h.in;
   grep for "7.5" and "pre" variants across kernel/, doc/, man pages,
   xconq.desktop). Make the CMake project version the single source of
   truth; anything else must derive from it at configure time. Handle the
   "pre" suffix explicitly: keep the displayed version as a
   pre-release string for now (e.g. 7.5.0-pre via a separate suffix
   variable) so nothing claims to BE 7.5.0 before the tag.
2. Tarball: a CMake dist target or a small script producing
   xconq-<version>.tar.gz from git archive (respecting .gitattributes
   export-ignore, which you may need to create — exclude .github, and
   consider whether cvs_hist/changelogs still exist or were removed by the
   §7 hygiene task). Sanity-test: unpack the tarball somewhere clean,
   configure and build from it, run the quick ctest suite — this replaces
   the old make distcheck and is the acceptance bar.
3. CI: a release.yml workflow triggered on tags matching v* that builds
   on Ubuntu, runs the quick test suite, produces the tarball, and creates
   a GitHub release (softprops/action-gh-release or gh release create)
   with the tarball attached and body taken from a RELEASE-NOTES file or
   the tag annotation.
4. Write a doc/RELEASING.md checklist: bump version + clear the pre
   suffix, update NEWS, tag v7.5.0 annotated, push, verify CI release,
   restore the pre suffix for the next cycle.
Verify: the tarball round-trip build+test above; workflow reviewed
(actionlint if available). You can exercise the workflow with a throwaway
tag like v0.0.0-testrelease ONLY if you then delete the tag and the draft
release — otherwise leave a note that the first real tag will be the live
test.
Commit; update this plan item with a dated note that the machinery is
ready and the item stays open until the maintainer cuts 7.5.0.
```

## 9. Long-term / structural (optional)

- **[L] De-globalize the kernel.** World state, sides, and units are global;
  the engine is single-instance and non-reentrant. Wrapping state in a context
  struct would enable a test harness, engine-as-library embedding, and a
  headless server. *(Note 2026-07-11: §10's server deliberately does NOT
  depend on this — `xconqd` is process-per-game precisely because the kernel
  is global-state (ARCHITECTURE.md §3.1). This item stays optional; its
  remaining motivations are test-harness teardown/reload and eventual
  multi-game-per-process density, neither on the §10 critical path.)*

**⚙ PROMPT 9.1 — recommended model: Opus.** *(A weeks-scale structural
refactor; the phasing below makes each session mechanical, but classifying
what is game state vs config vs cache, and keeping saves/tests bit-identical
throughout, needs strong judgment. Do not attempt with Sonnet.)*

```text
Task: begin de-globalizing the Xconq kernel by gathering mutable game state
into a single context struct — WITHOUT changing function signatures or
behavior. This is a multi-session effort; the phases below each end in a
green, committable state. Stop at the end of any phase; record progress on
the plan item.

Context: kernel state lives in file-scope and extern globals declared
across kernel/conq.h (~375 externs incl. functions), kernel.h (~161),
kpublic.h (~96), plus statics in individual .c files. Note that GDL global
variables already go through generated accessors (kernel/gvar.def's
X-macros) — that pattern (access-through-macro, storage centralized) is
the model for this whole task.
Non-goals (explicitly out of scope): threading a context parameter through
call chains, multiple simultaneous instances, thread safety. The milestone
is "all game state reachable from one struct pointer"; reentrancy comes
later, in a separate effort, if ever.

Phase 0 — inventory (its own commit: a doc, no code change):
1. Enumerate every global VARIABLE (not function) in the three headers and
   every non-const file-scope static in kernel/*.c. Classify each:
   (a) per-game world/session state (units, sides, world layers, turn
   counters, history, random-number state); (b) per-process config (paths,
   debug flags, UI hooks); (c) derived caches rebuilt on load; (d) truly
   const tables. Write the inventory to doc/deglobalization.md with counts.
   Category (a) is the work; (b) and (d) stay global; decide (c)
   case-by-case and record the decision.
Phase 1..N — mechanical gathering, one subsystem per session/commit
(suggested order: world/area layers, then sides, then units, then history/
scoring, then RNG+turn state):
2. For each category-(a) global in the subsystem: move the storage into a
   struct GameState (new kernel/gamestate.h), replace the global with a
   #define or inline accessor of the SAME NAME resolving through a single
   `extern GameState *gg` (or similar) so no call site changes. Keep the
   .def-generated storage (gvar.def etc.) working by pointing its expansion
   at the struct — change the expanding header, never expanded copies.
3. After each subsystem: full build of both UIs and
   ctest --test-dir build --label-exclude long. The save/restore tests are
   the real safety net here — any state you misclassify or fail to reset
   shows up as a save mismatch. Also run one long-label AI game if the
   subsystem touched AI state.
Acceptance for the overall item: init_world/game teardown can free-and-
rebuild the struct, demonstrated by a new small C++ unit test or skelconq
scripted sequence that loads one game, tears down, loads a different game
in the same process without leaks/stale state (this is the concrete win
that motivates the refactor — today a second load relies on globals
happening to get re-inited).
Update doc/deglobalization.md and the plan item with a dated note each
session (subsystems done / remaining).
```
- ~~**[L] Consider a modern UI built on the SDL3 port** (or a web frontend
  driven by a headless kernel over the existing network protocol) as the
  long-term evolution of `sdlconq`.~~ *(resolved 2026-07-11)*: the decision
  this item asked for was made directly with the maintainer and recorded in
  **ARCHITECTURE.md** rather than via the design-study prompt that used to
  live here (removed; see git history). Outcome, in this item's own terms: a
  variant of its "option c" — a web client against a NEW protocol (JSON over
  WebSocket) served by a headless server built on the kernel + the
  skelconq null-interface pattern — with the twist that the protocol is
  **server-authoritative** (per-side view sync via the UI callback surface,
  real fog of war), the existing lockstep protocol is removed rather than
  reused (its shared-replicated-state assumption is exactly what the old
  option b analysis would have tripped over), and `sdlconq` is kept and
  ported to the same protocol as the native thin client (plus LAN listen-
  server host). The §10 work plan implements it.
- **[M] GDL ergonomics:** a syntax description for editors (tree-sitter
  grammar or TextMate bundle), plus a `skelconq --validate` mode for game
  authors, would lower the barrier for the games library — which is the
  project's real crown jewel (200+ modules).

**⚙ PROMPT 9.3 — recommended model: Sonnet.** *(Two well-bounded
deliverables: a validate flag that reuses existing kernel machinery, and a
grammar that can be tested mechanically against all 174 library modules.)*

```text
Task: GDL author ergonomics, two independent deliverables in two commits:
(A) a `skelconq --validate` mode, (B) a tree-sitter grammar for GDL.

Part A — skelconq --validate:
1. Anchors: kernel/skelconq.c (main at ~line 88; the interpreter loop and
   init sequence show how a game is loaded without playing); shared
   command-line parsing in kernel/cmdline.c (add the flag there if
   skelconq uses it — check — else in skelconq.c).
2. Behavior: `skelconq --validate <module>` loads and initializes the
   module (through the same path the tests use — GDL parse, type/table
   checks, patch_object_references etc.) WITHOUT starting a game or
   entering the interactive loop, then exits 0 if no errors/warnings were
   produced, nonzero otherwise, printing each diagnostic with the source
   file (and line if the reader tracks it — check what init_warning has
   access to; do not build new line-tracking infrastructure in this task,
   just report what exists).
3. Include-fragments legitimately warn when loaded standalone (see
   test/common.sh policy). Keep --validate honest for them: a
   --fragment flag (or documented convention) that treats
   missing-game-level warnings as OK would help authors; implement only
   if it falls out naturally from how the warnings are classified,
   otherwise document the limitation in the man page.
4. Wire a quick CTest: --validate on a known-good game.dir module exits 0;
   on a scratch .g with a deliberate error, exits nonzero. Update
   kernel/skelconq.6in (man page).
Part B — tree-sitter grammar:
5. Sources of truth for the syntax: doc/syntax.texi (643 lines — the
   written grammar) cross-checked against kernel/lisp.c's actual tokenizer
   (comments including #| |# blocks if supported — verify, ; line
   comments, strings with escapes, numbers incl. dice notation like 1d6
   and fractions, symbols, |quoted symbols| if supported — check lisp.c,
   don't trust the docs alone).
6. Keep it a LEXICAL/structural grammar: sexps, atoms, comments, plus
   highlighting queries (queries/highlights.scm) that recognize the
   top-level form keywords (define, set, add, table, include, game-module
   — harvest the list from kernel/keyword.def rather than hardcoding a
   guess). Do not attempt semantic knowledge of every property name.
7. Layout: new top-level tools/tree-sitter-gdl/ with the standard
   tree-sitter project shape (grammar.js, queries/, package.json), its
   own README saying how to build/use it in Neovim/Helix/Emacs, NOT wired
   into the main CMake build (it needs node/tree-sitter-cli; keep the
   game's build dependency-free).
8. Acceptance test: `tree-sitter parse` over ALL lib/*.g files (174) —
   zero ERROR nodes. Add a small script (tools/tree-sitter-gdl/test.sh)
   that does that sweep so it stays checkable.
Verify: normal build + quick ctest green (Part A touches skelconq; Part B
touches nothing in the build). Commit each part; mark this item done
(strikethrough + date) in MODERNIZATION-PLAN.md, noting where the grammar
lives.
```

## 10. Client/server rearchitecture

The major-version track adopted 2026-07-11: kernel becomes server-only
(`xconqd`), a new browser client and the ported SDL client are thin clients
over a new server-authoritative WebSocket protocol, and the legacy lockstep
protocol is removed. **ARCHITECTURE.md is the decision record and context
anchor** — every prompt below assumes it has been read, and it must be kept
truthful as work lands. `doc/net-protocol.md` (task 10.2) becomes the second
anchor once it exists.

Prompt conventions as in §1/§3–§9: each task carries a ready-to-run fenced
prompt written so a fresh session can execute it with no conversation
context (CLAUDE.md loads automatically; ARCHITECTURE.md and
doc/net-protocol.md carry the architecture context), labeled **Opus** for
tasks needing design judgment or kernel-semantics triage, **Sonnet** for
well-bounded mechanical work against a written spec.

**Standing rule for every §10 task** (also restated in each prompt): when a
task finishes, in this file mark the task done (strikethrough + a dated note
of what *actually* happened, including surprises), add any newly identified
work as new §10 tasks in this same prompt convention, and update
ARCHITECTURE.md in the same commit if the implementation deviated from it.

Phases: **A** (protocol + server core, 10.1–10.8) is largely sequential.
**B** (web client, 10.9–10.12) follows A and deliberately precedes the SDL
port — the thinnest client validates the protocol while it is still cheap to
change. **C** (SDL client port + legacy removal, 10.13–10.17) can overlap B.
**D** (production server, 10.18–10.22) follows A; nothing internet-facing
deploys before §6.4 (path traversal) is done.

### Phase A — protocol and server core

- **[10.1] [M] Client-state inventory.** Enumerate everything a client needs:
  the callback surface plus every kernel data structure `sdl/` reads and every
  kernel call it makes. This inventory *is* the protocol's state-sync schema;
  its completeness is the top risk of the whole rearchitecture
  (ARCHITECTURE.md §9). Analysis only.

**⚙ PROMPT 10.1 — recommended model: Opus.** *(Judgment about which kernel
reads are per-side view state vs process-local scratch determines the
protocol's shape; a missed class surfaces as a redesign later.)*

```text
Task: produce doc/client-state-inventory.md — the complete inventory of
kernel state and entry points a thin Xconq client needs. ANALYSIS ONLY: no
code changes. Read ARCHITECTURE.md first (§2, §3.6, §9).

Method:
1. Callback surface: list every required-interface callback from
   kernel/skelconq_stubs.cc and the "must be supplied by an interface"
   declarations in kernel/conq.h. For each: what state changed, what
   content a network message must carry (the callback args are dirty
   notifications; the UI then reads kernel memory — name WHICH memory).
2. Direct reads: sweep sdl/*.cc for kernel data access — side->/unit->/
   plan->/task-> field reads, view-layer accessors (terrain_view, unit_view
   chains, coverage), area./world globals, type-property and table
   accessors (u_*/t_*/m_*/g_* and table macros), history, scores,
   doctrine, agreements. Classify each: (a) reachable from an existing
   callback moment; (b) read-on-demand side state that must enter the
   snapshot + deltas; (c) static per-game data for the manifest (type
   rosters, display properties); (d) process-local (prefs, window state —
   not protocol).
3. Command entry points: every kernel call sdl/ makes that CHANGES game
   state — the net_* action wrappers in kernel/kpublic.h, task/plan
   setters, side settings, naming, end-turn/resign, design-mode commands.
   This list becomes the command plane.
4. Blocking/modal spots: any place the kernel expects a synchronous UI
   answer (search conq.h/kpublic.h for query/ask/choose-style interface
   functions). These need async request/response pairs — flag each.
5. Deliverable: doc/client-state-inventory.md with one table per class,
   counts, and a "surprises/risks" section. Cross-check: skim curses/ in
   git history (deleted Step 3) ONLY if sdl/ coverage of some feature is
   in doubt; sdl/ is the reference.
Verify: nothing to build; the doc must cite real symbols (spot-check a
sample compiles the claim, i.e. the symbol exists where stated).
Commit. In MODERNIZATION-PLAN.md §10: mark 10.1 done (strikethrough +
dated note incl. surprises), add newly identified work as new §10 tasks,
and update ARCHITECTURE.md if findings contradict it.
```

- **[10.2] [M] Protocol specification** (`doc/net-protocol.md`): message
  envelope, session/lobby, snapshot, deltas, commands, errors, versioning.

**⚙ PROMPT 10.2 — recommended model: Opus.** *(The contract every later task
codes against; wrong altitude here is expensive everywhere.)*

```text
Task: write doc/net-protocol.md, the wire-protocol spec for the Xconq
client/server architecture. Read ARCHITECTURE.md (§4, §5) and
doc/client-state-inventory.md (task 10.1) first. Spec only — no code.

Requirements:
1. Transport: one JSON object per WebSocket text message; a "t" field
   names the message type. Document the hello/welcome handshake with an
   integer protocol version and clean-rejection semantics.
2. Session plane: join (game id, requested side, optional token), rejoin
   (token -> rebind + fresh snapshot), side assignment, variant settings,
   chat, error responses (machine-readable code + human text).
3. State-sync plane: define the join snapshot and one delta message per
   netui callback moment, carrying content per the 10.1 inventory
   (class a + b). Layers (terrain view etc.) need a compact encoding —
   specify run-length-encoded strings (the save writer's layer RLE is the
   conceptual precedent; keep the JSON form self-describing). Unit views,
   side state, turn/clock, history events, notices.
4. Command plane: one message per 10.1 class-(c) entry point that a
   player actually uses (audit against kernel/cmd.def for player-facing
   coverage); each carries acting unit/side ids; server validation rules
   stated per command. Async request/response pairs for every 10.1
   class-4 modal spot.
5. Manifest: schema for the per-game static data (type rosters, display
   properties, image-family names, world geometry) per ARCHITECTURE.md
   §3.4.
6. Worked examples: a full join transcript (hello -> welcome -> snapshot
   -> a turn of deltas -> a move command -> resulting deltas) as literal
   JSON.
7. State the versioning policy (pre-1.0: version bump on any breaking
   change, no compat shims) and message-size limits.
Verify: every snapshot/delta field traces to a 10.1 inventory row and
vice versa (class a-c rows all covered or explicitly deferred with a
reason — deferred rows get a "v2" appendix table).
Commit. In MODERNIZATION-PLAN.md §10: mark 10.2 done (strikethrough +
dated note), add newly identified work as new §10 tasks, and update
ARCHITECTURE.md if the spec deviated from it.
```

- **[10.3] [M] WebSocket + HTTP server module** (`server/net/`): standalone,
  kernel-free, unit-tested.

**⚙ PROMPT 10.3 — recommended model: Sonnet.** *(Well-bounded protocol code
against RFC 6455, mechanically testable; hostile-input handling has clear
acceptance tests.)*

```text
Task: implement a minimal, dependency-free WebSocket + static-HTTP server
module for Xconq under server/net/, with unit tests. No kernel linkage.
Read ARCHITECTURE.md §4 first.

Method:
1. server/net/: C++17, poll()-based, non-blocking, single-threaded, no
   threads anywhere. Public API: a listener object the caller polls
   (fd-set integration for xconqd's event loop), per-connection send/
   receive of complete text messages, close with status codes.
2. HTTP/1.1: parse only what is needed — GET for static files (root dir
   configurable, strict path sanitization: reject "..", encoded dots,
   absolute paths; small mime map for .html/.js/.css/.png/.json/.wasm)
   and the ws Upgrade handshake (Sec-WebSocket-Accept = SHA-1 + base64;
   implement SHA-1 in-tree, ~100 lines, cite the FIPS reference in a
   comment). Hard limits: header size, request line length, connection
   count; slow/oversized inputs get a clean close.
3. RFC 6455 frames: text, ping/pong (auto-reply), close; server-to-client
   unmasked, client-to-server masking REQUIRED (reject unmasked);
   fragmented messages reassembled up to a configurable max message size
   (default 4 MB), oversized -> close 1009. Binary frames: accept and
   deliver (future CBOR) but nothing sends them yet.
4. Vendor nlohmann/json single header under server/vendor/ with a
   LICENSE note (MIT) — used by later tasks; this module itself stays
   JSON-agnostic (bytes in, bytes out).
5. Tests: test/net/ CTest suite (label "net", quick lane): a raw-socket
   test client exercising handshake, echo, ping, fragmentation,
   oversized-frame rejection, unmasked-frame rejection, bad-path static
   requests (traversal attempts must 404, never escape root), header
   bombs. Follow test/unit/'s hand-rolled CHECK style — no new test
   framework.
Verify: full build + ctest quick lane green including the new net label;
run the net tests under the sanitizer toggle (-DXCONQ_SANITIZE=
address,undefined) locally and note the result in the commit message.
Commit. In MODERNIZATION-PLAN.md §10: mark 10.3 done (strikethrough +
dated note), add newly identified work as new §10 tasks, and update
ARCHITECTURE.md if you deviated from it.
```

- **[10.4] [M] `xconqd` server skeleton**: event loop, sessions, join/side
  binding, chat — the process every later server task grows.

**⚙ PROMPT 10.4 — recommended model: Opus.** *(Marries the kernel's
init/run_game lifecycle to an event loop; wrong lifecycle decisions here
ripple through every later task.)*

```text
Task: create the xconqd game-server skeleton under server/. Read
ARCHITECTURE.md (§3.1) and doc/net-protocol.md first. Prereqs: 10.2, 10.3.

Method:
1. server/xconqd.cc + CMake target xconqd (always builds, like skelconq;
   links libconq + server/net + vendored json). Args: game module,
   --port, --http-root (web bundle/atlases; optional), --seed, variant
   flags as needed. Model the kernel init sequence on kernel/skelconq.cc
   (init_library_path, module load, side/player setup, run_game(0) start
   sequence — copy its order, don't reinvent). For the required-interface
   callbacks, start from a copy of the skelconq_stubs pattern in a new
   server/netui.cc (fleshed out by 10.5); leave kernel/skelconq_stubs.cc
   itself untouched — skelconq and unittests keep using it.
2. Event loop: single thread; poll ws listener + connections with a short
   timeout; between polls run bounded run_game(maxactions) slices exactly
   like skelconq's free-run mode; never block on network I/O. SIGTERM ->
   clean shutdown (close 1001 to clients, exit 0).
3. Sessions: implement hello/welcome (protocol version check), join ->
   bind connection to a side (respecting player assignments; reject taken
   sides), chat relay, disconnect -> side marked unattended (game keeps
   running; policy refinement is task 10.20). Malformed JSON or unknown
   "t" -> protocol error message, repeated offenses -> close.
4. Smoke test: a CTest (label "net") that starts xconqd on an ephemeral
   port with a small lib/game.dir module, performs hello/join/chat with
   the 10.3 test-client utilities, and asserts a clean shutdown on
   SIGTERM. Bound it (timeout, hermetic XCONQHOME) per test/common.sh
   conventions.
Verify: full build + quick ctest green (555+ existing tests must be
unaffected — xconqd must not disturb skelconq/unittests linkage).
Commit. In MODERNIZATION-PLAN.md §10: mark 10.4 done (strikethrough +
dated note), add newly identified work as new §10 tasks, and update
ARCHITECTURE.md if you deviated from it.
```

- **[10.5] [M] netui: callback→delta serialization** — the server half of the
  state-sync plane.

**⚙ PROMPT 10.5 — recommended model: Opus.** *(Requires reading kernel view
semantics correctly; a wrong view read leaks fogged state or desyncs
clients.)*

```text
Task: implement server/netui.cc — the per-side delta emitters behind the
required-interface callbacks in xconqd. Read ARCHITECTURE.md (§2, §3.2)
and doc/net-protocol.md (state-sync plane) first. Prereq: 10.4.

Method:
1. For each callback in the required surface (list: kernel/
   skelconq_stubs.cc), implement the xconqd version to build the
   corresponding delta message per the spec: read the SIDE'S VIEW at
   callback time (terrain view layers, unit views, side/turn/clock state
   — the exact reads are mapped in doc/client-state-inventory.md class
   a), never ground truth the side cannot see. update_cell_display's
   flags argument tells you which layers changed — honor it rather than
   resending everything.
2. Emit only to sides with a live connection; drop silently otherwise
   (unattended sides catch up via rejoin snapshot, task 10.6/10.18).
   Respect flush_display_buffers as the batching boundary: accumulate
   deltas and send on flush, so one kernel action doesn't produce dozens
   of tiny ws messages.
3. History events (update_event_display) map to first-class hist
   messages; low_notify/update_message_display to notice/chat-adjacent
   messages per spec.
4. Testing: extend the 10.4 smoke test: join a side, run several turns,
   assert (a) turn deltas arrive in order, (b) a cell delta's coordinates
   are within the world, (c) NO message ever carries data for a cell the
   side's coverage says it cannot see (pick a game with fog — check
   see-all is off) — that last assertion is the fog-integrity canary and
   the most important line in the test.
Verify: full build + quick ctest green, including net label.
Commit. In MODERNIZATION-PLAN.md §10: mark 10.5 done (strikethrough +
dated note), add newly identified work as new §10 tasks, and update
ARCHITECTURE.md if you deviated from it.
```

- **[10.6] [M] Join snapshot + game manifest.**

**⚙ PROMPT 10.6 — recommended model: Opus.** *(Serializing a side's complete
view without leaking ground truth; the save writer is the precedent but its
output is full-knowledge, not per-side.)*

```text
Task: implement the join/rejoin snapshot and the game manifest in xconqd.
Read ARCHITECTURE.md (§3.2, §3.4) and doc/net-protocol.md (snapshot +
manifest schemas) first. Prereqs: 10.4, 10.5.

Method:
1. Manifest (sent in welcome): generated from the loaded game's type
   tables — unit/material/terrain/advance rosters with names, display
   chars, image-family names, and the display-relevant per-type
   properties enumerated in doc/client-state-inventory.md class (c);
   plus side roster and world geometry (hex/rect, dimensions, wrap).
   Type-property access goes through the generated accessors (u_*/t_*/
   m_* — see the .def X-macro system in CLAUDE.md), never hand-copied
   tables.
2. Snapshot (sent on join/rejoin, after welcome, before deltas resume):
   the side's terrain view layers RLE-encoded per spec, all unit views
   the side holds, own units' full state incl. plans/tasks (inventory
   class b), side state (treasury, research, scores), turn/date/clock.
   Model the layer walk on kernel/write.cc's layer writers (write_area
   RLE machinery) but emit the spec's JSON form; view accessors, not
   ground-truth layers.
3. Ordering: queue deltas generated during snapshot construction and
   send them after it (single-threaded, so this is just buffering
   discipline in the netui, not locking).
4. Test: extend the net CTest: join, capture snapshot, run turns, then
   REJOIN with a second connection and assert the second snapshot equals
   the first client's accumulated model for overlapping fields (turn,
   unit-view count, a sampled RLE row). This is the state-sync fidelity
   check — the analogue of check-save for the wire.
Verify: full build + quick ctest green including net label.
Commit. In MODERNIZATION-PLAN.md §10: mark 10.6 done (strikethrough +
dated note), add newly identified work as new §10 tasks, and update
ARCHITECTURE.md if you deviated from it.
```

- **[10.7] [M] Command plane**: client intents → validated kernel actions.

**⚙ PROMPT 10.7 — recommended model: Opus.** *(Every handler is a trust
boundary; validation must lean on kernel legality checks, not reimplement
them wrongly.)*

```text
Task: implement the client->server command plane in xconqd. Read
ARCHITECTURE.md (§3.3, §6) and doc/net-protocol.md (command plane) first.
Prereqs: 10.5, 10.6.

Method:
1. Implement one handler per command message in the spec, mapping to the
   existing kernel entry points inventoried in doc/client-state-
   inventory.md class (c) — the net_* wrappers in kernel/kpublic.h and
   task/plan setters. NOTE: the net_* wrappers have a local-execution leg
   and a legacy tp.cc broadcast leg; call so the local leg runs (hosting
   is FALSE in xconqd's process — verify, don't assume). Task 10.17
   deletes the broadcast legs later; do not touch tp.cc now.
2. Validation order per command: session bound to a side -> unit exists
   and belongs to that side -> arguments in range (coords in area, types
   valid) -> hand to the kernel entry point and let ITS legality checks
   decide (execute_action already rejects invalid types/inactive actors
   — step 1 hardening, see plan intro). Rejections produce the spec's
   error response with a code; never a crash, never silence.
3. Cover: movement/build/fire/attack tasks, plan mode/standing orders,
   unit naming, end-turn/resign, side settings, the async answers for
   10.1's modal spots. Chat existed since 10.4.
4. Test: extend the net CTest: issue a legal move (assert resulting
   deltas), an out-of-area move (assert error code), a command for
   ANOTHER side's unit (assert rejection — the anti-cheat assertion),
   and end-turn round-trip.
Verify: full build + quick ctest green including net label.
Commit. In MODERNIZATION-PLAN.md §10: mark 10.7 done (strikethrough +
dated note), add newly identified work as new §10 tasks, and update
ARCHITECTURE.md if you deviated from it.
```

- **[10.8] [M] Headless protocol test client + CI wiring** — the protocol's
  reference consumer and regression harness.

**⚙ PROMPT 10.8 — recommended model: Sonnet.** *(Scripted client against a
written spec, with existing test-suite conventions to follow.)*

```text
Task: build a headless scripted protocol client and wire protocol
regression tests into CTest. Read ARCHITECTURE.md and doc/net-protocol.md
first. Prereqs: 10.4–10.7 (extend the tests they started rather than
duplicating).

Method:
1. test/net/protoclient.cc: a C++ CLI reusing the 10.3 test-client ws
   code: connects, joins, then executes a simple line-oriented script
   (from a file): expect <msg-type> [field=value...], send <json>,
   run-turns <n>, save-transcript <file>. Timeouts on every expect.
2. Transcript mode: --record <file> writes every received message as
   JSON-lines with a direction marker — task 10.13 replays these to
   unit-test libclientmodel, so the format is a deliverable, document it
   in a comment header and in doc/net-protocol.md's appendix.
3. Tests: consolidate the 10.4–10.7 assertions plus a longer scenario —
   join the default introductory game vs AI, play a scripted opening
   (move a starting unit, end several turns), assert turn progression
   and fog integrity. One CTest per scenario script (mirror how test/
   CMakeLists.txt registers per-module tests: own scratch dir, bounded,
   label "net", quick lane). Record one canonical transcript into
   test/net/transcripts/ and add a replay-freshness test that regenerates
   it and diffs structurally (message-type sequence, not full bytes —
   timestamps/ids may vary; document what is normalized).
4. CI: the net label already runs in the quick lane on every leg —
   verify the sanitizer leg passes it too (widened timeouts via
   XCONQ_TEST_TIMEOUT_SCALE apply; check test/common.sh conventions are
   respected for any per-game bound you add).
Verify: full build; ctest quick lane green twice under -j$(nproc)
(parallel-safety check); sanitizer-toggle build of the net tests locally.
Commit. In MODERNIZATION-PLAN.md §10: mark 10.8 done (strikethrough +
dated note), add newly identified work as new §10 tasks, and update
ARCHITECTURE.md if you deviated from it.
```

### Phase B — web client

- **[10.9] [M] Asset pipeline**: `.imf`/GIF image families → PNG sprite
  atlases + JSON index.

**⚙ PROMPT 10.9 — recommended model: Sonnet.** *(A standalone converter tool
with an existing link-closure precedent and mechanically checkable
output.)*

```text
Task: build tools/imf2atlas — converts Xconq image families to PNG sprite
atlases + a JSON index for the web and SDL clients. Read ARCHITECTURE.md
§3.4 first.

Method:
1. tools/imf2atlas/: a standalone C++ CLI linking the kernel's image
   machinery the way kernel/imf2imf.cc does (copy its link closure /
   stub arrangement — it is the existence proof for a standalone imf
   reader). Input: one or more .imf files (default: sweep lib/*.imf and
   the families referenced by images/); output: <name>.png atlas sheets
   + <name>.json index mapping family name -> frame rect, size variants,
   and terrain/unit usage hints available from the imf data.
2. PNG writing: vendor stb_image_write.h under tools/vendor/ with a
   license note (public domain/MIT dual). Pack rects with a simple
   shelf/row packer — no dependency, no cleverness; atlases under
   4096x4096, spill to multiple sheets.
3. Color/transparency: the kernel decodes GIF + imf color/mask data into
   raw buffers (kernel/imf.cc, hardened 7/2026 — see §6.3); convert
   palette+mask to RGBA. Solid-color families (colors, no bitmap) get
   entries in the JSON index without atlas pixels.
4. Acceptance: run over the standard library; assert zero errors, spot-
   check counts (the index entry count vs families defined — compare
   against test/imf-uses.sh's counting approach), and visually inspect
   one atlas (write a trivial HTML contact sheet next to the output for
   manual checks). Wire a CTest (label "tools", quick) that converts one
   small .imf and validates the JSON schema + PNG signature.
5. Not wired into the normal build artifacts yet; xconqd's --http-root
   just serves whatever directory gets generated (document the flow in
   tools/imf2atlas/README.md).
Verify: full build + quick ctest green.
Commit. In MODERNIZATION-PLAN.md §10: mark 10.9 done (strikethrough +
dated note), add newly identified work as new §10 tasks, and update
ARCHITECTURE.md if you deviated from it.
```

- **[10.10] [M] Web client scaffold**: connection, lobby, chat, protocol
  types.

**⚙ PROMPT 10.10 — recommended model: Sonnet.** *(Project scaffolding plus
transcription of a written spec into TypeScript types.)*

```text
Task: scaffold the Xconq web client under web/: TypeScript + Vite, no UI
framework, connecting to xconqd through the session plane. Read
ARCHITECTURE.md (§3.5, §4) and doc/net-protocol.md first. Prereqs: 10.4,
10.6 running (a live xconqd to test against: build/server/xconqd <module>
--port ...).

Method:
1. web/: vite + strict TypeScript, zero runtime dependencies (dev deps
   only). Node toolchain confined to web/ — the C++ build must not grow
   a node requirement. web/README.md: dev-server usage (vite proxy or
   --http-root serving of a built bundle), build instructions.
2. web/src/protocol.ts: hand-written types for every message in
   doc/net-protocol.md, plus a thin ws wrapper (connect, hello/version,
   typed send/receive, reconnect-with-token stub). Keep this file
   organized as the spec's mirror — one section per plane, spec section
   references in comments.
3. UI (plain DOM, minimal CSS): connect form (server URL), game info +
   side list from welcome/manifest, side pick + join, chat pane, and a
   collapsible raw-message log (the debugging tool every later task
   uses). Render notices/history messages as text in the log for now.
4. CI: extend the workflow with a small web job (node LTS, npm ci, tsc
   --noEmit + vite build), non-blocking (continue-on-error) initially,
   mirroring how the macOS job is staged.
Verify: tsc clean; manual round-trip against a live local xconqd (join,
chat, see turn notices arrive) — record what you did in the commit
message; C++ build + quick ctest untouched and green.
Commit. In MODERNIZATION-PLAN.md §10: mark 10.10 done (strikethrough +
dated note), add newly identified work as new §10 tasks, and update
ARCHITECTURE.md if you deviated from it.
```

- **[10.11] [M] Map rendering**: canvas renderer for the side's view.

**⚙ PROMPT 10.11 — recommended model: Opus.** *(Hex geometry, view/fog
semantics, and incremental redraw need real judgment; this is the web
client's core.)*

```text
Task: implement the web client's map view — canvas rendering of the
side's view from snapshot + deltas, using the 10.9 atlases. Read
ARCHITECTURE.md and doc/net-protocol.md first. Prereqs: 10.6, 10.9,
10.10.

Method:
1. web/src/model.ts: the client-side view model — apply snapshot then
   deltas (terrain view layers from RLE, unit views keyed by view id,
   side/turn state). Keep it renderer-independent and unit-testable
   (vitest is acceptable as a dev dep if tests accompany it; otherwise
   assert-heavy code paths).
2. Renderer: canvas, hex AND rect grids (world geometry from the
   manifest — get the hex row-offset math right; the SDL client's
   sdl/*.cc drawing code documents Xconq's cell geometry conventions,
   read it before inventing any). Terrain from atlas sprites or solid
   colors per the atlas JSON; units from unit views with side emblem
   overlays if the atlas provides them; three visibility states
   rendered distinctly: never-seen (blank), remembered-but-not-covered
   (dimmed, stale views), currently-covered (full). Viewport pan
   (drag/keys) + zoom (2-3 fixed levels); redraw only dirtied cells on
   delta application, full redraw on pan/zoom.
3. Turn/date/side status header from side/turn deltas.
4. Acceptance: against a live xconqd running the default introductory
   game with fog on: initial view shows only the starting area;
   exploring (AI or scripted moves via 10.8's protoclient on the same
   game) grows the map; remembered vs covered renders distinguishably.
   Screenshot(s) in the PR/commit description.
Verify: tsc clean, web build green; C++ side untouched (quick ctest
green).
Commit. In MODERNIZATION-PLAN.md §10: mark 10.11 done (strikethrough +
dated note), add newly identified work as new §10 tasks, and update
ARCHITECTURE.md if you deviated from it.
```

- **[10.12] [M] Web client interactions — the playable milestone.**

**⚙ PROMPT 10.12 — recommended model: Opus.** *(Order-giving UX against the
command plane; the acceptance bar is a full game, which exercises
everything built so far.)*

```text
Task: make the web client playable: unit selection, orders, turn flow.
Read ARCHITECTURE.md and doc/net-protocol.md first. Prereqs: 10.7, 10.11.

Method:
1. Selection: click cycles through own units in a cell; selected-unit
   panel (type, hp, acp, supply, current task from own-unit state) from
   the model.
2. Orders -> command messages: move (click destination; show the
   resulting task on the unit), build (chooser from manifest's buildable
   types for the unit type), fire/attack (target click), sentry/skip,
   unit naming, plus a "next unit needing orders" key (drive from
   own-unit task state, client-side).
3. Turn flow: end-turn button + state (waiting on you / waiting on
   others from side deltas), server error responses surfaced as toasts,
   history/notice log pane grown from the 10.10 raw log into a readable
   message list.
4. Acceptance (the phase-B milestone): play the default introductory
   game against the AI in the browser from first turn to a decided
   result (win/lose/resign) without touching another client. Note the
   session length and any protocol gaps hit — gaps become new §10 tasks,
   which is exactly the churn phase B exists to surface before the SDL
   port consumes the protocol.
Verify: tsc clean; C++ quick ctest untouched and green.
Commit. In MODERNIZATION-PLAN.md §10: mark 10.12 done (strikethrough +
dated note incl. the gap list), add newly identified work as new §10
tasks, and update ARCHITECTURE.md if you deviated from it.
```

### Phase C — SDL client port and legacy removal

- **[10.13] [M] `libclientmodel`**: the C++ mirror of a side's view.

**⚙ PROMPT 10.13 — recommended model: Opus.** *(API design that determines
the SDL port's diff size; the accessor-compatibility judgment calls need
the inventory internalized.)*

```text
Task: implement client/ — libclientmodel, the C++ client-side view model
shared by the SDL client. Read ARCHITECTURE.md §3.6, doc/net-protocol.md,
and doc/client-state-inventory.md first. Prereqs: 10.2, 10.8 (recorded
transcripts).

Method:
1. client/ builds libclientmodel.a: C++17, links the vendored
   server/vendor JSON header, NOT the kernel. Contents: manifest-derived
   type info, terrain-view layers, unit views, own-unit state (plans/
   tasks), side/turn/clock state, history log; an apply(message) entry
   point; a subscription hook for UI dirty-notification (mirroring the
   update_* callback granularity, so SDL's existing redraw structure
   maps on).
2. Accessor shape: for each sdl/ kernel-read in the 10.1 inventory
   (class a/b), provide an accessor with a DELIBERATELY similar
   signature/name where reasonable (e.g. terrain_view(x,y)-alikes) and
   record the mapping table in client/README.md — the SDL port (10.14)
   consumes that table as its checklist. Do not contort the design for
   perfect name parity; note divergences in the table.
3. Tests: a client/test/ CTest (label "unit") replaying the 10.8
   transcripts through apply() and asserting model invariants (final
   turn number, unit-view counts, spot-checked cells vs values recorded
   in the transcript scenario). Hand-rolled CHECK style per test/unit/.
Verify: full build + quick ctest green.
Commit. In MODERNIZATION-PLAN.md §10: mark 10.13 done (strikethrough +
dated note), add newly identified work as new §10 tasks, and update
ARCHITECTURE.md if you deviated from it.
```

- **[10.14] [L] SDL client read-path port** (multi-session): render from
  `libclientmodel` in a new network-client mode.

**⚙ PROMPT 10.14 — recommended model: Opus.** *(A weeks-scale port touching
every screen; per-session scoping and kernel-read triage need judgment.
Expect multiple sessions — the prompt is phased so each ends committable.)*

```text
Task: port the SDL client's DISPLAY paths to libclientmodel, as a new
network-client mode. Multi-session; stop at any phase end, each phase
leaves the tree green and committed. Read ARCHITECTURE.md (§3.6),
client/README.md's mapping table, and doc/client-state-inventory.md
first. Prereq: 10.13.

Strategy (two coexisting modes during the port — the legacy in-process
mode keeps working until 10.15 deletes it):
Phase 1: `sdlconq --connect <host:port>` starts a network mode: ws
  client (reuse/adapt the 10.3 client-side code into client/), hello/
  join/side pick (minimal text UI or args), a libclientmodel instance,
  and an SDL window that renders ONLY what is ported so far (map view
  first); unported panels display "not available in network mode".
  Session/input plumbing stays isolated from the legacy path (new files
  preferred over #ifdef soup in existing ones).
Phase 2..N: port one display surface per session, replacing kernel reads
  with model accessors per the mapping table, checking items off IN
  client/README.md's table (that file is the cross-session progress
  tracker): map drawing + view/fog states, unit info panel, side/turn
  status, history/notices, help screens that only need manifest data.
  Where a read has no model accessor yet, add it to libclientmodel WITH
  a transcript test (10.13's harness) in the same commit.
Each session: build both modes; quick ctest green; manual network-mode
run against a live xconqd noting what newly renders (commit message).
Update MODERNIZATION-PLAN.md §10 with a dated progress note on THIS task
each session (surfaces done/remaining), add newly identified work as new
§10 tasks, and update ARCHITECTURE.md if you deviated. Mark the task done
(strikethrough) only when every display surface renders from the model in
network mode.
```

- **[10.15] [M] SDL client command-path port + kernel de-link** — network
  mode reaches parity; legacy in-process mode is removed.

**⚙ PROMPT 10.15 — recommended model: Opus.** *(The cutover: parity
judgment, then an irreversible unlink of the kernel from the UI.)*

```text
Task: finish the SDL client port — input/commands over the protocol,
then DELETE the legacy in-process mode and stop linking the kernel. Read
ARCHITECTURE.md (§3.6, §8) and doc/net-protocol.md first. Prereq: 10.14
complete.

Method:
1. Command paths: for each player-facing command the SDL client
   implements (sweep sdl/sdlcmd.cc against kernel/cmd.def), send the
   protocol command instead of calling the kernel; server errors surface
   in the UI. The 10.1 class-(c) inventory + 10.12's web implementation
   are the reference for message usage.
2. Parity check: play the default introductory game to a result in
   network mode using only the SDL client (against a local xconqd).
   Missing-but-previously-working functionality either gets ported now
   or listed explicitly in the plan note as accepted regressions —
   nothing silently dropped.
3. Cutover: remove the legacy in-process game path from sdl/ (local
   game setup screens, direct kernel calls, the tp.cc host/join UI);
   sdlconq's default behavior becomes connect/host (host = task 10.16;
   if 10.16 hasn't landed, default to a connect dialog and note it).
   Drop libconq linkage from sdl/CMakeLists.txt — sdlconq links
   libclientmodel + SDL3 only. Re-evaluate the direct X11 (Xlib/Xmu/
   Xext) linkage while in there: if it only served legacy paths, drop
   it and note the macOS implication (CLAUDE.md documents the current
   X11 gate).
4. Docs: update CLAUDE.md's UI/build description and README to match.
Verify: full build + quick ctest green (kernel tests unaffected); the
parity game from step 2 recorded in the commit message.
Commit. In MODERNIZATION-PLAN.md §10: mark 10.15 done (strikethrough +
dated note incl. accepted regressions), add newly identified work as new
§10 tasks, and update ARCHITECTURE.md if you deviated from it.
```

- **[10.16] [M] Listen server**: "host a game" from the SDL client; LAN
  peers and browsers join.

**⚙ PROMPT 10.16 — recommended model: Sonnet.** *(Process management with a
clearly specified lifecycle; the architecture decision — child process,
never in-process — is already made.)*

```text
Task: implement LAN hosting from the SDL client — spawn xconqd as a
child process, connect over loopback, let LAN peers join. Read
ARCHITECTURE.md (§3.7) first. Prereqs: 10.4 (xconqd), 10.14 phase 1+
(network mode exists); best after 10.15.

Method:
1. Host flow in sdlconq: a host dialog (game module from lib/game.dir,
   port with a sensible default, variants) -> fork/exec the xconqd
   binary (locate it relative to the sdlconq executable, with the same
   source-checkout fallback logic style as default_library_pathname —
   see kernel/unix.cc) -> wait for readiness (xconqd prints a READY
   line / or poll-connect the port with a timeout) -> connect over
   127.0.0.1 as an ordinary client.
2. Lifecycle: child killed (SIGTERM, then KILL after grace) when the
   hosting client exits or the host cancels; child crash -> clear UI
   error, not a hang. Reap zombies. Pass --http-root pointing at the
   installed/checkout web bundle + atlases if present, so LAN browsers
   can load the web client from http://<host-ip>:<port>/ per
   ARCHITECTURE.md's mixed-content note; show that URL and the LAN
   ip:port in the host dialog for sharing.
3. Join flow: a connect dialog accepting host:port (discovery is task
   10.21).
4. Test: a CTest (label "net") scripting the spawn/ready/connect/
   teardown cycle through the same helper functions the UI uses
   (factor them so they are testable without SDL video — a small
   client/hosting.cc library piece).
Verify: full build + quick ctest green; manual: host on one machine,
join from a browser on the LAN URL (or a second local browser window),
note the result in the commit message.
Commit. In MODERNIZATION-PLAN.md §10: mark 10.16 done (strikethrough +
dated note), add newly identified work as new §10 tasks, and update
ARCHITECTURE.md if you deviated from it.
```

- **[10.17] [S] Retire the legacy network protocol** (`tp.cc`, `socket.cc`)
  — closes the §6.2 residual.

**⚙ PROMPT 10.17 — recommended model: Sonnet.** *(A removal with a clear
blast radius and an established Step-2/Step-3 removal precedent to
follow.)*

```text
Task: remove the legacy lockstep network protocol now that no client
uses it. Read ARCHITECTURE.md §8 first. Prereq: 10.15 (SDL client no
longer calls tp.cc) — verify that, don't assume it.

Method:
1. git rm kernel/tp.cc kernel/socket.cc (and tp.h); remove them from
   kernel/CMakeLists.txt.
2. The net_* wrappers in kpublic.h/tp.cc: their LOCAL execution legs are
   the seams xconqd's command plane calls (task 10.7). Relocate the
   local legs to a non-network-named home (e.g. kernel/kpublic.cc or
   wherever 10.7 put its dispatch) and delete the broadcast legs, the
   hosting/downloading/my_rid/master_rid globals, remote_player_specs,
   and the rid plumbing. grep -r for every removed symbol; the compiler
   is the checklist.
3. Docs per the Step-3 removal precedent (see the Step 3 note in this
   plan's intro): update ACTIVE docs (CLAUDE.md, CONTRIBUTING.md,
   README, doc/INSTALL, man pages) to drop legacy-multiplayer claims;
   leave historical material (doc/net-security-review.md, dated plan
   notes, deep manual chapters) untouched.
4. Update the §6.2 plan item's note: the deferred findings (hostile-peer
   file-open, GDL-reader exposure, no auth) are now moot in-tree — the
   code is gone.
Verify: full fresh configure + build + quick ctest green (all suites —
skelconq/unittests must be unaffected); grep confirms no live references
to tp/socket symbols remain outside git history.
Commit. In MODERNIZATION-PLAN.md §10: mark 10.17 done (strikethrough +
dated note), add newly identified work as new §10 tasks, and update
ARCHITECTURE.md if you deviated from it (§8 should now be describable in
past tense — update it).
```

### Phase D — production server

- **[10.18] [M] Persistence and reconnect**: durable games, session tokens.

**⚙ PROMPT 10.18 — recommended model: Opus.** *(Save/session semantics
across restarts touch kernel restore behavior — the area with the most
historical bugs in this codebase.)*

```text
Task: make xconqd games durable and reconnectable. Read ARCHITECTURE.md
§7 and doc/net-protocol.md (session plane) first. Prereq: 10.7.

Method:
1. Auto-save: xconqd saves via the kernel's existing save machinery
   every N turns (flag) and on SIGTERM; --resume <save> restarts a game
   from one. Respect XCONQHOME conventions; know that check-save waives
   ten KNOWN_UNFAITHFUL modules (test/test-save.sh) — resumed fidelity
   inherits those limits, document that in --help.
2. Session tokens: issued in welcome (crypto-random, from /dev/urandom),
   required for rejoin; persist the token->side map alongside the
   auto-save so reconnect works across a server restart. Expiry policy:
   tokens live as long as the game.
3. Reconnect flow already partially exists (10.6 rejoin snapshot);
   finish it: rejoin after RESTART must land the client in a consistent
   state (fresh snapshot from restored kernel state).
4. Test: net CTest: join, play turns, SIGTERM, restart with --resume,
   rejoin with the old token, assert turn number and a sampled unit
   survive the round trip. Use a save-faithful module (not in
   KNOWN_UNFAITHFUL).
Verify: full build + quick ctest green including check-save (you must
not disturb save semantics for the existing suite).
Commit. In MODERNIZATION-PLAN.md §10: mark 10.18 done (strikethrough +
dated note), add newly identified work as new §10 tasks, and update
ARCHITECTURE.md if you deviated from it.
```

- **[10.19] [M] Front door**: multi-game hosting, process-per-game.

**⚙ PROMPT 10.19 — recommended model: Opus.** *(Multi-process lifecycle and
the routing design; keeps all game logic out of itself, which is a
discipline call.)*

```text
Task: implement the front-door/lobby process for multi-game hosting.
Read ARCHITECTURE.md (§3.8) first. Prereqs: 10.4, 10.18.

Method:
1. server/frontdoor.cc (target xconq-frontdoor, reusing server/net):
   serves the web bundle + atlases, and a lobby plane (extend
   doc/net-protocol.md: list games with status/players, create game
   (module + variants from an allowlist directory — lib/game.dir),
   join -> redirect). Routing model: REDIRECT, not proxy — the client
   receives the game process's port and reconnects there; document the
   firewall implication (a port range for game processes, configurable).
2. Game process management: spawn xconqd per created game (port from
   the configured range, --http-root unset — the front door serves
   statics), track children (pid, port, module, created-at), reap on
   exit, cap concurrent games (flag), idle-game shutdown after
   configurable inactivity (xconqd auto-saves on SIGTERM per 10.18, so
   idle shutdown is safe and games are resumable — wire resume-on-join
   for shut-down games).
3. State: the game registry persists (JSON file) so a front-door restart
   rediscovers resumable games.
4. Test: net CTest: create two games through the lobby plane, join each
   (assert isolation — chat in one never reaches the other), kill one
   game process (assert the lobby notices), restart front door (assert
   the registry survives).
Verify: full build + quick ctest green.
Commit. In MODERNIZATION-PLAN.md §10: mark 10.19 done (strikethrough +
dated note), add newly identified work as new §10 tasks, and update
ARCHITECTURE.md if you deviated from it.
```

- **[10.20] [M] Turn deadlines, AFK policy, spectators.**

**⚙ PROMPT 10.20 — recommended model: Opus.** *(Gameplay-semantics
decisions: what happens to an absent side is a game-design call with
kernel implications.)*

```text
Task: server-enforced turn pacing and spectators for xconqd. Read
ARCHITECTURE.md §7 and doc/net-protocol.md first. Prereq: 10.7; 10.18
for async play to be meaningful.

Method:
1. Investigate first, then build on what exists: the kernel has
   real-time game variables (rt-* gvars in kernel/gvar.def) and the
   sequential/simultaneous turn model in run.cc — document in the
   commit message what the kernel already enforces vs what xconqd must
   add. Do not duplicate a kernel mechanism in the server layer.
2. Deadlines: per-game turn deadline (variant/flag); on expiry the
   server finishes the turn for laggards (units go reserve/sentry — use
   the least-surprising existing kernel behavior; state your choice).
   Deadline/remaining-time deltas keep clients' clocks honest.
3. Absent sides: configurable policy — wait (default for 2-player),
   deadline-skip, or hand to AI after N missed turns (the kernel
   supports AI attachment to a side; verify via how players/sides bind
   at setup). Notices to other players on state changes.
4. Spectators: join as observer (protocol has the concept from 10.2;
   implement binding) — sees a designated side's view or a global view
   ONLY if the game's see-all variant is on; never a fog bypass.
   Spectators cannot send commands (enforce in the command plane).
5. Tests: net CTests for deadline expiry (short deadline, assert turn
   advances without the laggard's end-turn), spectator fog integrity
   (the 10.5 canary assertion, run for a spectator), spectator command
   rejection.
Verify: full build + quick ctest green.
Commit. In MODERNIZATION-PLAN.md §10: mark 10.20 done (strikethrough +
dated note incl. the policy decisions made), add newly identified work
as new §10 tasks, and update ARCHITECTURE.md if you deviated from it.
```

- **[10.21] [S] LAN discovery** (mDNS/DNS-SD) for listen servers.

**⚙ PROMPT 10.21 — recommended model: Sonnet.** *(Bounded protocol feature
with a vendorable single-header implementation path.)*

```text
Task: LAN game discovery — listen servers announce via mDNS, the SDL
client's connect dialog lists discovered games. Read ARCHITECTURE.md
§3.7 first. Prereq: 10.16.

Method:
1. Announce: xconqd (when started with a --lan flag, which the 10.16
   host flow sets) publishes _xconq._tcp via mDNS with TXT records
   (game module, player count, protocol version). Implementation:
   vendor a single-header mDNS library (e.g. mjansson/mdns, public
   domain — license note in server/vendor/) rather than depending on
   avahi; integrate with the existing poll loop (one UDP socket).
2. Browse: the SDL connect dialog (10.16) gains a discovered-games list
   (same mdns header, browse mode, few-second refresh); manual host:port
   entry stays.
3. Web: browsers cannot mDNS; the LAN web path stays "type the URL the
   host dialog shows" — note that limitation in the host dialog text.
4. Test: a net CTest doing announce + browse over loopback multicast in
   one process pair; skip cleanly (ctest SKIP return) if the sandbox/CI
   environment blocks multicast — probe first, don't fail.
Verify: full build + quick ctest green (including the skip path).
Commit. In MODERNIZATION-PLAN.md §10: mark 10.21 done (strikethrough +
dated note), add newly identified work as new §10 tasks, and update
ARCHITECTURE.md if you deviated from it.
```

- **[10.22] [S] Deployment kit**: docs, systemd units, reverse-proxy TLS.

**⚙ PROMPT 10.22 — recommended model: Sonnet.** *(Docs + config templates
against decisions already recorded.)*

```text
Task: write the server deployment kit. Read ARCHITECTURE.md (§4, §6)
first. Prereqs: 10.18, 10.19. HARD PREREQUISITE: §6.4 (path-traversal
audit) must be done before these docs can honestly call a public
deployment supported — check its status in this plan; if it is not done,
say so prominently in the doc and in your plan note.

Method:
1. doc/SERVER.md: architecture recap (one diagram, link ARCHITECTURE.md),
   single-game vs front-door deployment, sample nginx AND caddy configs
   terminating wss:// and proxying to the front door + game-port range,
   firewall notes, resource expectations (a game process's memory/CPU —
   measure one and state the number), backup guidance (the 10.18/10.19
   save + registry files), upgrade procedure (protocol version bump =
   clients must update; no rolling compat pre-1.0).
2. misc/systemd/: xconq-frontdoor.service (hardening directives:
   DynamicUser, ProtectSystem=strict with the state dir, NoNewPrivileges
   — game processes inherit), an example env file. Document manual
   install; no packaging work.
3. A smoke checklist in doc/SERVER.md: commands to verify a fresh
   deployment (protoclient hello against the public endpoint, a browser
   join).
Verify: configs lint (nginx -t / caddy validate if available locally;
otherwise careful review, noted); builds/tests untouched.
Commit. In MODERNIZATION-PLAN.md §10: mark 10.22 done (strikethrough +
dated note), add newly identified work as new §10 tasks, and update
ARCHITECTURE.md if you deviated from it.
```

## Suggested sequencing

1. ~~§3 test honesty (fail on errors) + §2 bug fixes~~ (done 7/2026) — green
   means something now.
2. ~~Remove the Tcl/Tk and Xt/Xaw UIs~~ (done 7/2026, Step 2) — rather than
   porting Tcl/Tk to 9.x, deleted it and the legacy Xt/Xaw UI outright;
   `sdlconq` is now the primary graphical client.
3. ~~§1 language cleanup through the C++17 bump~~ (done 7/2026) — the whole
   section is complete (gnu++17, `.c`→`.cc`, `-fpermissive`/`-Wno-write-strings`
   dropped, `-Wall -Wextra` + curated `-Werror`, compat shims removed);
   tooling and Clang are now unblocked.
4. ~~§4 SDL2/3 port~~ (done 7/2026, SDL2 then SDL3) — `sdlconq` now builds
   against native SDL3 instead of through the sdl12-compat shim.
5. §5–§8 as continuous background work.
6. ~~§9 only if the project attracts sustained interest.~~ *(revised
   2026-07-11: §9.2 is resolved — the successor-UI decision is made and
   recorded in ARCHITECTURE.md; §9.1 de-globalization is explicitly NOT on
   the new critical path; §9.3 GDL ergonomics remains nice-to-have.)*
7. **§10 client/server rearchitecture** (adopted 2026-07-11) is the main
   track going forward: phase A (protocol + `xconqd`), then B (web client,
   which validates the protocol), C (SDL port + legacy protocol removal)
   overlapping B, D (production server) last. **§6.4 (path traversal) must
   land before anything internet-facing deploys** (phase D); the remaining
   §7/§8 items stay background work and are unaffected.
