#include <mimalloc-new-delete.h>

// On Windows, we override new/delete in addition to malloc/free for performance reasons.
// https://microsoft.github.io/mimalloc/overrides.html, "Dynamic Override on Windows"