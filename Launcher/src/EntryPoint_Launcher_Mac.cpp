#if !(defined(__APPLE__) && defined(__MACH__))
static_assert(false);
#endif // !(defined(__APPLE__) && defined(__MACH__))

#include <cstdlib>
#include <cstring>
#include <iostream>

int main(int argc, char* argv[])
{
    bool isDevelopmentMode = false;
    bool isDebugMode       = false;
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--development") == 0)
            isDevelopmentMode = true;
        else if (strcmp(argv[i], "--debug") == 0)
            isDebugMode = true;
    }
    std::cout << "Development Mode: " << isDevelopmentMode << "\n";
    std::cout << "Debug Mode: " << isDebugMode << "\n";

    return EXIT_SUCCESS;
}
