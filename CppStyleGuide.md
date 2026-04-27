# Ad Hoc C++ Style Guide

I maintain this style guide as a personal reference, but future contributors may find it useful.

## Automated Code Formatting

This project uses **clang-format** to enforce a consistent style across the code base. Run it regularly as you code.

- **Visual Studio**: Use **Format Selection** (Ctrl + K, Ctrl + F) or **Format Document** (Ctrl + K, Ctrl + D) to apply clang-format.
- **Xcode**: You must run clang-format manually. My current approach is to set up an **Automator** script that formats highlighted text. Use the `.clang-format` file in the repository root.

If clang-format produces undesirable results (e.g., messing with whitespace in a macro definition), temporarily disable it with `// clang-format off` and `// clang-format on`. Never commit code that hasn't been formatted.

A script for formatting the entire repository at once is available in `scripts/clang_format.py`.

## Project Organization

### Code Files

Always use these file extensions for code files:

- C++ source: `.cpp`
- Objective-C++ source: `.mm`
- C source: `.c`
- Objective-C source: `.m`
- Headers (for all of the above): `.h`

Follow these guidelines when adding new source files:

- Place all **source files** in the project's `src/` folder.
- Put **exported headers** in `include/`, and **private headers** in `src/`.
- Each header should have at most **one associated source file**, and vice versa. Don't make both a public and private header for the same `.cpp` file.
- All definitions in a source file should be declared within that source file or its associated header.
- Use **identical filenames** for header/source pairs, and keep them at the same path relative to `include/` and `src/`.
  - Example: the header for `src/FolderA/ClassA.cpp` should be at either `include/FolderA/ClassA.h` or `src/FolderA/ClassA.h` (though not both).
- Otherwise, **keep all filenames unique**, even if they're in different folders. This makes it easier to locate files with **Code Search** (Visual Studio) and **Open Quickly** (Xcode).

### Folders

Within `src/`, `include/Engine`, and `include/Editor`, organize code into these subfolders:

- `Core/`: Foundational engine or editor code (e.g., application setup/teardown).
- `Common/`: Portable, highly reusable code (e.g., helper classes or utility functions).
- `Views/`: Built-in editor views. Use subfolders to group into logical collections.
- `Window/`, `Graphics/`, etc.: Each subsystem or feature should have its own folder.

Non-code assets (fonts, images, etc.) should go in `resources/`.

`vendor/` and `vcpkg_installed/` folders are reserved for third-party dependencies. Never put first-party files in them, especially `vcpkg_installed/`, which is dynamically populated by the build system.

Highly reusable code should generally go in a `Common/` folder. However, if the code addresses universal C++ concerns, and has no dependencies in Engine or Editor, consider putting it in the Ad Hoc Support Library instead (next section).

### Ad Hoc Support Library

The Ad Hoc Support Library (abbreviated: ASL) is a collection of general-purpose C++ utilities intended to augment the standard library. Unlike engine or editor code, ASL has no dependencies on Ad Hoc Engine and can (theoretically) be dropped into any C++ project with minimal effort.

ASL headers are located in `include/asl` in the Engine project, and all ASL symbols are contained in the `asl::` namespace.

```cpp
#include <asl/finally.h>

auto cleanup = asl::finally([&] { CloseHandle(handle); });
```

The ASL is based on the Guidelines Support Library (GSL), and borrows from it heavily. Extend it with generally useful C++ utilities which facilitate maintainable, expressive code. Good candidates include: custom casts, type utilities, RAII helpers, and concept definitions.

If a utility is generally useful, but isn't useful outside of Ad Hoc Engine, put it in a `Common/` folder instead as part of the Engine library.

### Third Party Dependencies

Maintaining and updating dependencies will always be a pain, but these guidelines should help.

First, whenever possible, use **vcpkg** to install new dependencies. Vcpkg is the primary dependency manager for Ad Hoc Engine, and it is fully integrated into the build system. The integration makes adding a new dependency pretty straightforward:

1. Identify which project needs the dependency. Choose the _lowest-level_ project which uses it, even if you plan to use it in more than one project.
2. Add the dependency to that project's `vcpkg.json`.
3. Rebuild the project. This will trigger a vcpkg install step in Visual Studio or Xcode.
4. Locate and link to the new dependencies in `vcpkg_installed/`.

A custom build step automatically installs dependencies when manifests are updated. Header files should integrate automatically -- linking is the only manual step.

On macOS, `vcpkg_installed/` will contain a folder for x86 binaries, a folder for ARM binaries, and finally a folder for universal binaries. Always link to the universal versions; the others are just intermediate files created by the build system.

If custom build flags are necessary for a dependency, add them to the custom triplets files in `triplets/`. There are existing examples of this in the triplet files.

If vcpkg can't be used to install a dependency for whatever reason, prefer the following alternatives (in order):

1. Add the dependency as a git submodule inside `<ProjectName>/vendor/`, else
2. Add the dependency's prebuilt binaries directly to `<ProjectName>/vendor/`, else
3. Add the dependency's source code directly to `<ProjectName>/vendor/`.

Always keep a copy of each dependency's license in its folder. Do not use libraries that are incompatible with Ad Hoc's open-source license.

### Xcode

While developing on Windows, regularly run `scripts/update_xcode_sources.rb` to update the corresponding Xcode projects. This prepares Xcode to build successfully after pulling changes from another platform (platform-specific code notwithstanding).

If an Xcode build fails after running the script, update the script to handle the edge case. Xcode is generally not used directly for engine development, so the script should be robust and gracefully handle edge cases.

## C++ Code Style

This section documents the code style used throughout Ad Hoc Engine. These aspects are not enforced by clang-format, but following them closely will help keep the project intelligible and scalable. Follow them when possible.

### Naming Conventions

#### General Guidelines

- Use clear, descriptive names that make your code self-explanatory.
- If a name isn't clear in all contexts, rename it. Don't be afraid of refactors.
- Only use acronyms in your names if they're widely recognized (e.g. HTTP, PNG).
- If you _do_ use acronyms, capitalize only the first letter (`HttpService`, not `HTTPService`).
- Avoid sucky legacy terminology like _Master_ and _Slave_ -- we live in the 21st century.

#### Casing Rules

| Element                     | Convention             |
| --------------------------- | ---------------------- |
| Types                       | `PascalCase`           |
| Public Variables/Constants  | `PascalCase`           |
| Private Variables/Constants | `camelCase`            |
| Functions                   | `PascalCase`           |
| Function Parameters         | `camelCase`            |
| Template Parameters         | `PascalCase`           |
| Macros                      | `SCREAMING_SNAKE_CASE` |

The Ad Hoc Support Library uses slightly different capitalization rules from the above:

| Element       | Convention   |
| ------------- | ------------ |
| ASL Types     | `snake_case` |
| ASL Functions | `snake_case` |

This is to make the ASL more closely resemble the standard library. It also makes it more obvious when something isn't (directly) part of the Ad Hoc code base.

### Namespaces

#### Guidelines

- **Engine code** should live under `::Engine`.
- **Editor code** should live under `::Editor`.
- **Launcher code** does not require a namespace, as it has no consumers.
- **Internal-only APIs** should live in an `Internal` sub-namespace (e.g. `::Engine::FeatureName::Internal`).
- **Platform-specific code** should live in a `Platform` sub-namespace (see **Platform Abstraction**).

Keep namespace nesting shallow -- **no more than three levels deep**. An exception is made for `Internal::` and `Platform::`, as they aren't intended to be directly used in consumer code.

The first namespace following `Engine::` or `Editor::` should generally correspond to a **feature or subsystem**. Some examples:

- `Engine::Graphics`
- `Engine::Window`
- `Engine::Time`

Try to keep the names themselves short, as they will be used repeatedly in consumer code (e.g. `Engine::Time::DeltaTime()`).

If you need to put an internal-use function or global into a public header, put it into an `Internal::` namespace. This cleanly documents which items are not part of your API, even it if it is visible to consumers.

### Header Include Guidelines

- Use **`#pragma once`** instead of C-style include guards.
- Always **include what you use directly**. Do not rely on transitive includes.
- Use **quotes** (`"Header.h"`) for paths **relative to the header file** itself.
- Use **angle brackets** (`<Header.h>`) in all other cases (including paths relative to `include/` or `src/`).

Organize your `#include`'s into the following logical groups, with an empty line separating each group:

1. Interface ("main") header
2. Current project headers
3. Adjacent project headers
4. Third-party library headers
5. Operating system headers
6. C++ standard library headers
7. C standard library headers

Each group is automatically sorted alphabetically by clang-format, which makes it important to include a blank line between each group.

### Preprocessor Macros

Function-like preprocessor macros should be avoided. Only use them for things that cannot be accomplished in pure C++. The two main examples are assertion macros and platform abstraction macros.

When defining new compiler flags, assign them a value of `1` or `0`. This enables auto-complete in unsupported configurations, and allows their use in C++ expressions, e.g. `SOME_SWITCH ? valueA : valueB`.

When reasonable, avoid adding preprocessor symbols to the build system. Instead, add them to relevant header files. This prevents common build system bugs and makes it less likely developers will use the flags incorrectly.

### Class/Struct Organization

Follow these conventions when writing new classes or structs:

- Use **structs** when you don't have private or protected members. Use **classes** for everything else.
- Always use explicit `private:` blocks in classes. Don't rely on implicit privateness.
- Order your access specifiers by decreasing access level: `public`, then `protected`, then `private`.
- Use only one block per access level, unless you have metaprogramming or data-packing reasons to use more.

Use the following order for members within each access scope:

1. Default constructor
2. Destructor
3. Non-default constructors
4. Copy constructor
5. Copy assignment operator
6. Move constructor
7. Move assignment operator
8. Member functions, **ordered by decreasing use frequency**
9. Data members, **generally ordered by decreasing alignment**
10. PIMPL types and pointers

For consistency, order the implementations of class functions after their declaration order in the corresponding `.cpp` file.

## Modern C++

Prefer **modern C++** features and patterns whenever possible. This keeps the codebase readable and maintainable, but also makes your life a lot easier.

General rules of thumb:

- Prefer **STL containers and algorithms** over writing your own.
- Prefer **smart pointers** (e.g. `std::unique_ptr`) over raw pointers.
- Use **`auto` by default** to prevent unexpected type conversions and increase readability.
- Prefer **named casts** over C-style casts.
- Prefer **range-based for loops** and the `<ranges>` library over iterators and index-based loops.
- Use **C++20 concepts** to constrain templates and make them more readable.
- Prefer `std::filesystem` over C APIs like `fopen()` or platform-specific alternatives.

### Almost Always `auto`

This guideline may be controversial. Ad Hoc Engine follows Herb Sutter's AAA style, described in full in his blog post: https://herbsutter.com/2013/08/12/gotw-94-solution-aaa-style-almost-always-auto/.

This style provides several benefits:

- Ensures variables are initialized.
- Prevents accidental type conversions.
- Forces intentional type conversions to be explicit.
- Simplifies refactoring and maintenance.
- Reduces cognitive load for both the reader and writer.

Cases where `auto` _harms_ readability are actually pretty rare. If you can't easily understand an `auto` variable's type or purpose from context, renaming something would probably be more effective than introducing a cast.

Use literal types to elegantly set the type of the variable during initialization:

```cpp
auto myFloat         = 5.1f;
auto myMutableString = "Some initial string value"s; // std::string_literals
auto myBitMask       = 0b1111'0000_u32;              // asl::int_literals
```

Alternatively, use brace initialization to name the type:

```cpp
auto myMutableString = std::string{"Some initial string value"};
```

Function return types should be tacit from both the function name and the variable name. If you need to change types, put the destination type on the right:

```cpp
// Good, it's obvious what resourceHandle is for
auto resourceHandle = GetSomeResource();

// Good, the conversion to WrapperType is made explicit
auto wrappedHandle = WrapperType{GetSomeResource()};
```

#### Trailing Return Types

Policy TBD. This section is intentionally left blank pending a decision.

### RAII and Resource Management

Use RAII for resource management whenever possible. It's generally the least problematic way to ensure resources are released correctly, even in cases of unhandled exceptions.

When using an API for which RAII is not directly supported (e.g. GLFW), use `asl::finally()` to generate ad hoc RAII wrappers.

```cpp
InitializeImGui();
const auto shutdownImGuiAction = asl::finally(ShutdownImGui);
```

### Lambdas

Lambda capture semantics can introduce irritating, subtle bugs if you're not careful. Follow these guidelines to cut down on those:

- Always use **explicit capture lists**: for example `[variableName]` or `[&variableName]`. Never use blanket captures (`[=]` or `[&]`).
- **Don't capture `this` or local variable references** unless the lambda will execute only in the current scope (e.g. algorithm predicates).

Explicit capture lists make lifetimes and ownership clear to both the reader and writer.

## Errors and Exceptions

For most situations, use `std::expected` (or another result type) to indicate when a function fails. This clearly communicates potential failure points to the caller, and forces the caller to handle each one.

That said, exceptions *are* enabled on all platforms. Follow these guidelines if and when you use them:

* Use exceptions to **simplify** error handling. For example, editor file operations, which can have dozens of points of failure, may benefit from letting exceptions percolate up to a single handler.
* Use exceptions **derived from `std::exception`** with a clear `what()` message. This ensures compatibility with most catch blocks.
* **Don't leak exceptions** from functions that might reasonably be invoked in game builds. Instead, return `std::expected` and/or handle all exceptions before returning.

In the future, I plan to implement global catch blocks to guard against exceptions leaked from user code, but the exact configuration of those is TBD.

### Throwing in Constructors

In C++, the only two ways to error out of a constructor are:

1. Throw an exception
2. Return an invalid object

Avoid #2 at all costs. Invalid objects are an insidious source of bugs. At the same time, throwing exceptions in game code poses its own risks.

When a constructor for a widely-used class can fail, consider using the following pattern:

1. Make the constructor for the class _private_.
2. Make a public static factory function which returns `std::expected`.
3. If the preconditions or postconditions for the constructor are not met, return `asl::unexpected` from the factory function.

This forces the caller to handle the case where the construction of the object fails, and makes invalid objects impossible.

### Assertions

Use Ad Hoc Engine's assertion library (in `<Engine/Core/Assertions.h>`) to regularly document and enforce invariants. When these assertions fail at runtime, `Engine::AssertionFailedException` is thrown. Follow these guidelines when using them:

* Prefer `static_assert()` over runtime assertions.
* Avoid using `Assert_True()` for everything. Instead, use the most expressive assert for your situation (for example, `Assert_NotNull(somePointer)`).
* Use `Slow` asserts in hot code paths. They are stripped from both Dev and Release builds.
* Use `Eval` asserts for expressions with side effects that must be evaluated even in Release builds.
* Use `Expect` asserts when the assertion failure need not be fatal. This logs errors, but doesn't throw.

Note that, even though `Expect` asserts aren't fatal, they are *not* for normal runtime errors. The category exists specifically to make assertion failures easier to correct from Developer Mode. They are exclusively for establishing invariants, just like the other assertion types.

### Logging

All engine and editor errors should be reported through the Console API, accessed by including `<Engine/Core/Console.h>`. These will appear in the editor's Console view, as well as in the editor's log files and the log files of game builds.

For editor GUI workflows, errors should _also_ be surfaced directly to the user. For example, if a menu command fails, the error might be logged to the console _and_ shown in an error modal.

Don't rely on the console to convey information to the user. Console logs are for retrospectively understanding sequences of events, not for providing real-time feedback.

### Error Message Style

Consistent error messages make logs easier to scan and debug. Follow these conventions whenever writing a string passed to an assertion, exception, console log call, or `asl::unexpected`.

#### Tense and Voice

Write messages in the present tense, using the passive voice or an impersonal subject. Describe what went wrong, not what the calling code was trying to do.

```cpp
// Good
Console::LogError("Vertex buffer is null.");
Console::LogError("File not found: {}", filePath);
Console::LogError("Expected at least one element, but the array is empty.");

// Avoid — past tense reads like a post-mortem, not a live diagnostic
Console::LogError("Vertex buffer was null.");
Console::LogError("Failed to find file.");
```

#### Punctuation

End every message with a period, unless it ends in a dynamic value (e.g. `{}`). Do not use exclamation points.

```cpp
// Good
Console::LogError("Vertex buffer is null.");
Console::LogError("File not found: {}", filePath);
Console::LogError("Invalid arguments. Arg1: {}, Arg2: {}", arg1, arg2);

// Bad
Console::LogError("Vertex buffer is null"); // No period
Console::LogError("Vertex buffer is null!"); // Exclamation point
Console::LogError("File not found: {}.", filePath); // Period after arg
```

#### Capitalization

Capitalize the first word of the message only (sentence case). Do not capitalize mid-sentence terms unless they are proper names, type names, or macro names.

```cpp
"Shader compilation failed: fragment stage returned no output."   // good
"Shader Compilation Failed: Fragment Stage Returned No Output."   // too much capitalization
```

If a message starts with a variable name, use its exact capitalization, even if it starts with a lowercase letter.

```cpp
Console::LogError("newScalar is not in [0f, 1f). Value: {}", newScalar);
```

#### What to Include

A good error message answers two questions: what went wrong, and (where possible) what the relevant values were.

- **State the violated condition clearly**. Don't just say something failed — say why.
- **Include relevant values using {fmt} arguments** (e.g. `{}`). File paths, resource names, enum values, and counts are all worth including.
- **Avoid describing implementation details** that are meaningless to a reader who isn't looking at the source. Prefer human-readable descriptions over raw pointer values or internal IDs unless they are directly actionable.

```cpp
// Good — states what was violated and includes the relevant value
Assert_Lt_Fmt(index, elementCount, "Index {} is out of range (size is {}).", index, elementCount);

// Too vague — doesn't explain the constraint
Assert_Lt_Fmt(index, elementCount, "Invalid index.");

// Too much detail — raw this pointer isn't useful in a log
Assert_NotNull_Fmt(buffer, "Buffer at {:p} is null.", static_cast<void*>(this));
```

Use `INJECT_ENUM_FORMATTER` in `<Engine/Core/Formatters/EnumFormatter.h>` to make enum types implicitly formattable in error messages.

Note that while `Fmt` versions of asserts allow you provide custom error messages, their use is discouraged in cases where the assertion is self-documenting:

```cpp
// Good: the purpose of the assertions are obvious
Assert_NotNull(inputPtr);
Assert_Lt(index, elementCount);

// Bad: the message doesn't add useful information
Assert_NotNull_Fmt(inputPtr, "inputPtr is null.");
Assert_Lt_Fmt(index, elementCount, "index must be less than elementCount.");
```

#### Identifying the Source

Do not manually include the function name or file path in the message — the assertion library and logging functions capture call site information automatically. Adding call site information, such as system or function names, can cause the error message to go stale during refactors.

## Automated Testing

Unit testing is strongly recommended for "workhorse" code or APIs. For these types of features, use **test-driven development (TDD)** to ensure meaningful test coverage.

Some features are intrinsically difficult to test. Don't try to force TDD to work in those cases. In the future, a dedicated Ad Hoc game project might be added for developing hard-to-test features.

There are two **gtest** projects in the repository, **EditorTests** and **EngineTests**. Each can be run from Visual Studio and Xcode. Be sure to run tests in all configurations to ensure nothing breaks during development.

### Guidelines for Tests

- **Mirror the folder structure** of the Engine/Editor projects in the gtest projects. For example, tests for `include/FolderA/ClassA.h` in the Engine project should live in `src/FolderA/ClassATests.cpp` in the EngineTests project.
- Name gtest source files "`<HeaderName>Tests.cpp`", where `<HeaderName>` is the exact name of header/source file for which you're writing tests (e.g. "`ClassA.h`" and "`ClassATests.h`"). This makes them easier to locate using **Code Search** (Visual Studio) or **Open Quickly** (Xcode).
- Use **test fixtures** to improve readability and reduce duplication. As a rule of thumb, create a fixture if three or more tests share the same setup/teardown.
- **Avoid putting tests in anonymous namespaces**. The tests will run just fine, but Visual Studio won't surface inline controls for them.
- Use preprocessor defines (`ADHOC_DEBUG`, `ADHOC_EDITOR`, etc.) to conditionally compile tests that only build or behave correctly **in certain configurations**.

## Platform Abstraction

Ad Hoc Engine uses a **platform abstraction layer** (sometimes abbreviated to "PAL") to separate platform-specific code from generic code. All files inside a `_platform` folder belong to this layer. Everything else should remain platform-agnostic wherever possible.

### Key Macros

Generic code pulls in platform-specific code through the use of two macros, both available via `#include <Engine/Core/PlatformAbstraction.h>`:

- **`PLATFORM_HEADER(RelativePath)`** — expands to `"_platform/<PlatformName>/<PlatformName><RelativePath>"`, resolving to the current platform's version of a header.
- **`PLATFORM_HEADER_EXISTS(RelativePath)`** — expands to `1` if that header exists, or `0` if it doesn't. Use this to make platform implementations optional.

#### Required vs. optional implementations

When a platform implementation is **required**, simply include it directly. The code will fail to compile if the platform hasn't provided it:

```cpp
#include PLATFORM_HEADER(RequiredFunctionImpl.h)

int RequiredFunction()
{
    return Platform::RequiredFunctionImpl();
}
```

When a platform implementation is **optional**, use `PLATFORM_HEADER_EXISTS` to provide a fallback:

```cpp
#if PLATFORM_HEADER_EXISTS(OptionalFunctionImpl.h)
#include PLATFORM_HEADER(OptionalFunctionImpl.h)
#define PLATFORM_SUPPORTS_THIS_FEATURE 1
#else
#define PLATFORM_SUPPORTS_THIS_FEATURE 0
#endif

int OptionalFunction()
{
#if PLATFORM_SUPPORTS_THIS_FEATURE
    return Platform::OptionalFunctionImpl();
#else
    return 0;  // fallback
#endif
}
```

Note in the above example that `PLATFORM_SUPPORTS_THIS_FEATURE`, a bespoke feature flag, is introduced instead of directly using `PLATFORM_HEADER_EXISTS()` to conditionally invoke `Platform::OptionalFunctionImpl()`. This is a best practice explored in the next section.

### Feature Flags

Whenever generic code needs to branch based on platform capabilities, use **feature flags** rather than `PLATFORM_HEADER_EXISTS()` or platform defines like `ADHOC_WINDOWS`. This keeps generic and platform-specific code cleanly separated.

```cpp
#if PLATFORM_SUPPORTS_ACHIEVEMENTS
    Platform::UnlockAchievement(achievementKey);
#endif
```

Using a named feature flag instead of a platform define accomplishes several things:

- It clearly documents _why_ the code branches.
- It keeps generic code closed to modification.
- It reduces the amount of work needed to add support for new platforms.

Feature flags should ideally be defined in a header as `0` or `1`. This makes the flag auto-complete in code editors, even when the current build target doesn't support the feature. It also eliminates the need to modify project files.

Here are a few different ways to set a feature flag:

```cpp
// Set a fallback value
#define PLATFORM_SUPPORTS_SOME_FEATURE 0

// The optional header now may modify it
#if PLATFORM_HEADER_EXISTS(SomeHeader.h)
#include PLATFORM_HEADER(SomeHeader.h)
#endif
```

```cpp
// Set the feature flag based on the existence of the platform header
#if PLATFORM_HEADER_EXISTS(SomeHeader.h)
#define PLATFORM_SUPPORTS_SOME_FEATURE 1
#include PLATFORM_HEADER(SomeHeader.h)
#else
#define PLATFORM_SUPPORTS_SOME_FEATURE 0
#endif
```

```cpp
// Required headers can still make specific feature flags optional.
#include PLATFORM_HEADER(SomeHeader.h)

#if PLATFORM_SUPPORTS_SOME_FEATURE // Not necessarily true
    const auto thing = Platform::SomeFunction();
#endif
```

### The `Platform` Namespace

All platform-specific implementations should live in a sub-namespace named `Platform`. This makes it immediately clear at the call site when a symbol originates from the platform layer:

```cpp
// _platform/<PlatformName>/<PlatformName>MyFunctionImpl.h

namespace Platform
{

inline int MyFunctionImpl() { return 0; }

}
```

```cpp
// MyFunction.cpp

#include <Engine/Core/PlatformAbstraction.h>
#include PLATFORM_HEADER(MyFunctionImpl.h)

int MyFunction()
{
    return Platform::MyFunctionImpl();  // clearly from the platform layer
}
```

Since only one platform is built per target, it's safe for each platform to define its own `Platform::MyFunctionImpl()`, as long as each platform does so only once.

Never add symbols *to* `Platform::` in generic code. The namespace is reserved for names injected by the PAL, which must never collide with names in the generic layer.

### Abstracting Functions

The cleanest approach to platform-specific function implementations is to invoke the platform's implementation from a generic wrapper function:

```cpp
// MyFunction.cpp (generic)

#include PLATFORM_HEADER(MyFunctionImpl.h)

int MyFunction()
{
    return Platform::MyFunctionImpl();
}
```

This removes the injection mechanism from the generic header entirely, and allows the function to be invoked in generic code without `#if` guards.

### Abstracting Classes and Structs

Abstracting classes is more involved than abstracting functions. There are two main techniques: **pimpl** and **class name aliasing**.

#### Pimpl ("Pointer to Implementation")

In most contexts, pimpl is used to _hide_ an implementation. Here, it's used to _inject_ an implementation into an otherwise generic class.

```cpp
// ExampleClass.h

#include <Engine/Core/PlatformAbstraction.h>
#include PLATFORM_HEADER(ExampleClassImpl.h)

class ExampleClass
{
public:
    int someGenericInt = 0;

    void SomeMemberFunction();

    ExampleClass();

private:
    const std::unique_ptr<Platform::ExampleClassImpl> platformImpl;
};
```

```cpp
// ExampleClass.cpp

#include "ExampleClass.h"

ExampleClass::ExampleClass()
    : platformImpl{std::make_unique<Platform::ExampleClassImpl>()}
{}

void ExampleClass::SomeMemberFunction()
{
    platformImpl->DoAThing();
}
```

The internals of `Platform::ExampleClassImpl` are unimportant to the generic code, as long as it satisfies the expected interface.

Feature flags can be used to conditionally compile certain uses of the pimpl, or even to omit it from the class entirely:

```cpp
ExampleClass::ExampleClass()
#if HAS_PLATFORM_IMPLEMENTATION
    : platformImpl{std::make_unique<Platform::ExampleClassImpl>()}
#endif
{}
```

This is the preferred approach for classes with both a generic component and a platform-specific one.

#### Class Name Aliasing

When an _entire class_ is platform-specific, it can be defined entirely in the platform layer and exposed to generic code via a `using` alias:

```cpp
// ExampleClass.h

#include <Engine/Core/PlatformAbstraction.h>
#include PLATFORM_HEADER(ExampleClass.h)

using ExampleClass = Platform::ExampleClass;
```

The alias being located outside of `Platform::` indicates that it is safe to use in generic code.

#### Implementation Contracts

Technically, explicit contracts are unnecessary for platform-specific classes, whether you're using pimpl or class name aliasing. The contract for the class is defined by how it is used in generic code: violations of that "implicit contract" produce clear compiler errors.

However, statically asserting that a class implementation meets an explicit contract can make expectations for new implementations a lot easier to identify. It also makes compilation errors related to the PAL a lot easier to understand.

C++20 concepts work great for this. Define a concept describing the contract, then assert that the implementation meets it, like so:

```cpp
template<typename T>
concept IsExampleClass = requires(T t)
{
    { t.DoThing() }  -> std::same_as<void>;
    { t.GetValue() } -> std::same_as<float>;
};

static_assert(IsExampleClass<Platform::ExampleClass>);
```

Whenever you are abstracting a class of meaningful complexity, statically assert a bespoke contract for it.

### Ensuring Source Files Compile Correctly

`PLATFORM_HEADER()` automatically handles header inclusion and exclusion. Source files, however, are included or excluded from builds based on the Visual Studio or Xcode project configuration.

The Xcode project is configured to handle platform exclusions automatically. However, Visual Studio does not natively support a mechanism to do this. This means that the exclusion of non-Windows `.cpp` files must be configured in Visual Studio manually.

To guard against a platform source file being compiled on the wrong target, add a `static_assert` to it near the top:

```cpp
// _platform/Windows/WindowsFile.cpp

static_assert(ADHOC_WINDOWS);
```

Eventually, I may develop a solution to automate this. But for now, include this assertion in every platform-specific `.cpp` file.
