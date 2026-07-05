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
- **[S] Add unit tests for pure-kernel logic** (GDL read/write round-trip,
  save/restore equality, movement/combat table lookups) — the current tests are
  end-to-end only.

**⚙ PROMPT 3.6 — recommended model: Sonnet.** *(The test cases are
enumerated below; the infrastructure copies skelconq's existing
null-interface pattern.)*

```text
Task: create Xconq's first unit-test binary for pure-kernel logic,
registered in the CTest quick lane.

Context: all current tests drive whole games through skelconq. The kernel
is global-state and has no teardown (one process = one test *session*,
but many independent checks can run sequentially in it). skelconq
(kernel/skelconq.c) is the proof of how to link the kernel with a null
interface: it provides the UI callback stubs the kernel expects — mirror
its stub set rather than inventing one (if practical, factor skelconq's
stubs into a shared file both link, but do not restructure skelconq
beyond that).
Method:
1. Infrastructure: test/unit/ with one C++ binary (unittests) linking the
   kernel libraries, using a tiny hand-rolled harness (a CHECK macro
   counting failures, main returning nonzero if any failed; no external
   framework — the tree may still be gnu++98 depending on §1 progress).
   One CTest entry, LABELS "unit", TIMEOUT 60.
2. Test areas, in dependency order (later ones may need more init —
   discover the minimal init sequence from skelconq.c's startup and
   test-lib's usage; document it in a comment):
   a. Lisp layer (kernel/lisp.c): read-from-string -> write-to-string
      round-trips for representative GDL: integers, negative numbers,
      strings with escapes/quotes, symbols, nested lists, dotted pairs if
      supported (check the reader), dice notation (1d6, fractions) — and
      the regression class from step 1: symbols adjacent to numbers must
      not glue (the u*0 bug), quoted/empty strings.
      Malformed-input rejection: unterminated string/list, token at the
      old BIGBUF boundary (regression for the off-by-one).
   b. Type/table machinery (.def system): after loading a minimal inline
      GDL fragment defining 2-3 unit/terrain types, check table default
      values, set values, and both orientations of a u-by-t table lookup;
      check a gvar default and override.
   c. Utility layer (kernel/misc.h etc.): dice roll min/mean/max
      functions against hand-computed values; pm-scale normalize
      helpers; anything else pure that step-1 bugs touched.
   Save/restore equality at unit granularity is a stretch goal: only if a
   world can be synthesized cheaply from a tiny GDL string; otherwise note
   that check-save covers it end-to-end and skip.
3. Keep each check independent of ordering where the global state
   allows; where it does not (type definitions accumulate), order tests
   explicitly and comment the dependency.
4. Update CLAUDE.md's test section and test/CMakeLists.txt header.
Verify: ctest -L unit green; full quick lane green; break one CHECK
deliberately to confirm nonzero exit, revert.
Commit; mark this item done (strikethrough + date) in MODERNIZATION-PLAN.md.
```

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

## 5. Platform behavior

> Execution prompts for §5–§8 follow the same convention as §1 (fenced
> block, recommended model). Unlike §1 these are mostly **independent** of
> each other — prerequisites are stated per prompt. File extensions are
> given as `.c`; after §1's rename task they may be `.cc` — adapt.

- **[M] Use XDG base directories.** Scores currently want a shared, setgid-era
  `/var/lib/xconq/scores`; per-user data belongs in `$XDG_DATA_HOME/xconq`,
  config in `$XDG_CONFIG_HOME`. Drop the remaining games-group install
  assumptions.

**⚙ PROMPT 5.1 — recommended model: Sonnet.** *(Well-specified path-logic
change; the one hard constraint — test hermeticity — is spelled out below.)*

```text
Task: make Xconq use XDG base directories for per-user data instead of
~/.xconq and the setgid-era shared scores directory.

Context/anchors:
- kernel/unix.c: game_homedir() (~line 187) returns $XCONQHOME if set, else
  $HOME/.xconq (created on demand). Everything per-user (saves, preferences,
  scores) hangs off it; grep for game_homedir and ".xconq" across kernel/
  and the UIs (curses/sdl — the Tcl/Tk UI, which may have built preference
  paths itself, was removed in Step 2, so there is nothing to check there).
- CMakeLists.txt:31: XCONQ_SCORES_DIR defaults to /var/lib/xconq/scores;
  kernel/score.c consults it. The games-group/setgid install model is dead.
Design:
1. HARD CONSTRAINT: $XCONQHOME, when set, must keep overriding everything
   exactly as today — the whole test suite's hermeticity depends on it
   (test/common.sh sets it). Do not change its meaning.
2. Otherwise: data (saves, scores) in $XDG_DATA_HOME/xconq (default
   ~/.local/share/xconq); config/preferences in $XDG_CONFIG_HOME/xconq
   (default ~/.config/xconq). Create with 0700 like the current code.
3. Legacy fallback: if the new dirs are absent but ~/.xconq exists, keep
   reading from ~/.xconq (print a one-time note suggesting a move); never
   write new files there.
4. Scores become per-user under the data dir by default. Keep
   XCONQ_SCORES_DIR as an optional override for shared installs, but stop
   defaulting to /var/lib and remove any games-group/chmod logic from
   CMake install rules and kernel/score.c.
Method: implement in kernel/unix.c (a small xdg_dir helper is fine), sweep
UIs for hardcoded ~/.xconq, update doc/ and man-page mentions.
Verify: build all UIs; ctest --test-dir build --label-exclude long (must
stay green — it exercises XCONQHOME); then a manual smoke run of skelconq
WITHOUT XCONQHOME set (HOME pointed at a scratch dir) confirming files land
under .local/share/xconq and that a pre-seeded .xconq is still read.
Commit; mark this item done (strikethrough + date) in MODERNIZATION-PLAN.md.
```
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
- `sdl/Xconq.RC`, `curses/Xconq.RC`: trivial 2-line `.rc` scripts, each
  just associating `Xconq.ico`/`Xcdoc.ico` (present in both directories)
  as the executable's icon. Not referenced by any CMakeLists.txt — dead
  weight, but harmless and tiny.
- Not previously inventoried, but decision-relevant: `sdl/CMakeLists.txt`
  unconditionally links `X11::Xext`/`X11::Xmu`/`X11::X11` and always
  builds `sdlunix.cc`, whose `main()` (sdlconq's actual entry point) calls
  `setuid`/`geteuid`/X11 geometry parsing directly — i.e. the SDL UI's
  platform glue is Unix/X11-coupled, not just "missing a WIN32 branch."
  There is no `sdlwin32.cc` or equivalent. `curses/` has no such coupling
  (no X11, no setuid) and would be the easier UI to port via PDCurses.

Recommendation: reviving Windows support is more than flipping the
existing `if(WIN32)` switch on. The plausible modern path is an SDL2/3
port of the SDL UI (already the top §4 UI priority) with a real
`sdlwin32.cc`-equivalent main/platform file, `winsock2`/`ws2_32` linkage
for networking (`socket.c` already has partial `WIN32`/`winsock2.h`
handling, untested), a MinGW or MSVC CI leg, and a decision on whether
`curses` gets a parallel PDCurses build (cheaper, since it has no
Unix-specific coupling today). `kernel/win32.c` contributes a plausible
starting point for the kernel-level path/signal glue (it's small and
already CMake-wired) but is unverified and pre-dates modern Win32/UTF-8
path conventions; everything UI-side would be new work. This is a
maintainer call, not made here — the Win32 files are left in place either
way pending that decision.
- **[S] Replace kernel/config.h's homegrown size detection**, which checks
  `SIZEOF_INT`/`SIZEOF_LONG` for its `Z16`/`Z32` typedefs — replace with
  `<cstdint>` fixed width types and delete the checks from `acdefs.h`.

**⚙ PROMPT 5.3 — recommended model: Sonnet.** *(Small, mechanical,
compiler-verified.)*

```text
Task: replace Xconq's homegrown fixed-width typedefs with <cstdint> and
delete the configure-time size checks.

Context/anchors:
- kernel/config.h lines ~58-78: Z16/Z16u/Z32/Z32u typedefs selected via
  #if on SIZEOF_INT / SIZEOF_LONG.
- Top-level CMakeLists.txt lines ~101-103: check_type_size(int/long/
  "long long" ...) feeding SIZEOF_* into the generated acdefs.h (template
  kernel/acdefs.h.in).
Method:
1. In kernel/config.h, define the Z types from <cstdint>: Z16 = int16_t,
   Z16u = uint16_t, Z32 = int32_t, Z32u = uint32_t. Keep the type NAMES —
   do not rename call sites.
2. grep -rn "SIZEOF_INT\|SIZEOF_LONG\|SIZEOF_LONG_LONG" across the whole
   tree. Remove every remaining consumer (fix it to <cstdint>/<climits>
   facts), then delete the check_type_size calls from CMakeLists.txt and
   the corresponding #cmakedefine/#define lines from kernel/acdefs.h.in.
3. Check printf-style format strings for Z32 arguments if any warnings
   appear (int32_t is plain int on all supported platforms, so expect
   none).
Verify: fresh configure (acdefs.h regenerates), build both UIs,
ctest --test-dir build --label-exclude long green.
Commit; mark this item done (strikethrough + date) in MODERNIZATION-PLAN.md.
```

## 6. Security & robustness

- **[M] Unsafe string call audit:** the kernel has ~300 `sprintf` and ~70
  `strcpy` calls into fixed buffers (the bundled `snprintf.c` exists because
  the code predates C99). Convert hot paths to bounded calls; the fuzzers from
  §3 will prioritize targets.

**⚙ PROMPT 6.1 — recommended model: Opus.** *(Security triage: deciding
which of ~400 call sites take attacker-influenced input, and what correct
truncation behavior is at each, is judgment work where a wrong call hides
an overflow.)*

```text
Task: audit and convert Xconq's unbounded string calls (sprintf/strcpy/
strcat/gets-family) to bounded equivalents, prioritizing paths reachable
from untrusted input. This can span multiple sessions — commit in coherent
slices (per subsystem), each leaving the tree green.

Context: ~320 sprintf and ~70 strcpy calls in kernel/ alone, plus more in
the UIs. Untrusted input reaches string formatting through: the GDL reader
(kernel/read.c, lisp.c — downloaded game modules and save files), network
messages (kernel/tp.c feeds the same reader), and player-supplied strings
(side/unit names, chat). Many kernel sites format into shared fixed
buffers (grep for spbuf/tmpbuf and their sizes).
Method:
1. Inventory first: for kernel/, list every sprintf/strcpy/strcat call
   whose format/source includes a %s or variable whose value can originate
   from GDL, saves, network, or player input (follow the data: type/unit/
   side names, module titles, doc strings, file paths). That subset is
   PRIORITY 1; convert all of it.
2. Conversion rules:
   - sprintf(buf, ...) -> snprintf(buf, sizeof(buf), ...) when buf is a
     true array in scope; when buf is a pointer, find the real capacity
     (allocation site or buffer contract) and pass that — never guess.
   - strcpy/strcat -> snprintf or explicit length-checked copies matching
     nearby idiom. Preserve existing semantics for sizes that are provably
     safe; the goal is that overflow becomes truncation, never corruption.
   - Where truncation could change behavior (constructed filenames, GDL
     symbols), decide deliberately: truncate-and-warn (run_warning exists)
     or reject; note the choice in a comment when non-obvious.
3. PRIORITY 2 (best effort, time permitting): remaining kernel sites with
   any %s or variable-length source. Leave purely-numeric formats into
   ample buffers alone or convert mechanically if touching the file anyway.
4. Keep a short tally in the commit message(s): sites converted, sites
   judged safe and left (with why-class), any real overflow found (call
   those out explicitly).
Verify after each slice: build all UIs; ctest --test-dir build
--label-exclude long. Watch for truncation-induced test diffs (save files
are compared for fidelity) — a save/restore mismatch after this change
means a buffer was actually too small; enlarge, don't mask.
When kernel PRIORITY 1 is fully converted, mark this item appropriately
in MODERNIZATION-PLAN.md (done, or a dated progress note if UIs remain).
```
- **[M] Review the homegrown networking layer** (`kernel/tp.c`,
  `kernel/socket.c`): it's a custom peer-to-peer protocol whose messages feed
  the GDL reader. At minimum, fuzz the message decoder; longer term consider
  whether multiplayer survives at all and behind what transport.

**⚙ PROMPT 6.2 — recommended model: Opus.** *(A security review whose
deliverable is analysis and threat modeling, not mechanical edits.)*

```text
Task: security-review Xconq's homegrown networking layer and produce a
written assessment plus fixes for any outright memory-safety bugs found.
This is primarily an ANALYSIS deliverable — do not redesign the protocol.

Context: kernel/tp.c ("transfer protocol") and kernel/socket.c implement a
custom peer-to-peer multiplayer protocol. Received messages ultimately feed
the GDL reader (kernel/read.c / lisp.c), which also parses saves and
modules. Assume a hostile peer.
Method:
1. Read tp.c and socket.c end to end. Document (for the report): framing/
   message format, how lengths are parsed and trusted, buffer handling on
   receive, how messages are dispatched, and exactly where received bytes
   enter the GDL reader or other interpreters (e.g. remote-designer or
   chat paths).
2. Hunt specifically for: unchecked length fields driving memcpy/reads,
   sprintf/strcpy of network data into fixed buffers (coordinate with the
   §6 unsafe-string task if both are in flight), integer overflow in size
   arithmetic, missing bounds on array indices taken from the wire (side
   numbers, unit ids), and any path where a peer can make this host open
   an arbitrary file.
3. Fix ONLY clear, local memory-safety bugs (bounds checks, bounded
   copies) — behavior-preserving hardening. Anything architectural goes in
   the report instead.
4. Deliverable: doc/net-security-review.md containing the protocol
   description, trust-boundary diagram in prose, findings (severity-ordered,
   file:line), what was fixed vs deferred, and a recommendation on the
   plan's open question: is the multiplayer layer worth keeping as-is,
   fuzz-and-harden, or should it be wrapped/replaced (e.g. run only over a
   trusted relay) long-term?
Verify: build all UIs; quick ctest green. If two skelconq/xconq instances
can be made to connect locally (check doc + tp.c for how hosting works),
smoke-test that multiplayer still handshakes after your fixes; if that is
not practical, say so honestly in the report.
Commit code fixes + report; add a dated pointer to the report on this plan
item in MODERNIZATION-PLAN.md (the item stays open until the fuzzing work
from §3 lands).
```
- **[S] Review the custom image decoders** (`kernel/gif.c`, `kernel/imf.c`,
  X11 XBM/XPM readers) — classic CVE territory. Consider delegating GIF to a
  library or restricting to trusted install dirs.

**⚙ PROMPT 6.3 — recommended model: Opus.** *(Decoder auditing is
subtle-bug territory — LZW state machines and dimension arithmetic — where
a superficial pass gives false confidence.)*

```text
Task: security-review and harden Xconq's custom image decoders. Fix
concrete bounds/overflow bugs in place; larger delegation decisions go in
a report note.

Context: the decoders parse files that can arrive alongside downloaded
game modules, so treat input as untrusted. Targets:
- kernel/gif.c — homegrown GIF reader (LZW decoder inside: the classic
  CVE pattern is missing code-size/table bounds and dimension-arithmetic
  overflow).
- kernel/imf.c (+ imf.h, and the imf2imf tool) — the image-family format:
  parsed via the GDL/lisp reader, so structure is lisp-checked, but look
  at what happens with hostile numeric fields (dimensions, offsets, color
  counts) and embedded raw data lengths.
- Any per-UI image loading (sdl/ mostly delegates to its toolkit — verify,
  don't assume). (The x11/ XBM/XPM readers this item used to name were
  removed along with the Xt/Xaw UI in Step 2.)
Method:
1. For each decoder: map every length/dimension/count read from input to
   where it sizes an allocation, indexes an array, or bounds a loop.
   Verify each is range-checked BEFORE use; check multiplication for
   overflow (w*h*depth patterns).
2. In gif.c's LZW loop: check code values against table size, table
   growth against its maximum, and output writes against the pixel
   buffer's real extent.
3. Fix what you find with minimal local checks that fail the load
   gracefully (existing error idiom: init_warning/run_warning — match
   surrounding code) rather than exiting.
4. Keep a findings list (file:line, what an attacker-controlled file could
   do, fix applied) — put it in the commit message, and if anything was
   serious, a dated note on this plan item.
5. Recommendation note (no implementation): whether gif.c should be
   replaced by a library dependency or GIF support dropped (check whether
   images/ actually ships GIFs and whether .imf families reference them).
Verify: build all UIs; ctest quick suite green (the imf machinery is
exercised by the image-family tooling and games with custom images);
run imf2imf over a few images/ files as a smoke test.
Commit; mark this item done (strikethrough + date) in MODERNIZATION-PLAN.md
with the delegation recommendation noted.
```
- **[S] Audit file-path handling** in save/restore and module loading
  (`fopen` of names derived from GDL input or network messages — check for
  directory traversal out of the library dir).

**⚙ PROMPT 6.4 — recommended model: Opus.** *(Requires tracing where each
filename can originate and deciding a containment policy — a wrong "safe"
verdict is an arbitrary-file-read/write primitive.)*

```text
Task: audit every place Xconq opens a file whose name derives from
untrusted input (GDL module content, save files, network messages), and
add containment against directory traversal.

Context: module loading resolves names like (include "foo") against a
library search path (see kernel/module.c and the path logic in
kernel/unix.c — XCONQLIB/XCONQ_DATA_DIR and game_homedir()). Save files
themselves contain module names/filenames that get re-resolved on restore
(step 1 already fixed the worst instance: a save's (filename ...) property
clobbering the real filename — see plan intro). Network peers can send
module names for the host to load.
Method:
1. Inventory: grep for fopen/open_module/open_library_file-style calls in
   kernel/ (module.c, read.c, write.c or save code, imf/image loading,
   score.c) and each UI. For each site, answer: where can the name come
   from (hardcoded / user's own command / GDL content / save content /
   network), and what path resolution happens.
2. Classify: a name from GDL content, a save, or the network is UNTRUSTED
   even though the user chose to open the containing file.
3. Containment policy to implement for untrusted names:
   - reject absolute paths and any name containing ".." components (and
     on principle, backslashes); a plain-name allowlist check (letters,
     digits, dash, underscore, dot, forward slash for subdirs) in ONE
     shared validation helper is preferable to per-site fixes;
   - untrusted names resolve only within the library search path dirs;
   - the user's OWN explicit paths (command line, UI file dialog, .xconq
     prefs) stay unrestricted — do not break "load my file from anywhere".
4. Apply the helper at every untrusted site; on rejection use the
   surrounding error idiom (init_warning etc.), don't exit.
5. Test: add a test/*.g fragment (or extend an existing scripted .inp) that
   attempts (include "../outside") and a save referencing a traversal path,
   asserting a clean warning, wired into an existing check-* script if it
   fits the harness; otherwise document the manual repro in the commit.
Verify: build all UIs; ctest quick suite green — ESPECIALLY check-save
(restore re-resolves module names; your validation must not reject any
legitimate library layout, including subdir includes like "lib/foo").
Commit with a site-by-site summary (trusted/untrusted/fixed); mark this
item done (strikethrough + date) in MODERNIZATION-PLAN.md.
```

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
uses Clang's frontend, which has no equivalent of -fpermissive). If curses
or SDL/X11 headers still trip Clang, scope this task to kernel/ and say
so in the plan note.
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
   (CMake build, two UIs: curses (cconq) and native SDL3 (sdlconq, the
   primary graphical client, ported from SDL 1.2 through SDL2 to SDL3,
   7/2026 — see §4) — the Tcl/Tk and Xt/Xaw UIs were removed in Step 2, 7/2026; a
   minimal UI-section update already landed with that removal, but
   re-verify against the current tree rather than trusting it), quick
   build instructions (cmake -B build &&
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
  headless server.

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
- **[L] Consider a modern UI built on the SDL3 port** (or a web frontend
  driven by a headless kernel over the existing network protocol) as the
  long-term evolution of `sdlconq`, now the sole graphical client since the
  Tcl/Tk interface was removed (Step 2, 7/2026).

**⚙ PROMPT 9.2 — recommended model: Opus.** *(The plan says "consider":
the deliverable is an architecture decision document grounded in reading
the real code, not an implementation. Getting the UI-callback surface
analysis right determines years of subsequent work.)*

```text
Task: produce the design assessment for Xconq's successor UI. This is an
ANALYSIS deliverable (doc/ui-successor-design.md) — implement nothing
beyond throwaway measurements. The maintainer decides afterward.

PREREQUISITES: the §4 SDL3 port has landed (7/2026); the §6.2 networking
review, if done, is direct input for the web-frontend option.
Method:
1. Ground truth first — measure the UI contract. Every UI implements the
   callback surface the kernel expects (the interface functions declared
   in kernel/conq.h / kpublic.h that each UI must provide — enumerate them
   from an existing UI, e.g. what skelconq stubs out vs what sdl/
   implements; tcltk/ was removed in Step 2, so sdl/ or curses/ is now the
   reference implementation). Document: how big is the required surface,
   which parts are display-only vs input vs blocking dialogs, and where
   the kernel assumes synchronous UI responses (the pain point for any async/web
   frontend).
2. Assess the three candidate paths honestly, each with: effort estimate,
   what §1-§9 work it depends on (esp. 9.1 de-globalization for anything
   server-shaped), risks, and what the 200-module games library demands of
   it (arbitrary map sizes, hex/rect terrain, image families, the full
   command set in kernel/cmd.def):
   a. Native SDL3 UI evolved from the sdl/ port (the plan's stated intent
      for it: "sleeker, faster replacement").
   b. Web frontend against a headless kernel over the EXISTING network
      protocol (read kernel/tp.c enough to judge whether the protocol can
      carry a full client or assumes shared local game state — this
      usually kills the idea or reshapes it into option c).
   c. Web frontend against a NEW thin protocol (JSON over websocket)
      exposed by a headless server built on skelconq's null-interface
      pattern (kernel/skelconq.c is the existence proof that the kernel
      runs UI-less).
3. Recommend ONE path with a phased milestone plan (milestone 1 must be
   a playable spectator/hotseat client of the default game, not feature
   parity) and name the first three concrete engineering tasks.
4. Keep `sdlconq`'s (and `cconq`'s) fate explicit: the recommendation must
   say what happens to each of the two remaining UIs under each option.
   (This item previously asked the same question about the Tcl/Tk UI,
   which was removed rather than ported — see Step 2.)
Deliverable: doc/ui-successor-design.md committed; add a dated pointer on
this plan item. Nothing else changes in the tree.
```
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
6. §9 only if the project attracts sustained interest.
