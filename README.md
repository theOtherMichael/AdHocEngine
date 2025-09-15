![Ad Hoc Engine](Readme_Logo_LightMode.png#gh-light-mode-only)
![Ad Hoc Engine](Readme_Logo_DarkMode.png#gh-dark-mode-only)

# Ad Hoc Engine

Ad Hoc Engine is a lightweight, "hypermodular" C++20 game engine focussed on extensibility, flexibility, and agility. It only *directly* includes a few core features:

* An extensible editor application, **Ad Hoc Editor**
* An extensible build system
* An extensible asset pipeline
* An extensible scene graph system
* A centralized package manager

All other major features (rendering, input, UI, etc.) are provided as first-party packages that are *themselves* built with Ad Hoc Engine. This lets you easily remove features you don’t use, fork and customize features you do use, and even replace features wholesale with third-party or in-house solutions.

At least, that's the design -- I just have to finish building it.

Follow development on Twitch:

[![twitch](https://img.shields.io/badge/Twitch-9146FF?style=for-the-badge&logo=twitch&logoColor=white)](https://www.twitch.tv/adhocdev)

## Building the Engine

The entirety of Ad Hoc Engine is built from a single Visual Studio solution (Windows) and Xcode workspace (macOS). Dependencies are automatically installed when you build.

### Visual Studio (Windows)

Open `AdHocEngine.sln`.

* Use the **`Debug`**, **`Dev`**, and **`Release`** configurations to build and run the editor in developer mode.
* Use the **`StaticDebug`**, **`StaticDev`**, and **`StaticRelease`** configurations to build and run tests on the static engine library.

### Xcode (macOS)

Open `AdHocEngine.xcworkspace`.

* Use the **`Debug`**, **`Dev`**, and **`Release`** schemes to build and run the editor in developer mode.
* Use the **`StaticDebug`**, **`StaticDev`**, and **`StaticRelease`** schemes to build and run tests on the static engine library.
* Use the **`Bundle`** scheme to build and archive the final editor app bundle.

## Build System Notes

The Visual Studio solution is organized as you’d expect, with a few exceptions:

* The `Debug`, `Dev`, and `Release` configs *also* trigger the `StaticDebug`, `StaticDev`, and `StaticRelease` builds (respectively) as a post-build step.
* A vcpkg install script runs as a custom build tool on each `vcpkg.json` file.

The Xcode workspace, on the other hand, is set up somewhat unconventionally, since Xcode doesn’t allow cross-configuration dependencies:

* The `Debug`, `Dev`, and `Release` "configurations" are implemented as separate build targets. `Debug` targets have a **"D"** suffix, `Dev` targets have a **"Dev"** suffix, and `Release` targets have **no suffix**.
* The Xcode config switch is repurposed as an **`ONLY_ACTIVE_ARCH`** switch. The `Debug`, `Dev`, and `Release` schemes use the **`ActiveArch`** config, while the `Bundle` scheme (used for shipping) uses the **`Universal`** config.

## Libraries Used

### Engine

* mimalloc: <https://github.com/microsoft/mimalloc>
* {fmt}: <https://github.com/fmtlib/fmt>
* GLFW: <https://github.com/glfw/glfw>

### Editor

* Dear ImGui: <https://github.com/ocornut/imgui>

## Other Resources Used

* vcpkg: <https://github.com/microsoft/vcpkg>
* Google Test: <https://github.com/google/googletest>
