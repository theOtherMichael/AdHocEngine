#pragma once

#define CONCATENATE(left, right) left##right

#define STRINGIFY_IMPLEMENTATION(macro) #macro

#define STRINGIFY(macro) STRINGIFY_IMPLEMENTATION(macro)
