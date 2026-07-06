# Security review: Xconq multiplayer networking layer

*Date: 2026-07-05. Scope: `kernel/tp.cc` (transfer protocol) and
`kernel/socket.cc` (Unix/Winsock transport). Threat model: a **hostile
peer** — any program that completes the TCP handshake and speaks the
protocol. Deliverable is primarily analysis; only clear, local
memory-safety bugs were fixed in place (behavior-preserving). Architectural
issues are documented here, not changed.*

This item stays **open** in MODERNIZATION-PLAN.md §6 until the message-decoder
fuzzing from §3 lands — see the recommendation at the end.

---

## 1. How the protocol works

### 1.1 Transport (`socket.cc`)

Xconq multiplayer is peer-to-peer with one **master** (the host, remote id
`rid` 1) and up to `MAXSIDES-1` clients. Transport is a raw TCP stream (or a
serial device if the method string has no `:`), one socket per peer:

- `open_remote_connection(method, willhost)` — host side `bind`/`listen`s on
  `INADDR_ANY:port`; client side `connect`s to `host:port` (retrying
  `ECONNREFUSED`). `TCP_NODELAY` + keepalive are set.
- `accept_remote_connection()` — called when the listening socket is readable;
  `accept`s, assigns the next `rid` (`nextrid++`), stores the fd in
  `remote_fd[rid]`.
- `low_send(rid, buf)` — `write()`s a NUL-terminated C string in full.
- `low_receive(&rid, buf, maxchars, timeout)` — `select()`s across the public
  socket and all peer fds, then `read()`s up to `maxchars` bytes from the
  ready peer and NUL-terminates. **There is no length prefix on the wire** —
  the transport delivers arbitrary stream fragments and the framing lives
  entirely in the application layer.

`remote_fd[]` / `fd_valid[]` are fixed 100-entry arrays; `remote_player_specs[]`
/ `online[]` in `tp.cc` are `MAXSIDES` (= 15) arrays.

### 1.2 Framing (`tp.cc`)

A packet on the wire is:

```
$  <escaped body>  ^  <hex csum hi>  <hex csum lo>
```

`STARTPKT='$'`, `ENDPKT='^'`, `ESCAPEPKT='!'`. Any `$`/`^`/`!` inside the body
is escaped to a two-byte sequence (`!%`, `!&`, `!@`). The checksum is an 8-bit
additive sum of the body, in two hex nibbles. `send_packet()` builds this form;
`receive_data()` reassembles it.

`receive_data()` accumulates raw `read()` fragments into a single
`packetbuf` (`PACKETBUFSIZE` = 1000 bytes) with `strcat`, scans for a complete
`$…^xx`, then **de-escapes the body into `rsltbuf`**, recomputes and compares
the checksum, `remove_chars()`-shifts the packet out of `packetbuf`, sends a
one-byte `+` ack, and hands the decoded body to `receive_packet()`. A bare `+`
in the stream clears `expecting_ack`. The checksum is *error detection only* —
a mismatch logs a warning and the packet is still processed (`pktcsum != 0`
guard). **Nothing here authenticates the peer or integrity-protects the body.**

### 1.3 Dispatch (`receive_packet`, `tp.cc:2976`)

The decoded body's first byte selects a handler (`switch (buf[0])`,
`tp.cc:3004`):

| tag | handler | effect |
|-----|---------|--------|
| `A` | `receive_action` | queue a unit action (`atype`, args from wire) |
| `B` | `receive_bugoff` | we were bounced |
| `C` | `receive_command` | wake/designer commands (5 int args) |
| `E` | `receive_error` | dump checksums |
| `M` | `receive_net_message` | side-to-side message text |
| `P` | `receive_player_prop` | add/assign player |
| `Q` | `receive_quit` | mark `online[rid]=FALSE` |
| `R`/`X` | randstate / `run_game` | drive turn execution |
| `S` | `receive_side_prop` | set ~30 side properties incl. names |
| `T` | `receive_task` | add a task to a unit's plan |
| `U` | `receive_unit_prop` | set ~15 unit properties incl. name |
| `V` | version check | |
| `W` | `receive_world_prop` | `paint_*` terrain/layer edits |
| `Z` | `receive_game_checksum` | sync check |
| `a`/`v` | assignment / variant setting | |
| `c` | `receive_chat` | chat text |
| `f`/`g` | **module download** — load a file / library module, or begin streaming a module body |
| `j` | join request | |
| `p`/`r` | remote-program announce / my rid | |
| `s` | start load stage | |
| `w` | **`save_game(name)`** — write a save file |

If we are the master and the handler leaves `should_rebroadcast` set, the
master **re-sends the received packet verbatim** to every other peer
(`broadcast_packet(buf)` at the bottom of `receive_packet`). This is important
for the memory-safety analysis: a body that arrives on `rsltbuf` is fed *back*
into `send_packet` — see F1/F7 below.

### 1.4 Module download

To bring a new peer's world state in sync, `download_game_module()` sends
either the *filename* of a saved game (`f <path>`), the *name* of a library
module (`g <name>`), or — in the currently-disabled `else` branch — the module
serialized as a stream of `~200`-byte packets bracketed by `gameModule` …
`\neludoMemag\n`. On the receive side (`receive_packet`, `downloading` branch)
the streamed body is `strcat`ed into `downloadbuf` (a 200 000-byte heap buffer,
length-guarded) and, at the end marker, handed to the GDL reader:

```
mainmodule->contents = downloadbuf;      // tp.cc:2986
open_module(mainmodule, FALSE);
read_forms(mainmodule);                  // tp.cc:2989  → kernel/read.cc + lisp.cc
```

## 2. Trust boundary (prose)

```
  hostile peer  ─TCP─▶  low_receive()             [socket.cc: raw read into buf]
                          │
                          ▼
                        receive_data()            [tp.cc: reassemble in packetbuf,
                          │                         de-escape into rsltbuf, checksum]
                          ▼
                        receive_packet()          [tp.cc: switch(buf[0]) dispatch]
             ┌────────────┼───────────────────────────────┐
             ▼            ▼                                ▼
     receive_* handlers   'f'/'g'/download                'w' save_game
     (parse ints/strings, │                                │
      call net setters,   ▼                                ▼
      index arrays)   read_forms()  ── GDL reader ──▶  fopen()/fwrite() of a
                      load_game_module()  (read.cc,    peer-controlled path
                          get_game_module()  lisp.cc)
```

Everything to the right of `low_receive` runs on bytes the peer chose. The two
hardest boundaries to cross safely are:

1. **The reassembly/de-escape buffers** (`packetbuf`, `rsltbuf`) and the
   **re-broadcast** path back through `send_packet` — pure memory safety, and
   the site of the most serious bug found (F1/F7).
2. **The GDL reader** (`read.cc`/`lisp.cc`) reached via module download and the
   `f`/`g` handlers. This is a large hand-written recursive-descent parser. It
   is the *real* attack surface and is deliberately out of scope for
   line-level fixes here — it is what the §3 fuzzing work must cover.

There is **no authentication and no integrity protection**: any peer that
connects can drive every `net_*` mutator, request saves, and (via `f`/`g`)
name a file for this host to open.

## 3. Findings

Severity reflects a hostile-peer threat model. "Fixed" = a local,
behavior-preserving change committed with this review; legitimate traffic is
byte-for-byte unaffected (the added guards only reject values no correct peer
sends). "Deferred" = architectural/behavioral, documented for the design work.

### FIXED

**F1 — Critical — heap buffer overflow de-escaping a packet body**
`kernel/tp.cc` `receive_data` (alloc `tp.cc:2796`, loop `tp.cc:2816`).
`rsltbuf` was `xmalloc(BUFSIZE)` = **255 bytes**, but the de-escape loop copies
the body out of `packetbuf`, which is **`PACKETBUFSIZE` = 1000 bytes** and is
filled by concatenating raw `read()` fragments. A hostile peer simply sends a
packet whose body (between `$` and `^`) is longer than 255 bytes — up to ~996 —
and the loop writes hundreds of bytes past the end of the 255-byte heap
allocation. Fully attacker-controlled contents and length ⇒ classic heap
overflow / RCE primitive. *Fix:* allocate `rsltbuf` at `PACKETBUFSIZE` (de-escaping
only ever shrinks the data, so it can no longer overflow) and add a defensive
`j < PACKETBUFSIZE - 1` bound in the loop.

**F7 — Critical — undersized outgoing packet buffer (overflow + relocation of F1)**
`kernel/tp.cc` `send_packet` (`tp.cc:2582`). The local framing buffer was
`char buf[BUFSIZE]` = 255, but framing writes `2*strlen(inbuf) + 5` bytes
(escaping can double every body byte). Two ways to overflow it: (a) any body
above ~125 special-char bytes; (b) more importantly, the master **re-broadcasts
received packets** — `broadcast_packet(rsltbuf)` → `send_packet(rsltbuf)` — so
after F1 enlarged `rsltbuf`, a large hostile packet would merely move its
overflow from `rsltbuf` into `send_packet`. *Fix:* size the buffer at
`2*PACKETBUFSIZE + 8` (moved the `PACKETBUFSIZE` `#define` to the top of the
file so it is in scope) and refuse any `inbuf` ≥ `PACKETBUFSIZE` up front rather
than truncating a control packet. F1 and F7 must land together.

**F2 — High — out-of-bounds array write from a wire-supplied `rid`**
`kernel/tp.cc` `receive_remote_program` (`tp.cc:3898`) and `receive_quit`
(`tp.cc:3403`). Both do `rid = strtol(wire)` and then `online[rid]` /
`remote_player_specs[rid]` (both `MAXSIDES` = 15 arrays); `receive_remote_program`
also seeds `numremotes = max(rid, numremotes)`, and `numremotes` later bounds
`for_all_remotes` loops that index `remote_fd[]`. An out-of-range or negative
`rid` is an OOB write and can poison every later peer iteration. *Fix:* reject
`rid < 1 || rid >= MAXSIDES`.

**F3 — Medium — out-of-bounds `assignments[]` index from the wire**
`kernel/tp.cc` `receive_assignment_setting` (`tp.cc:3969`, and the re-parsed
`remove` index at `tp.cc:3977`). `n = strtol(wire)` indexes
`assignments[MAXSIDES+1]` directly (the `advantage` branch:
`assignments[n].player`) and is passed to UI callbacks that do the same. *Fix:*
bound `n` to `[0, MAXSIDES]` at parse time and again for the `remove` branch's
independently-parsed index.

**F4 — Medium — out-of-bounds `variants[]` index from the wire**
`kernel/tp.cc` `receive_variant_setting` (`tp.cc:3945`). `which = strtol(wire)`
flows into `set_variant_value` → `mainmodule->variants[which].newvalues[...] =`,
an OOB write; `variants` is a heap array terminated by an `id == lispnil`
sentinel with no bound check. *Fix:* count the module's actual variants and
reject out-of-range `which` (NULL-guarded).

**F5 — Medium — NULL deref + uninitialized read on a malformed `C` packet**
`kernel/tp.cc` `receive_command` (`tp.cc:3212`). `argstr = strchr(str, ' ')`
returns NULL for a `C` packet with no space, and the following
`while (*argstr …)` dereferences it — a remote crash (DoS). Additionally `args[]`
was read uninitialized in branches that supply fewer than five integers. *Fix:*
NULL-guard `argstr` and zero-initialize `args[]`.

**F6 — Low — unbounded `nextrid` in the accept path**
`kernel/socket.cc` `accept_remote_connection` (`socket.cc:317`). `nextrid`
grows once per accepted connection and indexes the fixed 100-entry
`remote_fd[]`/`fd_valid[]`; nothing caps it, so a host that accepts enough
connections writes out of bounds. *Fix:* refuse connections once `nextrid`
reaches the array size (close the fd, warn).

### DEFERRED (architectural / behavioral — not changed)

**D1 — High — a peer can make this host open an arbitrary file.**
`receive_packet` case `f` (`tp.cc:3091`) does
`module->filename = copy_string(buf + 2); load_game_module(...)`; case `g`
(`tp.cc:3107`) does `get_game_module(buf + 2)`; case `w` (`tp.cc:3162`) does
`save_game(buf + 1)`. All three take a peer-controlled path/name. A malicious
host can therefore make a joining client **read and GDL-parse an arbitrary
local file** (path traversal, e.g. `f /etc/passwd` or a crafted module) or
**write** a save to a chosen name. This is inherent to the "send a filename
instead of the bytes" download design and needs a *containment policy*
(canonicalize and confine to the library/save dirs, or drop filename-based
download in favor of the streamed form) — a decision, not a local patch, so it
is deferred. It overlaps the §6 file-path-handling item (PROMPT 6.4).

**D2 — High — untrusted bytes reach the GDL reader.**
Module download feeds `downloadbuf`/named modules straight into `read_forms` /
`load_game_module` (`tp.cc:2986–2989`). The GDL parser (`read.cc`, `lisp.cc`)
was written for trusted local files and is the dominant attack surface. No
local fix is appropriate; this is exactly what the §3 GDL-reader fuzzing
covers, and it is why this plan item stays open. (The reader already grew a
libFuzzer harness in §3; extending a corpus/harness to the *packet decoder* and
the download assembler is the natural next step.)

**D3 — Medium — no authentication or integrity.**
The additive 8-bit checksum is error detection, not security, and mismatches
are logged-but-processed. Any peer completing the TCP handshake is fully
trusted. Kernel state desync between peers is tolerated by design. Addressing
this means a transport/auth decision (see recommendation).

**D4 — Low — wire integers passed to kernel setters that self-validate.**
Several handlers hand raw wire integers to kernel routines
(`exchange_players`, `rename_side_for_player`, coordinates into `paint_*`,
task/goal args). `side_n`, `find_unit`, `find_player` are safe linear searches
(return NULL on miss), and the direct-index sinks were fixed above, but the
deeper setters rely on their own bounds checks. These belong to the fuzzing
sweep rather than spot fixes.

**D5 — informational — `broadcast_action`'s `tprintf` append into `buf[BUFSIZE]`**
(`tp.cc`, `broadcast_action`). Bounded in practice (all-integer content, ≲ 90
bytes) and driven by local action data, not the wire; left as-is, noted for
completeness.

## 4. What was fixed vs deferred

| ID | Severity | Status | Site |
|----|----------|--------|------|
| F1 | Critical | **Fixed** | `tp.cc:2796,2816` rsltbuf de-escape overflow |
| F7 | Critical | **Fixed** | `tp.cc:2582` send_packet framing buffer |
| F2 | High | **Fixed** | `tp.cc:3403,3898` wire `rid` → `online[]`/`numremotes` |
| F3 | Medium | **Fixed** | `tp.cc:3969,3977` wire `n` → `assignments[]` |
| F4 | Medium | **Fixed** | `tp.cc:3945` wire `which` → `variants[]` |
| F5 | Medium | **Fixed** | `tp.cc:3212` `receive_command` NULL/uninit |
| F6 | Low | **Fixed** | `socket.cc:317` unbounded `nextrid` |
| D1 | High | Deferred | `tp.cc:3091,3107,3162` peer-named file open/save |
| D2 | High | Deferred | `tp.cc:2986` download → GDL reader (fuzz in §3) |
| D3 | Medium | Deferred | no auth/integrity (transport decision) |
| D4 | Low | Deferred | wire ints into self-validating setters |

## 5. Verification

- **Build:** all UIs (`skelconq`, `cconq`, `sdlconq`) build clean with the
  changes (the only warnings are pre-existing: an unused-variable in
  `download_game_module`, a pre-existing `size_t`/`long` sign-compare on the
  reassembly guard line, and an unused `receive_bugoff` parameter — none
  introduced here, none fatal).
- **Tests:** the quick lane is green — `ctest --label-exclude long` →
  **559/559 passed**, including `check-save` fidelity (no save regressed).
- **Handshake smoke test (partial, honest result):** `skelconq` (the headless
  test driver) does **not** wire up host/join at all; only the interactive UIs
  do. Hosting via `cconq -g standard -host :<port>` was confirmed to `bind` +
  `listen`, and a second `cconq … -join localhost:<port>` reaches
  `ESTABLISHED` in both directions and transmits a well-formed framed join
  packet (the host's socket shows the 8-byte `$jxxx^NN` queued) — exercising
  the `socket.cc` setup/accept region and the rebuilt `send_packet` framing
  path with **no crash, no "packet too large" drop, and no checksum error**.
  The full `j → r → p` handshake could **not** be observed to completion
  headlessly: an idle `cconq` host blocks on terminal input and does not poll
  the socket without keypresses, and driving two curses UIs through a pty is
  out of scope for this analysis task. So: connection setup and the outgoing
  frame path are verified live; end-to-end game sync after the fixes is
  verified only by construction (every change is a buffer enlargement, a
  reject-out-of-range guard, or a NULL guard — none alters bytes on the wire
  for well-formed traffic) plus the full test suite.

## 6. Recommendation on the open question

**Do not keep the multiplayer layer as-is for use over an untrusted network,
and do not treat the F1–F7 fixes as making it safe to expose.** The fixes
remove the memory-corruption primitives that were reachable *before* the GDL
reader, but two structural problems remain by design: any peer that connects is
fully trusted (D3), and a peer can steer this host into opening arbitrary files
and into the un-hardened GDL parser (D1, D2).

Recommended path, in order:

1. **Fuzz-and-harden the decoder (near term, keeps this item open).** Add a
   libFuzzer/AFL harness that drives `receive_data`/`receive_packet` on raw
   byte streams (reusing the §3 GDL harness for the download → `read_forms`
   leg). This is the cheapest way to shake out the remaining decoder bugs the
   spot-fixes above did not reach (D4), and it is the gating work for closing
   this plan item.
2. **Contain the file-open surface (near term).** Implement the D1 policy:
   canonicalize `f`/`g`/`w` names and confine them to the library/save
   directories, or remove filename-based download in favor of the streamed
   module body. Coordinate with PROMPT 6.4.
3. **Long term, wrap rather than re-secure.** The protocol has no framing
   length prefix, no auth, no integrity, and tolerates state desync. Rather
   than retrofit security into it, run it **only over a trusted channel** —
   loopback / LAN between cooperating players, or tunneled over SSH/TLS
   (a stunnel-style relay) — and document multiplayer as "trusted peers only."
   A full protocol redesign is not worth it for a hobby strategy engine; a
   thin trusted-transport wrapper plus the decoder hardening above is the
   right cost/benefit.

Until at least (1) and (2) land, the honest posture is: **multiplayer is for
trusted peers on a trusted network.** This item remains open in
MODERNIZATION-PLAN.md §6 pending the §3-style decoder fuzzing.
