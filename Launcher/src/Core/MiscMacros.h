#pragma once

// clang-format off

#define CONCATENATE(left, right) left ## right

// clang-format on

#define STRINGIFY_IMPLEMENTATION(macro) #macro
#define STRINGIFY(macro) STRINGIFY_IMPLEMENTATION(macro)
