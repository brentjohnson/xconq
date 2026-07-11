# Xconq Client/Server Architecture

*Decision record and target architecture for the next major version. Written
2026-07-11. The work plan implementing this lives in MODERNIZATION-PLAN.md §10;
the wire protocol specification lives in `doc/net-protocol.md` (task 10.2).
This document is the context anchor for that work: every §10 task assumes the
reader has read this file. If implementation diverges from what is written
here, update this file in the same commit.*

## 1. Summary

Xconq becomes a client/server game. The kernel — game rules, world state, AIs,
GDL — runs **only on a server**. Clients are thin: they hold a mirror of one
side's *view* of the game, render it, and send player intents. There are two
clients (a browser client and the native SDL client) and one server binary,
deployable two ways:

- **Standalone server**: a headless daemon on a host machine; SDL clients and
  web clients connect to it over the network.
- **LAN hosting from the SDL client** ("listen server"): the SDL client spawns
  the same server binary as a local child process, connects to it over
  loopback, and other clients on the LAN — native or browser — join the same
  port.

This is a breaking change. The legacy peer-to-peer lockstep protocol
(`kernel/tp.cc`, `kernel/socket.cc`) is removed, not bridged; there is no
compatibility with pre-rearchitecture versions.

## 2. Why server-authoritative (and what it replaces)

The legacy multiplayer model was replicated-kernel deterministic lockstep:
every participant ran the full kernel with full game state, one peer was the
master, and small text packets kept the copies in sync (checksum-verified).
That model cannot reach a browser without compiling the kernel to WASM, holds
full game state on every client (fog of war is honor-system), and its wire
protocol feeds the GDL reader from untrusted peers (see
`doc/net-security-review.md`).

Server-authoritative fixes all three: browsers get a thin TypeScript client,
clients only ever receive what their side can see (fog of war becomes real),
and the kernel parses GDL only from its own library and saves. The cost is a
new protocol and a port of the SDL client off direct kernel-memory access.

The key structural asset making this tractable: **the kernel↔UI boundary is
already almost a network protocol.** Every UI implements a fixed callback
surface (canonical list: `kernel/skelconq_stubs.cc` — `update_cell_display`,
`update_unit_display`, `update_event_display`, `update_side_display`,
`update_turn_display`, `low_notify`, …), and the kernel invokes those
callbacks per side, at exactly the moments state becomes visible to that side,
already fog-of-war-filtered via the per-side view layers. The server side of
the new protocol is one more implementation of that interface — a "netui"
that serializes each callback into a delta message instead of drawing. One
enrichment is required: today's callbacks are dirty-notifications ("cell x,y
changed") after which the UI reads kernel memory; the network messages must
carry the content (the side's view of the cell, the unit-view record, the
history event).

## 3. Components

```
┌──────────────────────────── server host ────────────────────────────┐
│  game process: xconqd (one OS process per game — the kernel is      │
│  global-state, single-instance by design)                           │
│  ┌───────────┐   ┌──────────────────────┐   ┌────────────────────┐  │
│  │  kernel    │──▶│ netui: callbacks →   │──▶│ WebSocket listener │  │
│  │  + AIs     │   │ per-side deltas      │   │ + minimal HTTP     │  │
│  │  run_game  │◀──│ commands → actions/  │◀──│ (static assets,    │  │
│  └───────────┘   │ tasks (validated)    │   │  ws upgrade)       │  │
│   lib/*.g stays  └──────────────────────┘   └─────────┬──────────┘  │
│   server-side only                                    │             │
│  front door / lobby process: routes joins to game processes         │
└────────────────────────────────────────────────────────┼────────────┘
                              wss:// (TLS via reverse proxy)
              ┌──────────────────────────┬───────────────┘
              │                          │
      ┌───────┴────────┐        ┌────────┴─────────┐
      │  web client     │        │  SDL client      │
      │  web/ (TS,      │        │  renders from    │
      │  canvas)        │        │  libclientmodel  │
      └────────────────┘        └────────┬─────────┘
                                          │ LAN hosting: spawn xconqd
                                          │ child, connect via loopback;
                                          │ LAN peers join the same port
```

### 3.1 Game server (`server/`, binary `xconqd`)

Headless kernel + AIs + netui + network listeners. Single-threaded: an event
loop interleaving bounded `run_game(maxactions)` slices with socket polling —
the same pattern skelconq's free-run uses today. One OS process hosts one
game (the kernel's pervasive globals make this the honest process model; it
also isolates crashes). Serves:

- WebSocket connections (the only game transport — see §4),
- minimal HTTP: the ws upgrade handshake, plus static files (the web client
  bundle and asset atlases) so a LAN listen server is self-contained.

### 3.2 netui (server-side, per connected side)

Implements the required UI callback surface; each callback serializes the
affected state (view-filtered, content-carrying) as a delta message to the
side's connection. Also produces the **join snapshot**: the side's complete
view (terrain view layers, unit views, side state, turn/date, scores),
serialized via the same machinery the save writer uses, translated to the
wire format.

### 3.3 Command plane

Clients send intents: unit tasks/actions (move, fire, build, …), plan/standing
order changes, unit naming, side settings, end-turn/resign, chat, and
game-design commands where permitted. The choke points where player input
enters the kernel today — the `net_*` action wrappers and task-adding calls —
enumerate the command set. The server validates every command against side
ownership plus the kernel's existing action-legality checks; that is where
anti-cheat lives. Clients never possess state they cannot see.

### 3.4 Game manifest and assets

Clients never parse GDL. At join, the server sends a *game manifest*
generated from the loaded game: unit/material/terrain/advance type rosters
(names, display chars, image-family names, the per-type properties UIs
display), side roster, world geometry (hex/rect, dimensions, wrap). Images are
converted offline from `.imf`/GIF sources into PNG sprite atlases + JSON
index (task 10.9); the server (or any static host) serves them; both clients
consume the same atlases.

### 3.5 Web client (`web/`)

TypeScript + canvas, no UI framework; toolchain (node/vite) confined to
`web/`, never required to build the C++ tree. Holds a TS implementation of
the client-side view model, populated from snapshot + deltas.

### 3.6 SDL client + `libclientmodel` (`client/`)

`libclientmodel` is a C++ library mirroring one side's view, populated from
protocol messages, with accessors shaped like the kernel calls the SDL
rendering code makes today (to keep the port diff manageable). The SDL client
is ported to render from it and to send commands instead of calling into the
kernel; it stops linking the kernel entirely. `doc/client-state-inventory.md`
(task 10.1) — the audit of every kernel structure `sdl/` reads — defines the
model's required surface, and with it the protocol's state-sync schema.

### 3.7 Listen server (LAN hosting)

"Host a game" in the SDL client = spawn `xconqd` as a child process, connect
over loopback like any other client. Never in-process: the kernel is not
thread-safe, and a child process guarantees the hosted path exercises
identical server code to the standalone path. LAN peers join the advertised
port; optional mDNS/DNS-SD announcement for discovery. Browsers on the LAN
load the web client bundle *from the hosting machine* over plain `http://` —
a page served from an `https://` origin cannot open `ws://` to a LAN IP
(mixed-content blocking), so the internet-facing site cannot be the entry
point for LAN games.

### 3.8 Front door (multi-game hosting)

A small lobby/router process: lists games, creates them (spawning a game
process each), routes/redirects joining connections. Deliberately dumb; all
game logic stays in game processes. Single-game deployments (and every listen
server) run without it.

## 4. Transport and encoding

- **WebSocket only.** Browsers require it; a native ws client is trivial
  (we control both ends). One transport, one listener, one protocol — no
  parallel "native TCP" mode.
- **JSON text messages** in v1, one protocol message per ws message (ws
  framing already delimits; no custom escaping layer). Debuggability wins
  until profiling says otherwise; a binary encoding (CBOR) can be added later
  behind the same message schema. Turn-based traffic is small.
- A `hello`/`welcome` handshake carries a protocol version; mismatch is a
  clean rejection. No cross-version compatibility promises pre-1.0.
- Implementation: minimal in-tree RFC 6455 server (handshake = SHA-1 +
  base64; frame codec is small) plus minimal HTTP/1.1 GET for static files —
  no heavyweight network dependency. JSON via a vendored single-header
  library (nlohmann/json, MIT) under `server/vendor/`, server/client-model
  side only; the kernel itself gains no new dependencies.
- TLS is out of scope in-process: public deployments terminate `wss://` at a
  reverse proxy (nginx/caddy). LAN games run plain `ws://`.

## 5. Protocol planes (spec: `doc/net-protocol.md`)

1. **Session/lobby**: hello/version, join/rejoin (session token), game list &
   create (front door), variant settings, side assignment, chat.
2. **State sync (server→client)**: join snapshot, then deltas from netui
   callbacks (cell view, unit view, side, turn, clock, history events,
   notices). History events double as the client's message log.
3. **Commands (client→server)**: the §3.3 intent set; every command carries
   the acting unit/side and is validated server-side.

## 6. Security model

- The kernel and GDL reader only ever parse server-local files (library,
  saves). Untrusted input surface = the ws/HTTP listener and the JSON command
  parser; commands are validated against ownership and legality.
- MODERNIZATION-PLAN.md §6.4 (path-traversal audit of file opens driven by
  GDL/save content) is a hard prerequisite for running a public server.
- Sessions are bearer tokens issued at join, required for rejoin. Accounts/
  identity beyond that are out of scope for v1.
- `tp.cc`/`socket.cc` (no auth, feeds the GDL reader from peers) are removed
  by this work (task 10.17); until then legacy multiplayer remains
  trusted-LAN-only per `doc/net-security-review.md`.

## 7. Persistence, reconnect, spectators

Server-side saves make games durable across restarts (the save machinery is
kernel-side and unchanged); with turn deadlines this yields asynchronous
"connect, take your turn, leave" play. Reconnect = present token, receive a
fresh snapshot. Spectators = a connection bound to an observer view. Turn
deadline/AFK policy is server-enforced.

## 8. What is removed

- `kernel/tp.cc`, `kernel/socket.cc`, and every host/join/download code path
  of the legacy protocol, including the SDL client's use of them and the
  `net_*` broadcast layer's remote legs (the local dispatch these wrappers do
  stays, as the seam the command plane calls into).
- The in-process rendering coupling: the SDL client stops linking the kernel.

skelconq stays: it is the kernel test harness (whole `check-*` suite) and the
closest ancestor of `xconqd`'s event loop.

## 9. Risks / open questions

- **State-surface completeness** is the top risk: the callback list is the
  skeleton, but UIs also read kernel memory directly (own units' plans/tasks,
  agreements, scorekeepers, doctrine…). Task 10.1's inventory exists to bound
  this before the protocol is frozen; expect a long tail anyway.
- **Kernel blocking assumptions**: places where the kernel expects a
  synchronous UI answer (modal queries) need an async request/response
  message pair; inventory in 10.1.
- **Protocol churn** while both clients are in flight — mitigated by doing
  the web client first (thinnest consumer) and keeping the headless test
  client (10.8) as the protocol's reference consumer in CI.
- **SDL port size**: `sdl/` touches kernel state pervasively; the port is
  deliberately split into read-path and command-path tasks and may span many
  sessions.
- Game-design/editor mode over the wire is deferred (v1 exposes it only where
  it maps to existing commands); full remote map editing is future work.
