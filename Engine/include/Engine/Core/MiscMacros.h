#pragma once

// clang-format off

#define CONCATENATE(left, right) left ## right

// clang-format on

#define STRINGIFY_IMPLEMENTATION(macro) #macro

#define STRINGIFY(macro) STRINGIFY_IMPLEMENTATION(macro)

/// Create a bit field with the nth bit set
#define BIT(n) (1ull << (n))
