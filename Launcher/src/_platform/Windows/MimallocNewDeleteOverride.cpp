// Force mimalloc.dll into the exe's import table before EditorDev.dll and EngineDev.dll so
// mimalloc-redirect.dll can patch all allocators before any other DLL's static initializers run.
// Equivalent to the "Force Symbol References" / mi_version entry in the pre-CMake VS project settings.
// https://microsoft.github.io/mimalloc/overrides.html
#pragma comment(linker, "/include:mi_version")

#include <mimalloc-new-delete.h>
