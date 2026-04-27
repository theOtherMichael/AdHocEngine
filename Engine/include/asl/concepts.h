#pragma once

#include <type_traits>

namespace asl
{

template <typename T>
concept arithmetic = std::is_arithmetic_v<T>;

template <typename T>
concept non_arithmetic = !std::is_arithmetic_v<T>;

} // namespace asl
