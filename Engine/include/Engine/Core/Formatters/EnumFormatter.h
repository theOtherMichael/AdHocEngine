#pragma once

#include <fmt/format.h>
#include <magic_enum/magic_enum.hpp>

#include <type_traits>

#define INJECT_ENUM_FORMATTER                                                                                          \
    template <typename E>                                                                                              \
        requires std::is_enum_v<E>                                                                                     \
    constexpr auto format_as(E e)                                                                                      \
    {                                                                                                                  \
        return magic_enum::enum_name(e);                                                                               \
    }
