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

> **Execution prompts (added 7/2026).** Each §1 task below carries a
> ready-to-run prompt in a fenced block, written so a fresh session of a
> smaller model can execute it without extra context (CLAUDE.md loads
> automatically and covers build/test basics). Run them **in order** —
> each assumes the previous ones are committed. Each block is labeled with
> the recommended model: **Opus** for tasks needing judgment about intent
> or real-bug triage, **Sonnet** for mechanical error-driven sweeps.

- ~~**[M] Make the code valid without `-fpermissive`.**~~ *(done 7/2026)*: Fix
  what the flag papers over (implicit conversions, old-style casts of function
  pointers, etc.) and drop it. This is a prerequisite for everything below and
  for building with Clang, which lacks an equivalent. In the event, the prior
  modernization steps had already cleaned up the offending idioms: dropping the
  `-fpermissive` generator expression from `xconq_common` left the whole tree
  (kernel, curses, SDL) building clean under GCC 15.2.1 with the full test
  suite green, so no source changes were needed.

**⚙ PROMPT 2.1 — recommended model: Opus.** *(Error-driven but the fixes need
judgment: each diagnostic has a "add a cast" fix and a "fix the actual type"
fix, and picking wrong can hide real bugs.)*

```text
Task: make the Xconq codebase compile as valid C++ without GCC's -fpermissive
flag, then remove the flag from the build.

Context: everything (including .c files) is compiled as C++ (gnu++98).
-fpermissive is applied on the xconq_common INTERFACE target at
CMakeLists.txt:140 (target_compile_options, the
$<$<COMPILE_LANG_AND_ID:CXX,GNU>:-fpermissive> generator expression).
Leave the -Wno-write-strings line alone — that is a separate follow-up task.

Method:
1. Remove the -fpermissive generator-expression line from CMakeLists.txt.
2. Configure a scratch build (cmake -B build-nofperm) and build
   (cmake --build build-nofperm -j 2>&1 | tee /tmp/errs.log). Expect a wall
   of errors; work through them file by file. Rebuild frequently.
3. Typical error classes and the preferred fixes:
   - invalid pointer conversions (void* -> T*, T* -> U*): add the explicit
     cast that matches what the code actually stores/reads, e.g. results of
     xmalloc, Lisp object payloads, and UI callback data.
   - function-pointer type mismatches (esp. in the .def X-macro tables and
     any UI command-dispatch tables in curses/ or sdl/): fix the function's
     signature to the expected type when it is only used through that table;
     otherwise cast at the registration site with the correct target type.
   - implicit int / missing prototypes / K&R remnants: declare the real type.
   - assigning const to non-const: defer if it is a string-literal case
     (next task); otherwise fix the declaration.
4. IMPORTANT constraints:
   - Do not change runtime behavior; these must all be type-level fixes.
   - The kernel's core structs/enums/tables are generated by the X-macro
     .def system (kernel/*.def expanded by headers). If an error points into
     an expansion, fix the .def entry or the expanding header, never any
     seemingly "generated" code (nothing is generated on disk).
   - Do not upgrade the C++ standard, rename files, or fix unrelated warnings.
5. When the whole tree builds with both UIs
   (cmake -B ... with default options; curses/sdl both found in CI),
   run: ctest --test-dir build-nofperm --label-exclude long. All tests must
   pass.
6. Also update the "Historical note" comment near CMakeLists.txt:122-125 to
   drop the -fpermissive mention, and remove the build-nofperm scratch dir.
7. Commit with a message summarizing the error classes fixed. Then mark this
   task done (strikethrough + date) in MODERNIZATION-PLAN.md, like the ones
   in §2.
```
- **[M] Fix string-literal constness and drop `-Wno-write-strings`.** The
  CMake migration already fixed ~20 `const char *` signature mismatches; a
  systematic pass (mostly in the UIs and the `.def` tables) finishes the job.

**⚙ PROMPT 2.2 — recommended model: Opus.** *(The hard part is proving a
string is never mutated before const-ing it; the kernel tokenizes and edits
strings in place in places, and a wrong `const` + `const_cast` can turn a
working mutation into undefined behavior on a literal.)*

```text
Task: make the Xconq codebase const-correct for string literals, then remove
-Wno-write-strings from the build. Prerequisite: the -fpermissive removal
task is already committed.

Context: -Wno-write-strings is applied on the xconq_common INTERFACE target
in the top-level CMakeLists.txt (target_compile_options, generator
expression). In C++, string literals are const char[]; this codebase
predates that and assigns literals to plain char* everywhere.

Method:
1. Remove the -Wno-write-strings line; to force yourself through the
   backlog, temporarily add
   $<$<COMPILE_LANGUAGE:CXX>:-Werror=write-strings> in its place.
2. Build and fix errors. Preferred fixes, in order:
   a. Change the pointer/parameter/struct field to const char* when nothing
      ever writes through it. Follow the pointer: check every assignment and
      every function it is passed to before const-ing. Expect ripple effects
      through function signatures — chase them to a fixed point rather than
      casting mid-chain.
   b. The .def X-macro tables (kernel/*.def expanded in headers) contain
      many literal-string fields (names, docstrings, GDL symbols). Change
      the field types to const char* in the structs/accessors declared in
      the expanding headers (kernel/*.h), not in generated-looking code
      (nothing is generated on disk).
   c. Where an external API genuinely takes writable char* (curses/SDL
      toolkit entry points), do NOT const the xconq side; keep a char* and,
      if the source is a literal, either copy it into a buffer or use a
      const_cast at the exact call site with a one-line comment naming the
      API that demands it.
   d. If code really does mutate a buffer that is sometimes initialized from
      a literal, replace the literal initialization with a copy
      (e.g. xmalloc + strcpy, matching nearby idiom) — do not const it.
3. Constraints: no behavior changes; do not bump the C++ standard; do not
   fix unrelated warnings; keep diffs mechanical and reviewable.
4. When the tree builds clean with both UIs, drop the temporary
   -Werror=write-strings (plain default behavior is fine — the warning only
   existed because of the suppression), and run
   ctest --test-dir build --label-exclude long. All tests must pass.
5. Commit, then mark this task done (strikethrough + date) in
   MODERNIZATION-PLAN.md.
```
- **[S] Replace the `min`/`max` macros in `kernel/misc.h`** with inline
  functions (or `std::min`/`std::max`). This is the sole reason the build is
  pinned to gnu++98 — libstdc++ headers clash with the macros under C++17.

**⚙ PROMPT 2.3 — recommended model: Sonnet.** *(Small, mechanical, verified
entirely by the compiler.)*

```text
Task: replace the min/max preprocessor macros in the Xconq kernel with inline
functions so they stop clashing with libstdc++ headers (this is what pins the
build to gnu++98). Prerequisites: the -fpermissive and write-strings tasks
are committed.

Context: kernel/misc.h defines, under #ifndef min / #ifndef max guards
(around lines 43-64):
    #define min(x,y) (((x) < (y)) ? (x) : (y))
    #define max(x,y) (((x) > (y)) ? (x) : (y))
Everything in the tree is compiled as C++, so replacing them with templates
is safe.

Method:
1. Replace both macro definitions (and their #ifndef guards) with inline
   template functions in the same header, keeping the lowercase names so no
   call site changes:
       template<typename T> inline T min(T x, T y) { return x < y ? x : y; }
       template<typename T> inline T max(T x, T y) { return x > y ? x : y; }
   Keep/adapt the existing doc comments.
2. Build all targets. Two failure classes to expect:
   - Mixed-type call sites (e.g. min(int, long) or min(int, short)) that
     compiled under the macro but now fail template deduction: fix the call
     site with an explicit cast or min<int>(...), preserving the value range
     the code needs.
   - Other files defining or #undef-ing min/max, or system headers that
     previously got clobbered by the macro. grep -rn "define min\|define max"
     and clean up duplicates so kernel/misc.h is the single definition.
3. Do NOT bump the C++ standard in this task (that is the next task); just
   make sure everything still builds at gnu++98 and
   ctest --test-dir build --label-exclude long passes.
4. Sanity-check the unblock: configure a scratch build with
   -DCMAKE_CXX_STANDARD=17, confirm the previous min/max-vs-libstdc++ header
   clash is gone (other C++17 errors are fine and expected — ignore them),
   then delete the scratch build dir.
5. Commit, then mark this task done (strikethrough + date) in
   MODERNIZATION-PLAN.md.
```
- **[M] Bump the C++ standard incrementally: gnu++98 → 11 → 17.** After the
  macro fix, each bump is mostly mechanical (`register`, `auto_ptr`-era idioms,
  narrowing conversions). Stop at C++17; the code gains little from newer.

**⚙ PROMPT 2.4 — recommended model: Sonnet.** *(Compiler-error-driven with
well-known fix patterns; escalate to Opus only if a bump surfaces a genuinely
ambiguous semantic change.)*

```text
Task: bump Xconq's C++ standard from gnu++98 to C++11, then to C++17, fixing
what each bump breaks. Prerequisites: the -fpermissive, write-strings, and
min/max-macro tasks are all committed (the min/max task removed the only
known header clash blocking C++17).

Context: the pin is in the top-level CMakeLists.txt:
    set(CMAKE_CXX_STANDARD 98)          # bump this
    set(CMAKE_CXX_STANDARD_REQUIRED OFF) # change to ON
    set(CMAKE_CXX_EXTENSIONS ON)         # keep ON (gnu++NN) to minimize churn
with an explanatory comment just above it that must be updated/removed.

Method — two separate passes, each ending in its own commit:
Pass 1 (C++11):
1. Set CMAKE_CXX_STANDARD to 11 and CMAKE_CXX_STANDARD_REQUIRED to ON.
2. Build everything; fix errors. Expected classes: narrowing conversions in
   brace initializers (fix the element type or add a cast); string-literal
   concatenation now needing a space (e.g. "foo"MACRO -> "foo" MACRO);
   changed meaning of >> in templates (rare here); identifiers that are now
   keywords. `register` is only deprecated in C++11, not an error — leave
   those for pass 2 where they become errors.
3. Both UIs must build; ctest --test-dir build --label-exclude long must
   pass. Commit.
Pass 2 (C++17):
4. Set CMAKE_CXX_STANDARD to 17.
5. Expected classes: `register` storage class is now an error — delete the
   keyword (plain sed-style sweep is fine, it appears in old loops);
   dynamic-exception specifications `throw(...)` are errors — delete them;
   any lingering auto_ptr/binder1st-era usage — replace with the obvious
   modern equivalent; new header-clash fallout if any min/max-style macro
   survived (fix the macro, as in the previous task).
6. Same bar: both UIs build, quick ctest passes. Update the comment
   above the standard settings to say why 17 (and why not newer). Commit.
Constraints: this is a compile-fix task, not a modernization sweep — do not
refactor to auto/range-for/smart pointers, do not fix unrelated warnings.
Finally, mark this task done (strikethrough + date) in MODERNIZATION-PLAN.md.
```
- **[M] Rename `.c` files that are compiled as C++ to `.cc`** (or `.cpp`) and
  drop the `LANGUAGE CXX` override in CMake. The current arrangement confuses
  every editor, indexer, and new contributor.

**⚙ PROMPT 2.5 — recommended model: Sonnet.** *(Pure mechanics: git mv +
build-list edits + reference sweep, fully verified by the build.)*

```text
Task: rename Xconq's .c files that are compiled as C++ to .cc, and remove the
LANGUAGE CXX override machinery from CMake. Prerequisites: all earlier §1
tasks (through the C++17 bump) are committed.

Context: ~56 .c files (kernel/ ~53, curses/ 3) are compiled as C++ via the
helper function xconq_cxx_sources() defined in the top-level CMakeLists.txt
(it sets LANGUAGE CXX source properties). Each UI directory's CMakeLists.txt
calls it on its source lists. The sdl/ UI already uses .cc. (The tcltk/ and
x11/ UIs, which used to add ~30 more .c files here, were removed in Step 2.)

Method:
1. Determine the exact rename set: every .c file that appears in a source
   list passed to xconq_cxx_sources() in kernel/, curses/ CMakeLists.txt.
   Dead platform files NOT in the build (e.g. kernel/mac.c, kernel/win32.c —
   verify by checking the source lists) stay .c: their fate is decided in
   plan §5, don't touch them.
2. Rename with `git mv file.c file.cc` so history follows.
3. Update every reference to the old names:
   - the source lists in each CMakeLists.txt;
   - any #include of a .c file (grep -rn '#include.*\.c"' — rare but check);
   - test/ scripts and test/*-diff.sh / *-uses.sh consistency checks that
     grep kernel sources by filename;
   - doc/ and CLAUDE.md prose that mentions the .c-compiled-as-C++
     arrangement (CLAUDE.md's "What this is" section must be rewritten to
     say sources are now .cc; also update the CMake historical-note comment).
4. Delete the xconq_cxx_sources() function and all its call sites once no
   .c files remain in any target.
5. Full verification: fresh configure from scratch (delete and re-create the
   build dir — stale LANGUAGE properties can mask breakage), build both
   UIs, run ctest --test-dir build --label-exclude long, and run the
   test/*-diff.sh and *-uses.sh scripts by hand to make sure the filename
   sweep didn't break their greps.
6. Commit (the diff is huge but should be almost entirely renames), then
   mark this task done (strikethrough + date) in MODERNIZATION-PLAN.md.
```
- **[M] Turn on real warnings (`-Wall -Wextra`) and burn the backlog down.**
  Start with warnings-as-errors on a curated subset (return types, uninitialized
  reads, format strings) and widen over time.

**⚙ PROMPT 2.6 — recommended model: Opus.** *(Warning triage is judgment
work: many of these warnings are real 20-year-old bugs, and the fix must
address the bug, not silence the diagnostic.)*

```text
Task: enable -Wall -Wextra on the Xconq build, make a curated subset of
warnings hard errors, and burn down that subset. Prerequisites: all earlier
§1 tasks (through the .cc renames) are committed.

Context: the build currently has no warning flags at all. Common flags live
on the xconq_common INTERFACE target in the top-level CMakeLists.txt
(target_compile_options with generator expressions — follow that pattern).

Method:
1. Add -Wall -Wextra for GNU/Clang to xconq_common. Build everything and
   capture the full warning log; summarize counts by warning flag
   (-Wno-error categories) so the commit message can record the baseline.
2. Promote this curated subset to errors and fix every instance:
       -Werror=return-type
       -Werror=uninitialized -Werror=maybe-uninitialized  (GCC; build a
        Release/-O2 config too — these only fire with optimization)
       -Werror=format -Werror=format-security
       -Werror=implicit-fallthrough
   Fix rules:
   - return-type: make the function actually return the right thing on every
     path; check callers to see what value they expect on the missing path.
   - uninitialized: initialize with the value the logic implies, not blindly
     0 — read the surrounding code; if a path is truly impossible, prefer
     restructuring over a dummy init. maybe-uninitialized false positives may
     be silenced with an init + short comment.
   - format: fix the format string or argument (these are real bugs when
     wrong); never cast to shut them up.
   - implicit-fallthrough: decide from the logic whether the fallthrough is
     intended (annotate with /* fall through */ which GCC honors) or a
     missing break (add it — and note any such finding in the commit
     message, since it is a behavior change).
3. Do NOT try to clear all of -Wall -Wextra; anything outside the curated
   subset stays a plain warning for future passes. If sign-compare or
   unused-parameter noise is overwhelming, you may add targeted
   -Wno-... entries on xconq_common with a comment, to keep logs readable.
4. Any warning that reveals a genuine behavior bug (wrong operator
   precedence, dead condition, missing break): fix it and list it explicitly
   in the commit message.
5. Both UIs build; ctest --test-dir build --label-exclude long passes
   in both Debug and -O2 configs. Commit, then mark this task done
   (strikethrough + date) in MODERNIZATION-PLAN.md, noting the remaining
   warning baseline count.
```
- **[S] Remove bundled compat shims that modern platforms make dead code:**
  `kernel/snprintf.c` (C99), `kernel/timestuff.c` (`gettimeofday` is
  everywhere), and consider replacing `kernel/obstack.c` (bundled GNU obstack)
  with plain allocation.

**⚙ PROMPT 2.7 — recommended model: Sonnet.** *(Deletion of dead compat code
with clear verification; the only judgment call — obstack — is scoped to an
assessment, not a mandate.)*

```text
Task: remove Xconq's bundled compatibility shims that modern platforms make
dead code. Prerequisites: earlier §1 tasks are committed (at least the
C++17 bump; this task is independent of the warnings task).

Context: kernel/CMakeLists.txt unconditionally compiles snprintf.c,
timestuff.c, and obstack.c into the kernel libraries (they appear in the
source lists around lines 47 and 61-62; extensions may now be .cc after the
rename task — adapt). Note the files themselves guard their contents with
config #ifdefs, so check what actually gets compiled before assuming.

Method:
1. kernel/snprintf.c (or .cc): a pre-C99 snprintf/vsnprintf fallback. Check
   how it is gated (grep for HAVE_SNPRINTF / HAVE_VSNPRINTF in the file,
   kernel headers, CMakeLists.txt, and kernel/acdefs.h.in). Delete the file,
   its source-list entry, any configure-time checks and acdefs.h.in
   defines for it, and any prototype declarations in kernel headers. Callers
   just use libc snprintf.
2. kernel/timestuff.c (or .cc): gettimeofday/time compat. Same treatment:
   find what symbols it provides (grep the tree for each), confirm every
   user is satisfied by POSIX libc, delete file + build entry + related
   configure checks/prototypes. If it provides something genuinely still
   needed (check before deleting!), keep just that piece and say so.
3. kernel/obstack.c/.h (bundled GNU obstack): its only kernel user is
   kernel/help.c (~8 obstack_ calls). ASSESS rather than blindly replace:
   read how help.c uses it (likely accumulating variable-length help text).
   If the usage maps cleanly onto a simple grow-able buffer or the existing
   xmalloc idioms, replace it and delete obstack.[ch]; if the replacement
   would be invasive or subtle, leave obstack alone and record the finding
   as a note on this plan item instead. Do not half-convert.
4. Sweep for leftovers: grep for snprintf.c/timestuff.c/obstack in doc/,
   test/ scripts, pkg/, and CLAUDE.md; clean up references.
5. Verify: fresh configure + build of both UIs,
   ctest --test-dir build --label-exclude long passes. The help system is
   the risk area if obstack was replaced — also run the skelconq help
   smoke check (test/check-test or the help .inp scripts under test/).
6. Commit, then mark this task done (strikethrough + date) in
   MODERNIZATION-PLAN.md, including the obstack decision.
```

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

- **[S] Split `check-lib`/`check-actions` into one CTest per game module** so
  failures name the offending `.g` file and run in parallel.

**⚙ PROMPT 3.1 — recommended model: Sonnet.** *(CMake/shell mechanics; the
policy logic already exists in test/common.sh and must not change.)*

```text
Task: split Xconq's aggregate check-lib and check-actions CTest entries
into one test per game module, runnable in parallel.

Context: test/CMakeLists.txt registers each test-*.sh script as a single
CTest (see the XCONQ_TEST_SCRIPTS loop); test-lib.sh and test-acts.sh
iterate over the modules internally, applying the shared policy in
test/common.sh (crashes fail everywhere; lib/game.dir games are also held
to no-diagnostics/save standards). Scripts run from the binary dir with
XCONQHOME pointed into it.
Method:
1. Add a single-module mode to test-lib.sh and test-acts.sh (e.g.
   `test-lib.sh <srcdir> <module.g>`), reusing common.sh unchanged so the
   per-run policy/bounds stay identical. Keep the no-argument sweep mode
   working for manual use.
2. In test/CMakeLists.txt, glob lib/*.g at configure time (file(GLOB ...
   CONFIGURE_DEPENDS)) and add_test one check-lib-<module> and
   check-actions-<module> per file, replacing the aggregate check-lib /
   check-actions entries. Keep total quick-lane behavior: same set of
   modules, per-test TIMEOUT scaled down (the old 1800s covered the whole
   sweep; ~600s per module matches the script's internal 10-min bound).
   Give them LABELS "lib"/"actions" so ctest -L works.
3. PARALLEL SAFETY is the real work: with ctest -j, tests share the binary
   dir. Give each test its own scratch subdirectory (working directory or
   an env/argument the script uses) so XCONQHOME, saves, and *.log files
   cannot collide. Verify by running ctest -j$(nproc) twice and comparing
   failures to a serial run.
4. Check whether test-save.sh and test-test.sh want the same split; do
   them too if the mechanism drops out for free (test-save.sh has the
   KNOWN_UNFAITHFUL waiver list — keep it working), otherwise leave and
   note.
5. Update the comment header in test/CMakeLists.txt, CLAUDE.md's "Running
   tests" section, and .github/workflows/c-cpp.yml if it names tests.
Verify: ctest --test-dir build --label-exclude long -j$(nproc) green;
deliberately break one lib module locally to confirm the failure names
exactly that module's test, then revert.
Commit; mark this item done (strikethrough + date) in MODERNIZATION-PLAN.md.
```
- **[M] Add a CI matrix:** GCC + Clang, Debug + Release, and a macOS job
  (the kernel and SDL/curses UIs should be close to working there).

**⚙ PROMPT 3.2 — recommended model: Sonnet.** *(Workflow wiring; the
escape hatches for the two risky legs — Clang and macOS — are specified.)*

```text
Task: turn Xconq's single-configuration CI into a matrix: GCC + Clang,
Debug + Release on Ubuntu, plus a macOS job.

PREREQUISITE for the Clang legs: the §1 -fpermissive removal (prompt 2.1)
must be committed — Clang has no equivalent flag. If it has not landed,
do the GCC matrix + macOS now and leave a commented-out Clang leg with a
note.
Context: .github/workflows/c-cpp.yml currently builds once on Ubuntu with
the default compiler and runs ctest --label-exclude long.
Method:
1. Ubuntu matrix: {gcc, clang} x {Debug, Release} via CC/CXX and
   -DCMAKE_BUILD_TYPE. Both UIs must configure (same apt packages);
   quick test lane must pass on every leg. Release is the leg most likely
   to surface new warnings/UB-dependent behavior — report, don't paper
   over: a test that fails only in Release is a real finding; file it as
   a note on this plan item if you cannot fix it quickly.
2. macOS job (macos-latest): install deps via brew. Expect: sdl12-compat may
   or may not be available (sdl2 + sdl12-compat exist in brew — try; skip
   the SDL UI if not); curses ships with the OS. (The Tcl/Tk and Xt/Xaw UIs
   this note used to caveat around no longer exist — see Step 2.)
   Minimum bar for the job to be worth merging: kernel + skelconq +
   cconq build and the quick ctest lane passes. Fix small portability
   issues this exposes (BSD vs GNU userland in test scripts, missing
   includes); if something needs real porting work, scope it out —
   mark the macOS job continue-on-error: true with a linked note on this
   plan item rather than shipping a red main branch.
3. Keep total CI time sane: build with -j, cache nothing fancy initially,
   and keep the long label excluded everywhere.
4. Update CLAUDE.md's CI description and the README build/status section
   if it exists.
Verify: all matrix legs green in an actual CI run (push to a branch/PR
and check, do not merge red), local spot-check of one clang Debug build +
ctest.
Commit; mark this item done (strikethrough + date) in MODERNIZATION-PLAN.md,
recording which UIs build on macOS.
```
- **[M] Add ASan/UBSan CI jobs** running the quick test suite. A codebase this
  old will surface real bugs immediately; fix as they appear.

**⚙ PROMPT 3.3 — recommended model: Opus.** *(The wiring is easy; the job
will immediately report real memory bugs in 30-year-old code, and fixing
those correctly — not just quieting the sanitizer — is the actual task.)*

```text
Task: add ASan+UBSan builds of Xconq to CI running the quick test suite,
and fix (or explicitly suppress, with justification) everything they
report until the job is green.

Method:
1. Build config: a CMake toggle (-DXCONQ_SANITIZE=address,undefined style
   or an env-driven flags block) adding -fsanitize=address,undefined
   -fno-omit-frame-pointer -g to compile and link flags, on
   RelWithDebInfo. Wire one Ubuntu CI job using it (GCC or Clang,
   whichever leg exists in CI by now), running
   ctest --label-exclude long.
2. Runtime environment, decided up front and documented in the workflow:
   - ASAN_OPTIONS=detect_leaks=0 to start. The kernel allocates via
     xmalloc and frees almost nothing by design; LSan would drown the
     signal. Note on the plan item that leak checking is deferred until
     §9 de-globalization gives the kernel a teardown path.
   - UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1 — UB findings fail
     the run.
3. Expect and triage real findings. Rules:
   - ASan errors (overflow, use-after-free): always real; fix the bug,
     never suppress. The step-1 history (BIGBUF off-by-one, undersized
     per-type side arrays) says more of this class exists.
   - UBSan: fix real ones (shifts, signed overflow in coordinate/pm-scale
     math can change behavior between builds); for a genuinely benign
     high-noise class (e.g. misaligned reads in an image loader that is
     otherwise correct), use a UBSan suppressions file checked into
     test/ with one comment per entry saying why — keep it short; every
     entry is debt.
   - Every behavior-relevant fix gets called out in its commit message;
     the save/restore tests are the regression net — a sanitizer fix that
     changes a save is wrong (or was hiding a bug worth explaining).
4. Bounds: sanitized runs are 2-5x slower. If per-game or per-test
   timeouts trip, scale them via an environment knob consumed by
   test/common.sh and the CTest TIMEOUT properties for this job only —
   do not raise limits globally.
5. Do this in slices if the backlog is big (one commit per finding class),
   keeping the job allowed-to-fail until clean, then flip it blocking.
Verify: sanitizer CI job green and blocking; normal jobs unaffected;
quick ctest green locally in both sanitized and normal builds.
Commit(s); mark this item done (strikethrough + date) in
MODERNIZATION-PLAN.md, listing the bugs fixed and any suppressions.
```
- **[M] Fuzz the GDL reader** (`kernel/read.c`, `kernel/lisp.c`) with
  libFuzzer/AFL. It parses untrusted input: downloaded game modules, network
  messages, and save files all go through it.

**⚙ PROMPT 3.4 — recommended model: Opus.** *(Harness design against a
global-state kernel plus crash triage; a naive harness either won't reach
the interesting code or will "find" init-order artifacts that aren't real.)*

```text
Task: build a libFuzzer harness for Xconq's GDL reader, run it locally
until quiet, fix what it finds, and wire a short CI smoke-fuzz job.
This hardens a defensive surface: the reader parses downloaded modules,
save files, and network-fed data.

PREREQUISITE: builds with Clang (§1 prompt 2.1 done). The §3 sanitizer
task (3.3) is complementary but not required.
Method:
1. Harness (new test/fuzz/fuzz_gdl.cc): LLVMFuzzerTestOneInput feeds one
   input buffer through the lisp-object reader. Study kernel/lisp.c and
   read.c for the entry point that parses a stream/string into objects
   (the same machinery module loading uses); prefer the lowest entry that
   still exercises real parsing (strings, escapes, numbers/dice notation,
   symbols, nested lists) WITHOUT needing a full game world. Mind global
   state: the lisp package/obstack allocators persist across iterations —
   do one-time init in LLVMFuzzerInitialize, and confirm iteration N's
   parse cannot be corrupted by N-1's leftovers (interned symbols
   accumulate — that is acceptable memory growth, not corruption; use
   -rss_limit_mb generously). If interpretation of *forms* (interp_form
   and friends) is reachable without a full game, add a second harness
   for it; do not force it in one.
2. Build wiring: an opt-in CMake target (XCONQ_FUZZ=ON, Clang-only,
   -fsanitize=fuzzer,address,undefined on the kernel objects it links).
   Keep it out of default builds.
3. Corpus: seed from test/*.g and a selection of lib/*.g (small ones),
   plus a dictionary generated from kernel/keyword.def's GDL symbols
   (a small script can extract them — keywords steer the fuzzer into
   real syntax fast).
4. Run locally >= a few CPU-hours (parallel jobs fine). Triage every
   crash/OOM: minimize (llvm's -minimize_crash), identify root cause in
   the reader, fix with bounds/validation matching surrounding idiom
   (history: the reader already had a BIGBUF off-by-one — expect
   relatives). Every fixed crash's minimized input goes into
   test/fuzz/corpus/ as a regression seed.
5. CI: a short job (Clang, ~5 minutes of fuzzing with the checked-in
   corpus + -runs bound) as a smoke test, non-optional once green.
   Document longer local runs in test/fuzz/README.md. Note OSS-Fuzz as a
   future option on the plan item, don't apply.
Verify: harness builds and runs clean over the full checked-in corpus;
normal build + quick ctest unaffected and green (reader fixes are on the
hot path for every game load — the lib sweep is the regression test).
Commit(s); mark this item done (strikethrough + date) in
MODERNIZATION-PLAN.md, recording bugs found/fixed.
```
- **[S] Port the `test/*-diff.sh` / `*-uses.sh` consistency checks into CTest**
  (docs ↔ `.def` symbols ↔ library usage) so they can't silently rot.

**⚙ PROMPT 3.5 — recommended model: Sonnet.** *(Script conversion with a
defined pass/fail policy; the one judgment point — real drift found on
first run — has explicit handling below.)*

```text
Task: wire Xconq's consistency-check scripts into CTest so doc/.def/
library drift fails visibly.

Context: the scripts are test/cmd-diff.sh, game-diff.sh, game-uses.sh,
imf-diff.sh, imf-uses.sh, lib-uses.sh (glob for others). They
cross-check .def symbol lists, the Texinfo docs, and library usage.
Currently manual-only; unknown exit-code discipline.
Method:
1. Read each script: what it compares, whether it is sh or still csh
   (step 1 converted the game-running scripts; these may not be), and
   whether it exits nonzero on differences or just prints them. Convert
   csh -> sh matching the style of the converted scripts, and make each
   exit nonzero exactly when a real inconsistency is found.
2. Run them all. Expect drift after 20 years. Handle it in this order:
   (a) fix trivial drift at the source (a doc missing a recently added
   symbol, a stale entry — these are the point of the checks);
   (b) if a script's whole premise has rotted (compares against something
   that no longer exists), fix its inputs if cheap, else leave it OUT of
   CTest with a dated comment in the script saying why;
   (c) for known-legitimate exceptions, give scripts an explicit waiver
   list at the top (the KNOWN_UNFAITHFUL pattern in test-save.sh is the
   house style) — never a blanket ignore.
3. Register each surviving script in test/CMakeLists.txt as
   check-consistency-<name>, LABELS "consistency", small TIMEOUT (60s),
   runnable from the binary dir (they read sources — pass the source dir
   as the scripts' argument like the existing entries do).
4. Update CLAUDE.md's test section (it lists these as manual-only) and
   the test/CMakeLists.txt header comment.
Verify: ctest -L consistency green; full quick lane green; deliberately
add a bogus symbol to a .def file locally and confirm the relevant check
fails, then revert.
Commit; mark this item done (strikethrough + date) in MODERNIZATION-PLAN.md,
noting any scripts left unwired and why.
```
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
- **[M] Port the SDL UI from SDL 1.2 to SDL2 or SDL3.** It currently builds
  only via sdl12-compat. It was explicitly intended as the successor UI
  ("sleeker, faster replacement"), and now that the Tcl/Tk UI is gone
  (Step 2, 7/2026) it is the sole graphical client and the top UI priority;
  an SDL3 port is the most plausible long-term cross-platform frontend.
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

**⚙ PROMPT 5.2 — recommended model: Sonnet.** *(The deletion half is already
decided by the plan; the Windows half is explicitly scoped to an assessment,
so no judgment call is delegated.)*

```text
Task: delete Xconq's dead Classic Mac OS support, and write an assessment
(not an implementation) of what reviving Windows support would take.

Part 1 — delete Classic Mac code (the plan has already decided this):
1. Candidate files: kernel/mac.c, sdl/sdlmac.cc. (The Tcl/Tk-side Classic Mac
   files this task used to list, tcltk/tkmac.c, tcltk/tkxmac.c,
   tcltk/iappmac.c, were already removed with the rest of tcltk/ in Step 2 —
   nothing to do there now.) Verify each remaining candidate is NOT
   referenced by any CMakeLists.txt (it should not be), then git rm it.
2. Sweep for Classic-Mac-only conditional code and references:
   grep -rn "MAC_OS\|MACINTOSH\|THINK_C\|MPW\|__MWERKS__\|mac\.c" across
   kernel/, UIs, doc/, pkg/, and CMake comments. Remove #ifdef'd Classic
   Mac blocks in shared files ONLY where the guard clearly names Classic
   Mac toolchains/APIs (Carbon/Toolbox, the macros above). Do not touch
   anything that could mean modern macOS (e.g. __APPLE__, unix paths) —
   modern macOS runs the unix/SDL/curses code and must keep working.
3. Also remove Classic-Mac resource/media leftovers if clearly tied to it
   (check misc/ and images/ for .hqx/.rsrc-style files) and prune doc
   mentions (README, doc/INSTALL-*).
Part 2 — Windows assessment (do NOT delete or implement):
4. Inventory the Win32 code: kernel/win32.c and any remaining .rc files.
   (tcltk/tkwin32.c, tcltk/iappwin32.c, pkg/xconq.nsi.in, and
   pkg/README_devcpp_w32.rtf were already removed — the former pair with
   tcltk/ in Step 2, the latter pair with pkg/ in the earlier repo-hygiene
   commit.) For each remaining file: what it provides, what it targets
   (which UI, which era of the Win32 API), rough state.
5. Append a short "Windows assessment (date)" note to this plan item in
   MODERNIZATION-PLAN.md: the plausible modern path (SDL2/3 UI + winsock +
   MinGW/MSVC in CI), what the existing files contribute to that (likely
   almost nothing), and a recommendation. The maintainer decides; leave the
   Win32 files in place.
Verify: fresh configure + build of both UIs, quick ctest green.
Commit part 1; mark the Classic-Mac half done in the plan item and leave
the Windows decision open with your assessment note.
```

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
  `SIZEOF_LONG` for its `Z16`/`Z32` typedefs — replace with `<cstdint>` fixed
  width types and delete the checks from `acdefs.h`.

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

**⚙ PROMPT 7.4 — recommended model: Sonnet.** *(Deletions are pre-decided
below; CONTRIBUTING.md content can be assembled from CLAUDE.md and the
plan.)*

```text
Task: Xconq repo hygiene — delete historical clutter and write
CONTRIBUTING.md.

Part 1 — deletions (git history preserves everything; deletion is safe):
1. git rm -r cvs_hist/ (2.9 MB of CVS-era history exports), changelogs/
   (1.9 MB of ancient per-year ChangeLogs), and the top-level ChangeLog.
   State in the commit message that these remain available in git history.
2. pkg/: xconq.spec.in and xconq.nsi.in reference the deleted autoconf
   build — git rm them, plus README_devcpp_w32.rtf (Dev-C++/Win32-era; if
   the §5 Windows-assessment task has not run yet, mention the deletion
   there is fine regardless — the file documents a dead toolchain).
   Inspect pkg/install (look at it first): if it is autoconf-era tooling,
   delete it too; if anything in pkg/ is still meaningful under CMake,
   keep it and say why in the commit. Delete pkg/ entirely if it ends up
   empty. Then grep -rn "pkg/\|xconq.spec\|ChangeLog\|cvs_hist\|changelogs"
   across CMakeLists.txt files, test/ scripts, doc/, README, CLAUDE.md and
   fix references.
Part 2 — CONTRIBUTING.md at repo root, covering (concise, ~1-2 pages):
   - what Xconq is (one paragraph; adapt from CLAUDE.md);
   - building: the CMake workflow, UI option flags, where executables land;
   - testing: ctest labels, what failure means (the test/common.sh policy:
     crashes always fail; game.dir games also fail on diagnostics and
     save/restore mismatch), how to run one game by hand under skelconq;
   - the .def X-macro system: why you edit kernel/*.def and never the
     expanded output, with one worked example (e.g. adding a unit-type
     property touches utype.def and shows up in GDL + saves + docs checks);
   - source conventions: everything compiles as C++ (or "sources are .cc"
     if the §1 rename has landed — check), the formatting policy if
     .clang-format exists by now (format only lines you touch);
   - where game content lives (lib/*.g) and that lib/game.dir games are
     held to a higher test bar.
   Cross-check every claim against the current tree rather than copying
   this outline blindly — some referenced tasks may or may not have landed.
Verify: fresh configure + build still succeeds (nothing in the build
referenced the deleted trees); quick ctest green.
Commit; mark this item done (strikethrough + date) in MODERNIZATION-PLAN.md.
```
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
   (CMake build, two UIs: curses (cconq) and SDL via sdl12-compat
   (sdlconq, the primary graphical client) — the Tcl/Tk and Xt/Xaw UIs
   were removed in Step 2, 7/2026; a minimal UI-section update already
   landed with that removal, but re-verify against the current tree
   rather than trusting it), quick build instructions (cmake -B build &&
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
  Cut a 7.5.0 once the SDL2/3 port lands (the Tcl/Tk UI, the plan's other
  original gate, was removed rather than ported — see Step 2), with release
  notes and a source tarball from CI (replaces the old `make distcheck`).

**⚙ PROMPT 8.3 — recommended model: Sonnet.** *(Release machinery only —
the decision to actually cut 7.5.0 stays with the maintainer.)*

```text
Task: build Xconq's release machinery so that pushing a git tag produces a
GitHub release with a source tarball and notes. Do NOT tag or cut the
release itself — the plan gates 7.5.0 on the SDL2/3 port landing (the
Tcl/Tk UI, the plan's other original gate, was removed in Step 2 rather
than ported), and that call is the maintainer's.

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

PREREQUISITES: ideally the §4 SDL2/3 port has landed (read its state
either way); the §6.2 networking review, if done, is direct input for the
web-frontend option.
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
3. §1 language cleanup through the C++17 bump — unblocks tooling and Clang.
4. §4 SDL2/3 port — the project's future graphical face, now that Tcl/Tk
   is gone.
5. §5–§8 as continuous background work.
6. §9 only if the project attracts sustained interest.
