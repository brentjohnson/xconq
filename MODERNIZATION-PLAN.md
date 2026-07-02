# Xconq Modernization Plan

A roadmap for bringing a 1987–2005 codebase up to modern standards. Items are
grouped by theme; each carries a rough effort tag (**S**mall: hours, **M**edium:
days, **L**arge: weeks). Within a section, items are ordered by suggested
sequence — earlier items unblock or de-risk later ones.

Step 0 (done, July 2026): replaced autoconf with CMake, removed the abandoned
`src/` automake rework, wired the test suite into CTest, fixed the bit-rot
needed to get all four UIs + tools compiling again, CI green on Ubuntu.

## 1. Language & toolchain

- **[M] Make the code valid without `-fpermissive`.** Fix what the flag papers
  over (implicit conversions, old-style casts of function pointers, etc.) and
  drop it. This is a prerequisite for everything below and for building with
  Clang, which lacks an equivalent.
- **[M] Fix string-literal constness and drop `-Wno-write-strings`.** The
  CMake migration already fixed ~20 `const char *` signature mismatches; a
  systematic pass (mostly in the UIs and the `.def` tables) finishes the job.
- **[S] Replace the `min`/`max` macros in `kernel/misc.h`** with inline
  functions (or `std::min`/`std::max`). This is the sole reason the build is
  pinned to gnu++98 — libstdc++ headers clash with the macros under C++17.
- **[M] Bump the C++ standard incrementally: gnu++98 → 11 → 17.** After the
  macro fix, each bump is mostly mechanical (`register`, `auto_ptr`-era idioms,
  narrowing conversions). Stop at C++17; the code gains little from newer.
- **[M] Rename `.c` files that are compiled as C++ to `.cc`** (or `.cpp`) and
  drop the `LANGUAGE CXX` override in CMake. The current arrangement confuses
  every editor, indexer, and new contributor.
- **[M] Turn on real warnings (`-Wall -Wextra`) and burn the backlog down.**
  Start with warnings-as-errors on a curated subset (return types, uninitialized
  reads, format strings) and widen over time.
- **[S] Remove bundled compat shims that modern platforms make dead code:**
  `kernel/snprintf.c` (C99), `kernel/timestuff.c` (`gettimeofday` is
  everywhere), and consider replacing `kernel/obstack.c` (bundled GNU obstack)
  with plain allocation.

## 2. Known bugs (pre-existing, found while migrating)

- **[M] `check-auto` assertion crash:** `skelconq --auto` dies in
  `do_toolup_action` (`kernel/actions.c:2284`) — the actor fails an in-play/
  completeness precondition. The autotest is disabled in CTest until fixed.
- **[M] `check-ai` infinite loop:** the AI test loops forever on
  `lib/cil-rules.g`, printing "run_game: tested for game start." until the disk
  fills (it produced a 13 GB log before being caught). Also disabled. Likely an
  AI planning loop that never consumes ACP; bisecting `cil-rules.g` against
  `cil.g` should localize it.
- **[S] Audit other library games for the same loop class** by running
  `check-ai` per-game with the CTest timeout, then re-enable the test.

## 3. Testing & CI

- **[M] Make the test scripts actually fail.** Today every `test-*.sh` exits 0
  no matter what; only harness crashes are detected. Grep the logs for
  `Error:`/`Warning:`/assertion output and return nonzero, or replace the shell
  scripts with a small CTest driver. This is the single highest-leverage
  quality investment — right now CI green means "it didn't segfault."
- **[S] Split `check-lib`/`check-actions` into one CTest per game module** so
  failures name the offending `.g` file and run in parallel.
- **[M] Add a CI matrix:** GCC + Clang, Debug + Release, and a macOS job
  (the kernel and SDL/curses UIs should be close to working there).
- **[M] Add ASan/UBSan CI jobs** running the quick test suite. A codebase this
  old will surface real bugs immediately; fix as they appear.
- **[M] Fuzz the GDL reader** (`kernel/read.c`, `kernel/lisp.c`) with
  libFuzzer/AFL. It parses untrusted input: downloaded game modules, network
  messages, and save files all go through it.
- **[S] Port the `test/*-diff.sh` / `*-uses.sh` consistency checks into CTest**
  (docs ↔ `.def` symbols ↔ library usage) so they can't silently rot.
- **[S] Add unit tests for pure-kernel logic** (GDL read/write round-trip,
  save/restore equality, movement/combat table lookups) — the current tests are
  end-to-end only.

## 4. User interfaces

- **[L] Port the Tcl/Tk UI (the primary one) to Tcl/Tk 9.** Tk 9 removed
  `Tk_Offset` and moved `Tk_ConfigureWidget` to `Tcl_Obj`-based configuration
  (~57 uses in `tkisamp.c`/`tkmap.c`, plus an API sweep). Until then the UI
  only builds against 8.x (CMake detects and skips 9.x); Fedora already ships
  only Tcl 9, so this is becoming urgent.
- **[M] Port the SDL UI from SDL 1.2 to SDL2 or SDL3.** It currently builds
  only via sdl12-compat. It was explicitly intended as the successor UI
  ("sleeker, faster replacement"), and an SDL3 port is the most plausible
  long-term cross-platform frontend.
- **[S] Decide the fate of the Xt/Xaw UI (`xtconq`).** It was dropped from the
  old configure system decades ago and only compiles again as of the CMake
  migration. Recommendation: keep it build-tested but officially deprecated,
  and delete it (plus the bundled 1990 K&R `SelFile` widget) once the SDL port
  is usable.
- **[S] Vendor-audit the bundled Tcl `BWidget` toolkit copy** in `tcltk/` —
  either update it or depend on the distro package.
- **[S] Ship an AppStream metainfo file** alongside the existing
  `xconq.desktop`/`xconq.png` so desktop stores list the game properly.

## 5. Platform behavior

- **[M] Use XDG base directories.** Scores currently want a shared, setgid-era
  `/var/lib/xconq/scores`; per-user data belongs in `$XDG_DATA_HOME/xconq`,
  config in `$XDG_CONFIG_HOME`. Drop the remaining games-group install
  assumptions.
- **[M] Decide on Windows/macOS support.** The tree carries Classic Mac OS
  (`kernel/mac.c`, `tcltk/tkmac.c`, `sdl/sdlmac.cc`) and Win32 (`kernel/win32.c`,
  `tcltk/tkwin32.c`, RC files) code that hasn't compiled in 20 years. Either
  wire Windows into CMake + CI (feasible: SDL + winsock) or delete the dead
  files — Classic Mac support should be deleted regardless.
- **[S] Large-file/64-bit sweep:** the kernel still consults `SIZEOF_INT`/
  `SIZEOF_LONG` for its `Z16`/`Z32` typedefs — replace with `<cstdint>` fixed
  width types and delete the checks from `acdefs.h`.

## 6. Security & robustness

- **[M] Unsafe string call audit:** the kernel has ~300 `sprintf` and ~70
  `strcpy` calls into fixed buffers (the bundled `snprintf.c` exists because
  the code predates C99). Convert hot paths to bounded calls; the fuzzers from
  §3 will prioritize targets.
- **[M] Review the homegrown networking layer** (`kernel/tp.c`,
  `kernel/socket.c`): it's a custom peer-to-peer protocol whose messages feed
  the GDL reader. At minimum, fuzz the message decoder; longer term consider
  whether multiplayer survives at all and behind what transport.
- **[S] Review the custom image decoders** (`kernel/gif.c`, `kernel/imf.c`,
  X11 XBM/XPM readers) — classic CVE territory. Consider delegating GIF to a
  library or restricting to trusted install dirs.
- **[S] Audit file-path handling** in save/restore and module loading
  (`fopen` of names derived from GDL input or network messages — check for
  directory traversal out of the library dir).

## 7. Code quality & developer experience

- **[S] Add `.clang-format` + `.editorconfig`** matching the existing GNU-ish
  style; format only files you touch (no big-bang reformat under active
  history).
- **[M] Stand up `clang-tidy`/`cppcheck` with a minimal profile** (bugprone-*,
  clang-analyzer-*) and fix incrementally.
- **[S] Wire up Doxygen.** The kernel headers already carry extensive `\file`/
  `\brief` comments; a `docs` CMake target + CI artifact makes them useful.
- **[S] Repo hygiene:** archive or delete `cvs_hist/`, `changelogs/`, the
  monolithic `ChangeLog`, and the stale packaging in `pkg/` (`xconq.spec.in`,
  NSIS script reference the deleted autoconf build). Add `CONTRIBUTING.md`
  documenting the CMake workflow and the `.def` X-macro system.
- **[S] Add SPDX license headers** (GPL-2.0-or-later) to source files that
  carry only prose notices.

## 8. Documentation & release management

- **[M] Modernize the manuals' delivery.** The Texinfo sources (user guide,
  designer's manual, hacking guide) are good content with no modern rendering.
  Build HTML in CI and publish via GitHub Pages; consider converting to
  Markdown/Sphinx only if the Texinfo toolchain becomes a burden.
- **[S] Update the prose docs** (`README`, `doc/INSTALL-*`): dead URLs
  (sources.redhat.com, SourceForge mailing lists), references to Mac OS 9 /
  Windows ME, and the "7.5 Prerelease" framing from 2005.
- **[S] Start tagging releases.** The version has been 7.5.0pre for 20 years.
  Cut a 7.5.0 once the Tcl/Tk 9 port or SDL port lands, with release notes and
  a source tarball from CI (replaces the old `make distcheck`).

## 9. Long-term / structural (optional)

- **[L] De-globalize the kernel.** World state, sides, and units are global;
  the engine is single-instance and non-reentrant. Wrapping state in a context
  struct would enable a test harness, engine-as-library embedding, and a
  headless server.
- **[L] Consider a modern UI built on the SDL3 port** (or a web frontend
  driven by a headless kernel over the existing network protocol) as the
  eventual successor to the Tcl/Tk interface.
- **[M] GDL ergonomics:** a syntax description for editors (tree-sitter
  grammar or TextMate bundle), plus a `skelconq --validate` mode for game
  authors, would lower the barrier for the games library — which is the
  project's real crown jewel (200+ modules).

## Suggested sequencing

1. §3 test honesty (fail on errors) + §2 bug fixes — make green mean something.
2. §1 language cleanup through the C++17 bump — unblocks tooling and Clang.
3. §4 Tcl/Tk 9 port (urgent: distros are dropping 8.x) or the SDL2/3 port,
   whichever the project wants as its future face.
4. §5–§8 as continuous background work.
5. §9 only if the project attracts sustained interest.
