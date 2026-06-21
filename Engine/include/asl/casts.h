#pragma once

#include "concepts.h"

#include <exception>
#include <utility>

namespace asl
{

// Shamelessly adapted from https://github.com/microsoft/GSL/ (MIT License)

template <class T, class U>
constexpr T narrow_cast(U&& u) noexcept
{
    return static_cast<T>(std::forward<U>(u));
}

struct narrowing_error : public std::exception
{
    const char* what() const noexcept override { return "narrowing error"; }
};

template <asl::arithmetic T, typename U>
constexpr T narrow(U u)
{
    constexpr const bool isDifferentSignedness = (std::is_signed<T>::value != std::is_signed<U>::value);

    const auto t = narrow_cast<T>(u);

    // Note: NaN will always throw, since NaN != NaN
    if (static_cast<U>(t) != u || (isDifferentSignedness && ((t < T{}) != (u < U{}))))
    {
        throw narrowing_error{};
    }

    return t;
}

template <asl::non_arithmetic T, typename U>
constexpr T narrow(U u)
{
    const T t = narrow_cast<T>(u);

    if (static_cast<U>(t) != u)
    {
        throw narrowing_error{};
    }

    return t;
}

} // namespace asl
