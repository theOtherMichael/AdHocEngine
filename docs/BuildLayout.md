# Build Product Layout

Where every build artifact is named and placed, per platform and config, for
both the **merged shipping bundle/installer** and a **single-config source-mode
slot**. The CMake install rules implement this layout in full; for the
slot/reload model itself see [SourceMode.md](SourceMode.md).

---

## Naming

### Config suffix (first-party binaries)

First-party build products carry a per-config suffix, identical across platforms:

| Config | Suffix |
|---|---|
| Release | *(none)* |
| Debug | `D` |
| Dev | `Dev` |

### Product names

| Product | Windows | macOS |
|---|---|---|
| Launcher (the editor entry point) | `AdHocEditor<suffix>.exe` | Mach-O `AdHocEditor<suffix>` inside `Ad Hoc Editor.app` |
| Launcher — console "terminal build" | `AdHocEditorConsole<suffix>.exe` | *(none — single binary already blocks)* |
| Engine | `Engine<suffix>.dll` | `libEngine<suffix>.dylib` |
| Editor | `Editor<suffix>.dll` | `libEditor<suffix>.dylib` |

The **console "terminal build"** (`AdHocEditorConsole.exe`) is a Windows-only twin
of the Launcher built from the identical sources but with `/SUBSYSTEM:CONSOLE`
instead of `/SUBSYSTEM:WINDOWS`. It blocks until exit and routes stdout/stderr to
the caller's console or pipe — the natural fit for CI, automation, and any
non-interactive (headless) invocation. It stages beside the GUI exe in the same
config-dependent directory and ships in the installer, so a pipeline can stage the
slot (`--target source-mode`) and run it directly. macOS/Linux need no twin: the
single editor binary already behaves this way.

The macOS **app bundle directory** is always `Ad Hoc Editor.app` — config-independent
— so one bundle can hold every config's Mach-O. Only the inner executable name and
`CFBundleExecutable` vary by config.

vcpkg runtime dependencies (`fmt`, `glfw3`, `mimalloc`; `magic-enum` is
header-only and ships nothing) keep **whatever names vcpkg emits**. We do **not**
rely on debug variants having distinct names — debug and release deps are kept
apart by folder, not by name (see below).

---

## Placement rule

Exactly one rule drives every layout decision:

> **Debug artifacts live in a `debug/` subfolder; Release and Dev artifacts live
> at the top level.** A single-config slot uses the *same* layout it would have
> in the merged bundle — a Debug slot still puts its binaries under `debug/`.

Rationale:
- **Like-named coexistence.** Debug and release vcpkg deps frequently share a
  filename. Foldering keeps them apart without depending on any port to add a
  suffix.
- **Slot mirrors shipping.** Each binary's load paths (Windows DLL-search
  directory; macOS baked `@rpath`) are relative. If a slot placed a binary at a
  different depth than the shipping bundle, those baked paths would resolve in
  one but not the other. Keeping the layout identical means the *same* binary,
  unmodified, runs correctly in both.

**macOS executable exception.** A `.app` requires its primary Mach-O in
`Contents/MacOS/`, so `AdHocEditorD` stays there (next to `AdHocEditor`) rather
than moving into a `MacOS/debug/`. Only its *dependencies* move to
`Frameworks/debug/`; the executable reaches them via its own rpath. (Dev/Release
need no exception — they're already at the top level.)

**Dev never coexists with Release.** Shipping bundles exclude Dev, and slots are
single-config, so Dev and Release binaries are never assembled into the same tree.
The "Dev at top level, like Release" rule therefore just means *Dev lays out
exactly like Release* — there is no Dev↔Release name collision to resolve because
they are never present together. Dev links the **release** vcpkg deps
(`CMAKE_MAP_IMPORTED_CONFIG_DEV = Release`), so a Dev slot's top-level deps are
the release ones.

---

## Windows

### Single-config slot

A slot holds exactly one config. Top level vs `debug/` is chosen by that config.

```
# Debug slot
<slot>/
  debug/
    AdHocEditorD.exe  AdHocEditorConsoleD.exe   # GUI + console twin
    EngineD.dll  EditorD.dll
    fmt.dll  glfw3.dll  mimalloc.dll        # debug vcpkg deps (names as emitted)
    *.pdb                                    # debug symbols, beside their binaries

# Dev slot
<slot>/
  AdHocEditorDev.exe  AdHocEditorConsoleDev.exe
  EngineDev.dll  EditorDev.dll
  fmt.dll  glfw3.dll  mimalloc.dll          # release vcpkg deps (Dev maps to Release)
  *.pdb

# Release slot
<slot>/
  AdHocEditor.exe  AdHocEditorConsole.exe
  Engine.dll  Editor.dll
  fmt.dll  glfw3.dll  mimalloc.dll          # release vcpkg deps
```

Windows has no rpath: a DLL is resolved from the loading module's own directory.
That is why the Debug exe sits **in `debug/` alongside its DLLs** rather than at
the top level — colocation is what makes the load succeed.

### Merged shipping installer

`scripts/package.py` builds Debug + Release (never Dev), stages each, and merges:

```
<install>/
  AdHocEditor.exe  AdHocEditorConsole.exe    # release — GUI entry point + console twin
  Engine.dll  Editor.dll
  fmt.dll  glfw3.dll  mimalloc.dll          # release vcpkg deps
  *.pdb                                      # release symbols
  debug/
    AdHocEditorD.exe  AdHocEditorConsoleD.exe
    EngineD.dll  EditorD.dll
    fmt.dll  glfw3.dll  mimalloc.dll        # debug vcpkg deps (like-named, isolated)
    *.pdb                                    # debug symbols
  lib/                                       # import libraries — not needed at runtime
  llvm/                                      # embedded toolchain (package.py)
  build-libs/                                # cross-compiled static engine libs (package.py)
```

The debug↔release runtime switch launches the sibling exe: release
`AdHocEditor.exe` re-launches `debug/AdHocEditorD.exe` (and vice versa).

---

## macOS

The bundle directory is `Ad Hoc Editor.app` in every config and in the merged
bundle. `CFBundleExecutable` names the config's Mach-O.

### Single-config slot

```
# Debug slot
<slot>/Ad Hoc Editor.app/Contents/
  MacOS/AdHocEditorD                         # exception: exe stays here, not in a subfolder
  Frameworks/debug/
    libEngineD.dylib  libEditorD.dylib
    libfmt*.dylib  libglfw*.dylib  libmimalloc*.dylib    # debug vcpkg deps
  Resources/{AppIcon.icns, Assets.car}
  Info.plist                                 # CFBundleExecutable = AdHocEditorD
# rpath of AdHocEditorD: @executable_path/../Frameworks/debug

# Dev slot
<slot>/Ad Hoc Editor.app/Contents/
  MacOS/AdHocEditorDev
  Frameworks/
    libEngineDev.dylib  libEditorDev.dylib
    libfmt*.dylib  libglfw*.dylib  libmimalloc*.dylib    # release vcpkg deps
  Info.plist                                 # CFBundleExecutable = AdHocEditorDev
# rpath of AdHocEditorDev: @executable_path/../Frameworks

# Release slot
<slot>/Ad Hoc Editor.app/Contents/
  MacOS/AdHocEditor
  Frameworks/
    libEngine.dylib  libEditor.dylib
    libfmt*.dylib  libglfw*.dylib  libmimalloc*.dylib    # release vcpkg deps
  Info.plist                                 # CFBundleExecutable = AdHocEditor
# rpath of AdHocEditor: @executable_path/../Frameworks
```

The mimalloc preload symlink for source mode lives in the **slot root**, outside
the signed bundle, and points at the config's mimalloc inside the slot:

```
<slot-root>/libmimalloc-adhoc.dylib  ->  <slot>/Ad Hoc Editor.app/Contents/Frameworks[/debug]/libmimalloc*.dylib
```

### Merged shipping bundle

```
Ad Hoc Editor.app/Contents/
  MacOS/
    AdHocEditor                              # release — CFBundleExecutable
    AdHocEditorD                             # debug
  Frameworks/
    libEngine.dylib  libEditor.dylib                     # release first-party
    libfmt*.dylib  libglfw*.dylib  libmimalloc*.dylib    # release vcpkg deps
    debug/
      libEngineD.dylib  libEditorD.dylib
      libfmt*.dylib  libglfw*.dylib  libmimalloc*.dylib  # debug vcpkg deps (like-named, isolated)
  Resources/
    AppIcon.icns  Assets.car
    llvm/                                    # embedded toolchain (package.py)
    build-libs/                              # cross-compiled static engine libs (package.py)
  Info.plist                                 # CFBundleExecutable = AdHocEditor
```

Each Mach-O carries its own rpath: `AdHocEditor` → `../Frameworks`,
`AdHocEditorD` → `../Frameworks/debug`. The release exe is the default launch;
the runtime switch execs `AdHocEditorD` directly.

---

## Launch path per config (source mode)

Because Debug binaries move into `debug/`, the cold-start launch path is **not**
uniform across configs on Windows:

| Config | Windows launch path | macOS launch path |
|---|---|---|
| Debug | `<slot>/debug/AdHocEditorD.exe` | `<slot>/Ad Hoc Editor.app/Contents/MacOS/AdHocEditorD` |
| Dev | `<slot>/AdHocEditorDev.exe` | `…/Contents/MacOS/AdHocEditorDev` |
| Release | `<slot>/AdHocEditor.exe` | `…/Contents/MacOS/AdHocEditor` |

macOS keeps a stable `Contents/MacOS/<exe>` shape (only the suffix changes,
because of the executable exception). Windows gains a `debug/` segment for the
Debug config. IDE run configs must account for this — `cmake.launchTargetFilename`
yields the filename but not the `debug/` prefix.

---

## Implementation status

The layout above is fully implemented by the `SourceMode` install component
(`cmake/AdHocMacBundle.cmake`, `cmake/AdHocWindowsInstaller.cmake`) and the
`source-mode` target. In particular:

1. **macOS inner-binary rename.** The Launcher's `OUTPUT_NAME` bakes the config
   suffix (`AdHocEditor<suffix>`) so the bundle builds as
   `AdHocEditor<suffix>.app`; a post-install step renames the directory to the
   fixed `Ad Hoc Editor.app`, decoupling the bundle dir name from the inner
   Mach-O name (`Launcher/CMakeLists.txt`, `cmake/AdHocMacBundle.cmake`).
2. **`debug/` destinations.** Debug installs into `debug/` (Windows) and
   `Frameworks/debug/` (macOS), pulling the config-matched vcpkg variant
   (`debug/lib` vs `lib`) so like-named debug/release deps coexist after merge.
3. **Config-aware macOS rpath fixup.** Each MacOS binary gets
   `@loader_path/../Frameworks<[/debug]>` matched to the config's Frameworks
   subfolder.
4. **mimalloc preload symlink** points at the config's mimalloc inside
   `Frameworks[/debug]/`.
5. **IDE run configs** use one launch config per OS. The Windows Debug-vs-
   Release/Dev path split (`out/source/a/debug/…` vs `out/source/a/…`) is hidden
   behind the `out/source/a-bin` directory junction, which the source-mode stage
   step (`scripts/run_source_mode.py --stage-only`) points at the active config's
   binary dir — so the launch.json `windows` block is one config-uniform path.

What remains is the **in-session reload runtime** — not part of this layout
spec. See the deferred items in [SourceMode.md](SourceMode.md) (IPC endpoint,
single-instance guard, editor-side free-slot staging, serialize/restore).
