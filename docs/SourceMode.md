# Source Mode

Source mode is the intended primary workflow for working on the engine and
editor: you edit code, build, and the editor **restarts into the new binaries
while restoring your work**, so the edit→build→run loop feels continuous even
though the process genuinely restarts each time.

This document describes the mechanism and the decisions behind it. It is a
design spec, not an implementation guide — pieces marked **(deferred)** are not
yet decided in detail. Today only the cold-start path is implemented; the
in-session reload runtime (IPC, second slot, serialize/restore) is deferred.

---

## Core decisions

### Reload model: serialize state, restart the process

On reload the editor **serializes its state, restarts the whole process, and
restores** — it does **not** hot-swap dynamic libraries inside a live process.

Why full restart rather than in-process dylib reload:

1. The shipped editor lets end-users switch between the **debug** and **release**
   editor at runtime (the reason both are shipped). You do not want to swap one
   C++ runtime for another (debug ↔ release) inside a single live process.
2. `Editor` and `Engine` are **load-time (statically) linked** into the
   `Launcher` executable, so modern C++ can cross the `Launcher` `main.cpp` ↔
   `EditorMain` boundary directly, rather than through a `dlopen` + C-ABI plugin
   seam. Picking up a rebuilt load-time-linked library requires a fresh process.

### The serialize/relaunch/restore core lives in the editor — there is no separate supervisor

The user-facing **debug ↔ release runtime switch** is mechanically the *same
operation* as a source-mode reload: *serialize state → relaunch a (possibly
different) binary → restore state*. Because that switch is a shipping feature, the
serialize/relaunch/restore core must live **in the editor itself**.

Once it lives there, the editor can **relaunch itself**, so there is **no
separate "Reloader" / supervisor process**. Source-mode reload is just that
same mechanism, triggered by a dev-only **signal from the run task** (see below)
instead of a menu item:

- The mechanism (serialize/relaunch/restore) is shipping code, reused by two
  callers: the user-facing switch, and a dev-only reload signal.
- Day-to-day there is exactly **one binary** (`Launcher`), which relaunches
  itself.

### No crash auto-recovery

An outliving supervisor's only unique benefit would be auto-restart after a
crash. This is **explicitly out of scope**, because it has no value here:

- When the engine crashes during development, the developer is already watching
  and immediately starts troubleshooting.
- For an end-user crash, an **emergency serialization performed during the
  crash** is restored on the next *manual* relaunch (with a prompt to report the
  crash). Users always relaunch after a crash anyway.

This removes the last reason to have a second long-lived process.

---

## The Windows locked-DLL problem

Windows is the primary development platform, so this is a first-class concern,
not an afterthought.

On Windows, a load-time-linked DLL is memory-mapped and **locked against
overwrite while the process runs**. Since `Engine.dll`, `Editor.dll`, and
`AdHocEditor.exe` are all mapped by a running editor, a naive inner loop is
impossible:

> edit → Build → the linker tries to write `build/Engine.dll` → the running
> editor has it mapped → **sharing violation → build fails.**

This happens on *every* rebuild, not occasionally. The hard precondition it
imposes: **the running editor must not have any `build/` file mapped.**

(macOS does not have this lock — a mapped dylib can be overwritten because the
running process keeps its old inode. We still run from a copy on macOS for a
single code path; see below.)

### Solution: run from a copy ("slots"), never from `build/`

The editor always executes from a **copy** of the runnable file set located
outside the build tree, in a fixed directory called a **slot**. Then `build/` is
never locked and rebuilds always succeed.

There are exactly **two** slots, **A** and **B**, used symmetrically and reused
in place (never accumulating copies on disk). The editor runs from one; the
other is always free. On reload the new build is staged into the **free** slot
and the successor runs **directly from where it was staged** — so the slots
simply ping-pong A ↔ B and **no process ever copies itself**:

| Step | Editor runs from | Free slot (next stage target) |
|---|---|---|
| Cold start | A | B |
| 1st reload | B | A |
| 2nd reload | A | B |

The only copy in the whole flow is the unavoidable `build/` → slot **stage**;
there is no second "relocate" copy. This is what keeps the reexec count at the
floor (macOS reload: one reexec, for mimalloc injection; Windows reload: zero).

Why two symmetric slots rather than three fixed ones: because the new build is
staged into *whichever slot the editor is not in*, the running slot is never the
stage target, so it is never locked against staging — and there is never a need
to reserve a separate cold-start slot or to evict a running editor from a pinned
slot. Two slots suffice with no deadlock (the editor holds one; the other is
always free).

Each slot copy must include **debug symbols** (`.pdb` on Windows, alongside the
DLLs) or debug sessions come up symbol-less.

---

## Single-instance guard and the reload signal

The editor is a single-instance application. On startup it acquires a **named
lock** (named mutex on Windows; lockfile / `flock` on macOS) and **listens on a
well-known IPC endpoint** (an `AF_UNIX` socket on macOS; a named pipe on
Windows). The lock is held by the process, so it auto-releases on crash (the
stale slot files remain and are simply overwritten next time).

### Connect-as-liveness-probe

The reload trigger is **not** a filesystem watch thread. The run task that
builds the code (see [IDE wiring](#ide-wiring-build--branch--launch)) decides
*cold start vs reload* by trying to **connect** to that endpoint:

- **Connect succeeds** → an editor is alive and listening. The run task sends
  `reload` on that same connection and is done. The connect *is* the liveness
  proof and it carries the message, so there is **no TOCTOU window** — there is
  no gap between "check if alive" and "deliver" in which the editor could exit
  and the signal be lost.
- **Connect refused / endpoint absent** → no editor is running → **cold start**:
  the run task stages slot A and launches it itself. A stale socket node from a
  crashed editor also yields "refused" (nobody is accepting), which correctly
  reads as cold start.

A post-build signal is an unambiguous "build is done **now**" — unlike a
filesystem watcher, which must debounce a storm of intermediate object/link
writes and *guess* when a build has settled. That fragility is the main reason
the watch thread was dropped.

### Three startup scenarios

The named lock distinguishes the *legitimate* reload handoff from *accidental*
duplicate launches:

1. **No instance running** → acquire the lock, bind the endpoint, start normally
   (this is the cold-start launch).
2. **Instance already running, launched fresh** (e.g. an accidental double-run
   that the connect-probe could not foresee) → the lock is held → **focus the
   existing window and exit immediately.** No second instance appears.
3. **Reload successor** → the *running* editor itself spawns the successor (see
   the reload flow), so the successor is sanctioned by construction. It waits for
   the predecessor to release the lock, then acquires it and restores state. The
   waiting is what makes the handoff race-free.

The connect-probe keeps duplicates from being spawned in the common case; the
named-lock guard (scenario 2) is the correctness backstop for genuine races
(e.g. the run task fired twice).

---

## Flows

### Cold start (everyday Run/Reload, no editor running)

1. The run task builds into `build/` and stages it → slot **A** (the
   `source-mode` target; always succeeds — no running editor maps `build/`).
2. The run task connect-probes the editor endpoint → **refused** (nobody home).
3. The run task launches `A/<editor>` **detached, without a debugger**, passing
   `--source-mode --build-tree <build/…> --slot-root <out/source>`. The Launcher
   self-injects mimalloc by re-exec'ing itself (the normal production path; see
   [Debugging](#debugging) for why this differs under a debugger). The detached
   editor's stdout/stderr go to `out/source/editor.log`, **not** the launching
   terminal — once the run task exits it closes that pipe, and an editor still
   writing to it would abort on the broken-pipe write (`fmt::println` throws).

### Reload (in-session) — **deferred runtime**

1. The run task builds into `build/` and connect-probes the endpoint →
   **connected** → sends `reload`, then exits. (The build itself is unaware of
   the reload mechanism; it just writes `build/`.)
2. The running editor receives `reload` and **prompts** the user (reload is never
   forced mid-edit).
3. On accept, the editor stages `build/` → the **free** slot and **spawns the
   successor there**, which comes up immediately and shows a "restoring previous
   session…" handoff UI while it **waits on the lock**.
4. The editor serializes its state, releases the lock, and exits.
5. The successor acquires the lock, binds the endpoint, and restores state. It
   runs **directly from the staged slot** — no self-relocate.

Because the successor is spawned only **after** the user accepts, a **declined**
reload leaves no orphan process; because it is spawned **before** the predecessor
fully exits, it can show handoff progress.

### Accidental Run while the editor is already running

Two layers protect this:

- **Connect-probe:** the run task connects, finds a listener, and treats the run
  as a reload signal rather than a fresh launch — so no duplicate is spawned in
  the common case.
- **Single-instance guard (backstop):** if a fresh process is launched anyway
  (e.g. the run task fired twice and raced), it fails to acquire the lock,
  focuses the existing window, and exits (scenario 2).

Either way: no duplicate, and the already-running editor still reloads correctly
off the next signal.

---

## Debugging

The reframe that makes the moving slot location a non-issue: **a debugger
attaches to a *process*, not a *path*.** The editor's image *name* is stable
regardless of which slot it runs from.

Debug-from-`main()` is a **separate, rare flow** — its own launch configuration,
not part of the everyday Run/Reload button (see [IDE wiring](#ide-wiring-build--branch--launch)).
It is inherently a cold-start act (launching a debugged process while one is
already running just focuses the existing window).

- **Launch under debugger (cold start):** the launch step staged a known, fixed
  slot (A), so it hands the debugger that exact path. Deterministic; gives a
  debugged session from `main()` onward.
- **Reattach after a reload:** the successor runs in a different slot, outside
  the debugger. Reattach via "Attach to Process" and pick the editor by **name**
  — its directory is irrelevant. Manual reattach is acceptable because most
  editor work runs with no debugger attached.

**Debug engine (deliberate): CodeLLDB on both platforms.** VS Code cannot vary a
launch config's debugger `type` per-OS — only `program`/`env`/`cwd`/`args` go in
the `osx`/`windows` override blocks. So a *single* debug-from-main config across
macOS and Windows requires one engine on both, and we use **CodeLLDB (`lldb`)**
everywhere (already the macOS engine).

| Engine | Reattach | Symbols (PDB) |
|---|---|---|
| CodeLLDB (`lldb`) | manual process-pick (or `"waitFor": true` to auto-grab the reload successor) | historically rougher on Windows |
| `cppvsdbg` (VS C++ debugger) | manual pick each reload | best fidelity, but Windows-only |

Trade-off accepted for the single-config win: Windows symbols go through CodeLLDB
rather than cppvsdbg. Revisit (re-add a Windows-only `cppvsdbg` entry) if Windows
debugging gets rough. CLion uses its own bundled LLDB-based debugger, unaffected.

**macOS mimalloc under the debugger (deliberate):** the Launcher normally makes
mimalloc the allocator by re-exec'ing itself with `DYLD_INSERT_LIBRARIES` set —
but re-exec'ing a *debugger-traced* process with that variable set faults inside
dyld on macOS 26 (`EXC_BAD_ACCESS` in `dyld4::ExternallyViewableState`, then
SIGKILL — the symptom is a launch that dies immediately with "exited with code
9"). So the debug-from-`main()` launch config **preloads** mimalloc via
`DYLD_INSERT_LIBRARIES`, pointed at the stable `out/source/libmimalloc-adhoc.dylib`
symlink the `source-mode` step stages (config-agnostic, regenerated each launch),
which makes the re-exec unnecessary. If a debugger is attached *without* that
preload, the Launcher detects the trace and runs on the system allocator for
that session rather than crashing. The everyday (un-traced) Run/Reload flow takes
the normal re-exec path and needs no preload. See
`Launcher/src/_platform/Mac/MacMimallocInjectionImpl.cpp`.

---

## Relationship to the build system

- The "runnable copy in a slot" *is* the relocatable launch layout produced by
  the `source-mode` / `SourceMode` install component (see
  `cmake/AdHocMacBundle.cmake`, `cmake/AdHocWindowsInstaller.cmake`). On Windows
  this assembly step is **load-bearing**, not redundant with the build: it
  produces the unlocked copy that lets rebuilds succeed while the editor runs.
- The same `SourceMode` component is reused by `scripts/package.py` to stage each
  `(platform, config)` tree, then merge `Debug` + `Release` (Debug binaries get a
  `D` suffix) into one shipping bundle. The merged bundle — both editors
  co-located — is exactly what the user-facing debug ↔ release runtime switch
  consumes.
- Heavy, config-independent assets are **referenced in place, not staged into
  slots.** The pinned LLVM toolchain is multi-GB, so copying it per slot (and
  again on every ping-pong reload) is untenable; in source mode the editor
  reaches back to the repo's `tools/llvm/current` — the stable, version-agnostic
  handle created at bootstrap (`scripts/bootstrap_llvm.py`). This is the seam
  between source mode (reference the repo) and a shipping bundle, which **must**
  copy everything in to stay standalone (`package.py` embeds the toolchain under
  `Resources/llvm`). The editor's build system for standalone games invokes LLVM
  through this handle; other large/shared resources may later follow the same
  reference-in-source-mode, copy-when-shipping rule.

---

## IDE wiring (build → branch → launch)

The slots live under the already-gitignored `out/source/`. Because the editor is
single-instance, the run slots are **global**, not per-preset:

```
out/source/a/     slot A — cold start stages + launches here
out/source/b/     slot B — reload ping-pong target (runtime-written; not used yet)
```

Staging a slot is the `source-mode` target: it builds `Launcher`/`Engine`/
`Editor` and installs the single-config `SourceMode` layout. The `source-mode`
target currently always stages slot **A** (the cold-start slot); editor-side
staging of the free slot on reload is deferred runtime work.

```sh
cmake --build build/<preset> --target source-mode
```

### Three triggers, one orchestration script

Each core action sits on a hotkey that ships with the repo — **no manual
keybinding** (VS Code has no workspace-committed `keybindings.json`):

| Action | Trigger | Debugger | What it does |
|---|---|---|---|
| **Run/Reload** (everyday) | **Cmd/Ctrl+Shift+B** (default build task) | no | build → connect-probe → reload **or** (stage A → cold-launch) |
| **Debug from main()** (rare) | **F5** → *Editor - debug from main* | yes | build → stage A → launch A under CodeLLDB |
| **Attach** (debug a running editor) | **F5** → *Editor - attach* | yes | pick the editor process; Stop = detach (app lives) |
| Pure build (compile-check) | **F7** (CMake Tools) | no | build `build/` only; no stage, no launch |

The everyday flow is `scripts/run_source_mode.py`, run as the **default build
task** so Cmd/Ctrl+Shift+B drives it with zero setup. It is the single source of
"build → branch → launch": CMake is a dumb build subroutine, and the cold-vs-
reload *decision* lives in the script (ordinary imperative code), keyed off the
connect-probe. **Build is unconditional; staging happens only on the cold-start
branch** — when an editor is live the script signals it and stages nothing,
leaving the running slot untouched (the editor stages the free slot itself).

Build and run share the build hotkey by design (the editor *prompts* before any
reload, so it is non-intrusive). A *pure* build that neither stages nor launches
is **F7** (CMake Tools).

> **Cold-start-only today.** Until the editor's IPC endpoint and second-slot
> staging exist, the connect-probe always fails closed to cold start, so the run
> task always builds, stages A, and launches A. With no single-instance guard
> yet, running it while an editor is already up re-stages slot A and spawns a
> duplicate; on Windows the re-stage can fail outright, because the running
> instance still has slot A's binaries mapped and locked. Both resolve once the
> editor binds the endpoint (the second run becomes a reload that stages the
> *free* slot) and acquires the single-instance lock.

The cold-start launch passes the path contract the editor will consume:

```
--source-mode --build-tree <abs build/<preset>> --slot-root <abs repo>/out/source
```

The per-config artifact names and folder layout inside a slot are defined in
[BuildLayout.md](BuildLayout.md). In brief: first-party binaries carry a config
suffix (`AdHocEditor` / `AdHocEditorD` / `AdHocEditorDev`), and **Debug** binaries
plus their dependencies live in a `debug/` subfolder (Windows slot root / macOS
`Frameworks/debug`), so the launched path is config-dependent:

- **Windows:** `out/source/a/AdHocEditor.exe` (Release/Dev) vs
  `out/source/a/debug/AdHocEditorD.exe` (Debug).
- **macOS:** always `out/source/a/Ad Hoc Editor.app/Contents/MacOS/AdHocEditor<suffix>`
  — the bundle directory is fixed and the executable stays in `Contents/MacOS/`
  in every config (only the suffix changes; its debug dylibs sit in
  `Frameworks/debug` and are reached via the executable's rpath).

### VS Code (checked in)

`.vscode/tasks.json`, `.vscode/launch.json`, and `.vscode/extensions.json` are
committed; the recommended extensions (CMake Tools, CodeLLDB, C/C++) install on
prompt. One-time setup: select the build **preset**, and select **`Launcher`** as
the CMake Tools *launch target* (the launch configs interpolate
`${command:cmake.launchTargetFilename}`, which launch.json can't compute on its
own). The everyday Run/Reload task needs nothing further —
`run_source_mode.py` derives the editor binary name from `CMAKE_BUILD_TYPE`.

- **Everyday — Run/Reload (Cmd/Ctrl+Shift+B):** the **default build task** runs
  `run_source_mode.py`: build the editor, then connect-probe — a live editor is
  signaled to reload; otherwise stage slot A and launch it detached (no debugger).
  Staging happens only on the cold-start branch, so a running editor's slot is
  never overwritten (the editor stages the free slot itself on reload).
- **Pure build (F7):** CMake Tools builds `build/` only — no stage, no launch.
- **Rare — Debug from main() (F5 → *Editor - debug from main*):** the
  *Source mode: stage* pre-launch task builds + stages `out/source/a/`; CodeLLDB
  launches it and breaks at `main()`. A single config covers every OS/config —
  the `osx`/`windows` blocks override only the program path, and on Windows the
  stage step points the `out/source/a-bin` junction at the config's binary dir so
  the path is config-uniform.
- **Attach (F5 → *Editor - attach*):** pick the running editor process. Press
  **Stop** to **detach** — the editor keeps running. Use this to debug an
  already-running editor (F5 on *debug from main* cold-starts a fresh process).

### CLion (committed `.run/`, best-effort)

CLion's run config isn't debugger-bound: a single *CMake Application* config can
be launched with the **Run ▶** button (no debugger, the everyday flow) or the
**Debug 🐞** button (debugger from `main()`, the rare flow), both running the same
`source-mode` Before-launch build. Two configs are committed under `.run/`:
**`Editor (source mode)`** (the CMake Application above) and
**`Editor - Run/Reload (script)`** (runs `run_source_mode.py` for reload parity
with VS Code's default-build flow). Attach in CLion is a manual *Run → Attach to
Process* (Stop detaches).

CLion has no launch-target-filename variable and its `.run/` schema is
version-specific, so the committed XML is a **best-effort starting point** —
CLion may rewrite it on first edit. If it doesn't load, recreate the CMake
Application config from these steps (this is the authoritative reference):

1. **Run → Edit Configurations → + → CMake Application.** Name it e.g.
   *Editor (source mode) — Debug*.
2. **Target:** `source-mode` (CLion builds this custom target before launch, which
   stages the cold slot).
3. **Executable:** *Select other…* and browse to the slot A binary (config suffix:
   `D`=Debug, `Dev`, none=Release):
   - macOS: `out/source/a/Ad Hoc Editor.app/Contents/MacOS/AdHocEditor<suffix>`
     (bundle directory is fixed; only the inner binary name changes)
   - Windows Debug: `out\source\a\debug\AdHocEditorD.exe`; Release/Dev:
     `out\source\a\AdHocEditor.exe` / `out\source\a\AdHocEditorDev.exe`
4. **Program arguments:**
   `--source-mode --build-tree $ProjectFileDir$/build/host-mac-debug --slot-root $ProjectFileDir$/out/source`
   (point `--build-tree` at the active profile's build dir).
5. **Working directory:** `$ProjectFileDir$/out/source/a` (use
   `…/out/source/a/debug` for a Windows Debug profile, where the exe is staged).
6. Confirm **Before launch** shows **Build** (builds the `source-mode` target).
7. Use the matching CMake profile (Debug profile ↔ the `LauncherD` config), then
   commit the generated `.run/*.run.xml` to share it. Use **Run ▶** for everyday
   work, **Debug 🐞** only when you need to break in startup.

---

## Deferred (decide at implementation time)

- **The IPC endpoint and reload signal** — the named lock, the `AF_UNIX`
  socket / named pipe, and the `reload` message handshake; how the editor's main
  loop services the endpoint without stalling.
- **Editor-side free-slot staging** — the running editor copying `build/` into
  the free slot and spawning the successor there (the script only stages slot A
  for cold start today).
- **What state must survive a reload** (open projects, undo history, window
  layout, unsaved edits, live game-world state) and therefore whether
  serialize/restore is cheap enough to run on every handoff.
- **Handoff handshake mechanics** — how serialized state crosses between
  predecessor and successor (file vs shared memory vs inherited handle) and how
  the successor waits for the predecessor without a race.
- **macOS slot behavior** — confirmed run-from-copy everywhere for a single code
  path; whether macOS could collapse to a single slot (no DLL lock) is left open
  in favor of one cross-platform code path.
