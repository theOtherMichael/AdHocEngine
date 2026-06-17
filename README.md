![Ad Hoc Engine](Readme_Logo_LightMode.png#gh-light-mode-only)
![Ad Hoc Engine](Readme_Logo_DarkMode.png#gh-dark-mode-only)

# Ad Hoc Engine

Ad Hoc Engine is a lightweight, "hypermodular" C++ game engine focused on extensibility, flexibility, and agility. It only *directly* provides the following:

* An extensible editor application, **Ad Hoc Editor**
* An extensible build system
* An extensible asset pipeline
* An extensible scene graph system
* A package manager

All other major engine features (rendering, input, UI, etc.) are *themselves* packages built with Ad Hoc Engine. Packages are first-class citizens, which lets you include only what you need, and nothing you don't.

This engine is a work in progess. Follow development on Twitch: [![twitch](https://img.shields.io/badge/Twitch-9146FF?style=for-the-badge&logo=twitch&logoColor=white)](https://www.twitch.tv/adhocdev)

## Supported Platforms

Ad Hoc Editor only supports Mac and Windows. Linux support is planned, but not prioritized. Mobile and console game support are not planned.

## Prerequisites

| Tool | Notes |
|---|---|
| **CMake 3.28+** | Build generator |
| **Ninja** | Used in CMake presets |
| **Python 3** | Used in `cmake/` and `scripts/` |
| **(macOS) Xcode** | To build certain resources on Mac |
| **(Windows) NSIS** | To make Windows installers |

> [!NOTE]
> The full Xcode application is required; Xcode Command Line Tools is insufficient. Be sure to point at it with `xcode-select` at least once: `sudo xcode-select -s /Applications/Xcode.app/Contents/Developer`

## Getting Started

Ad Hoc Engine uses CMake to build. To get started, configure CMake using a preset with `--preset`:

```bash
cmake --preset host-windows-debug
```

* **host-windows-\<config\>**: Build the Windows editor in a specific config.
* **host-mac-\<config\>**: Build the macOS editor in a specific config.

Ad Hoc Engine builds itself and its game projects using a pinned version of clang. For this reason, always use the CMake presets found in `CMakePresets.json` to configure. To switch to another build config (debug, dev, or release), just reconfigure using the corresponding preset.

During configuration, CMake will auto-install the following:

* **LLVM/Clang**: Used by the editor and to build the editor. Installed at `tools/llvm/`.
* **vcpkg**: Dependency manager. Installed as submodule at `tools/vcpkg/`.
* **vcpkg dependencies**: The packages specified in `vcpkg.json`. These install in your build tree.

These downloads are large (particularly LLVM), so your first configuration will take several minutes to finish. Subsequent configures will go much faster.

Once configured, use the following scripts to take common actions:

- **Build** — `python scripts/build.py`. Builds the editor for one config.
- **Test** — `python scripts/test.py`. Runs all tests for a config.
- **Package** — `python scripts/package.py`. Packages the editor for distribution.

## Visual Studio Code + CMake Tools

While you can certainly work from the command line, this repository is really meant to be used with CMake Tools and VS Code. Follow these steps to get started:

1. Open the repository in vscode.
2. Accept the prompt to install the **CMake Tools**, **CodeLLDB**, and **C/C++**.
3. Select a **build preset** from the CMake Tools status bar (e.g. *Windows host — Debug*).

Then you're set. Here are the things you can do:

| Key | Action |
|---|---|
| **Cmd/Ctrl+Shift+B** | **Run/Reload** (default build task) — build, and reload an editor in "source mode". |
| **F7** | **Pure build** — compile `build/` only; no stage, no launch (CMake Tools). |
| **F5 → "Editor - debug from main"** | Build and debug from source mode slot A |
| **F5 → "Editor - attach"** | Attach the debugger to a running editor process. |

The first time you launch, VS Code will ask you to pick a launch target. Choose **`Launcher`**.

## Dependencies

| Package | Used by | |
|---|---|---|
| LLVM/Clang | Tooling | <https://github.com/llvm/llvm-project> |
| Google Test | Tests | <https://github.com/google/googletest> |
| vcpkg | Tooling | <https://github.com/microsoft/vcpkg> |
| magic-enum | Engine | <https://github.com/Neargye/magic_enum> |
| {fmt} | Engine | <https://github.com/fmtlib/fmt> |
| metal-cpp *(macOS)* | Engine | <https://github.com/apple/metal-cpp> |
| GLFW | Engine / Editor | <https://github.com/glfw/glfw> |
| mimalloc | Editor / Launcher | <https://github.com/microsoft/mimalloc> |
| Dear ImGui | Editor | <https://github.com/ocornut/imgui> |
