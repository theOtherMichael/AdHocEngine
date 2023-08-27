#include "Enterprise/Add.h"

#include <fmt/format.h>

int Add(int a, int b)
{
    fmt::print("{} + {} is {}", a, b, a + b);
    return a + b;
}
