#pragma once

#define STRINGIFY_IMPLEMENTATION(text) #text

/// Enclose text in quotes
#define STRINGIFY(text) STRINGIFY_IMPLEMENTATION(text)

// clang-format off

/// Concatenate two sections of text
#define CONCATENATE(left, right) left##right

// clang-format on

/// Create a bit field with the nth bit set
#define BIT(n) (1ull << (n))
