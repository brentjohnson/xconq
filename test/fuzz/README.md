# Fuzzing the GDL reader

Xconq's GDL reader parses untrusted input: downloaded game modules, save
files, and the peer-to-peer network transfer protocol (`kernel/tp.cc`) all feed
bytes through the same lisp tokenizer/parser in `kernel/lisp.cc`. This directory
holds a [libFuzzer](https://llvm.org/docs/LibFuzzer.html) harness for that
surface, an opt-in build target, a seed corpus, and a GDL keyword dictionary.

## What it exercises

`fuzz_gdl.cc` drives `read_form_from_string()` — the lowest reader entry point —
over the whole input buffer, one top-level form at a time, exactly as module
loading reads a file. That covers the interesting parsing surface without
building a game world:

- strings and their backslash / octal (`\041`, `\0101`) escapes,
- `|bar quoted|` symbols,
- numbers: decimals, percents, negatives, and dice notation (`3d6+2`, `-1d8-3`),
- nested lists and `#| block |#` comments (including nesting),
- the shared `BIGBUF` token buffer.

A crash here is a real reader bug, not an init-order artifact. Interpreting the
parsed forms (`interp_form` and friends in `kernel/read.cc`) is deliberately
**not** fuzzed: it needs full world state and performs file I/O side effects
(`include`, image loading, default-game loading), so a harness over it would
mostly exercise init ordering and file opening rather than the parser. The
untrusted-bytes-to-parser surface is `lisp.cc`, and that is what this targets.

## Building and running

The harness is Clang-only (libFuzzer) and off by default. Configure with
`-DXCONQ_FUZZ=ON`:

```sh
CC=clang CXX=clang++ cmake -B build-fuzz -DXCONQ_FUZZ=ON \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo -DXCONQ_UI_SDL=OFF
cmake --build build-fuzz --target fuzz_gdl -j
```

It links a dedicated copy of the low-level reader objects
(`kernel/lisp.cc`, `util.cc`, and platform glue) built with
`-fsanitize=fuzzer,address,undefined`, plus `fuzz_stubs.cc` (fake versions of
the high-level kernel callbacks the reader references but does not need — the
same idea as `kernel/imf2imf.cc`).

Replay the checked-in corpus (also wired as the `fuzz-gdl-corpus` CTest, label
`fuzz`):

```sh
ASAN_OPTIONS=detect_leaks=0 ./build-fuzz/test/fuzz/fuzz_gdl -runs=0 test/fuzz/corpus
# or: ctest --test-dir build-fuzz -L fuzz
```

Run an actual campaign, seeded from the corpus and steered by the dictionary:

```sh
ASAN_OPTIONS=detect_leaks=0:abort_on_error=1 \
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 \
./build-fuzz/test/fuzz/fuzz_gdl \
    -dict=test/fuzz/gdl.dict -rss_limit_mb=4096 \
    -jobs=$(nproc) -workers=$(nproc) test/fuzz/corpus
```

`ASAN_OPTIONS=detect_leaks=0` is required: the kernel allocates via `xmalloc`
and frees almost nothing by design, so leak detection would fire on the initial
corpus. Use a generous `-rss_limit_mb`: the reader interns every symbol it sees
and never releases them, so memory grows slowly over a long run (bounded per
input — this is acceptable growth, not corruption). For multi-hour runs, restart
periodically or raise the limit.

When a crash is found, minimize it and add the minimized input to `corpus/` as a
regression seed:

```sh
./build-fuzz/test/fuzz/fuzz_gdl -minimize_crash=1 -runs=100000 crash-<hash>
cp minimized-from-<hash> test/fuzz/corpus/regress_<description>
```

## Corpus and dictionary

- `corpus/` — checked-in seeds: minimized regression inputs for every fixed bug
  (named `regress_*`) plus a few representative GDL snippets (`seed_*`). CI
  replays all of them and mutates from them. Grow it locally with real
  `test/*.g` and `lib/*.g` files before a long campaign; those are not checked
  in here since they already live in the tree.
- `gdl.dict` — a libFuzzer dictionary of ~1000 GDL tokens (keywords, property /
  type / table names) extracted from `kernel/*.def`. Regenerate with
  `sh gen_dict.sh` after changing the `.def` files.

## CI

The `fuzz` job in `.github/workflows/c-cpp.yml` builds the harness with Clang,
replays the corpus (the `fuzz` CTest), and runs ~5 minutes of live fuzzing over
the checked-in corpus under ASan+UBSan as a smoke test. It is non-optional.

Longer continuous fuzzing belongs in a dedicated local run or, as a future
option, [OSS-Fuzz](https://github.com/google/oss-fuzz) — the harness and corpus
here are structured to drop into an OSS-Fuzz project (single
`LLVMFuzzerTestOneInput`, seed corpus, dictionary) if that is set up later.

## Bugs found and fixed

All in `kernel/lisp.cc`, all reachable from a truncated or hostile module / save
/ network message:

- **Infinite allocation loop on an unclosed list ending in a symbol** (e.g.
  `(foo`). `strmgetc` returns EOF at a string's terminating NUL without
  advancing, but `strmungetc` decremented the pointer unconditionally, so
  pushing that EOF back re-exposed the previous character and `read_list`
  re-read it forever, consing without bound (a DoS). Fixed by making
  `strmungetc(EOF, …)` a no-op, matching stdio's `ungetc`.
- **Signed integer overflow accumulating a long digit run** (`num*10 + digit`)
  in the number reader — UB on inputs like `999999999999`. Now saturates at
  `INT_MAX`; also parenthesized `(ch - '0')` so operator precedence can't
  overflow the intermediate on a value near `INT_MAX`.
- **Signed overflow in the decimal `factor * num` step** — a large value times
  the decimal factor (up to 100) overflowed int. Now saturates.
- **Left shift of a negative value in dice bit-packing** (`(dice - 2) << 7`,
  `(numdice - 1) << 11`) — UB when the dice size/count is out of range (e.g.
  `1d`, `1d1`), which the code already warned about but then computed anyway.
  Now packs the fields as unsigned; bit-identical for valid dice.
- **Signed overflow accumulating a long octal escape** (`8 * octch + digit`) in
  `read_delimited_text` — UB on inputs like `"\04444444444"`. Only the low byte
  is kept, so it now accumulates as unsigned (defined wrap, same result).

All are behavior-preserving for valid input; the full quick ctest lane
(including `check-save`'s save/restore fidelity comparison) stays green.
