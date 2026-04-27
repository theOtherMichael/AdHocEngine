#include <asl/casts.h>

#include <gtest/gtest.h>

#include <complex>
#include <limits>
#include <type_traits>

// Shamelessly adapted from https://github.com/microsoft/GSL/ (MIT License)

namespace asl
{

TEST(CastTests, NarrowCast)
{
    const auto fitsInChar            = int{120};
    const auto tooBigForUnsignedChar = int{300};

    auto c = asl::narrow_cast<char>(fitsInChar);
    static_assert(std::is_same_v<decltype(c), char>);
    EXPECT_TRUE(c == 120);

    auto uc = asl::narrow_cast<unsigned char>(tooBigForUnsignedChar);
    static_assert(std::is_same_v<decltype(uc), unsigned char>);
    EXPECT_TRUE(uc == 44);
}

TEST(CastTests, Narrow)
{
    const auto fitsInChar = int{120};

    auto c = asl::narrow<char>(fitsInChar);
    static_assert(std::is_same_v<decltype(c), char>);
    EXPECT_TRUE(c == 120);

    const auto tooBigForUnsignedChar = int{300};

    EXPECT_THROW(asl::narrow<char>(tooBigForUnsignedChar), asl::narrowing_error);

    constexpr auto int32Max = std::numeric_limits<int32_t>::max();
    constexpr auto int32Min = std::numeric_limits<int32_t>::min();

    EXPECT_TRUE(asl::narrow<uint32_t>(int32_t{0}) == 0);
    EXPECT_TRUE(asl::narrow<uint32_t>(int32_t{1}) == 1);
    EXPECT_TRUE(asl::narrow<uint32_t>(int32Max) == static_cast<uint32_t>(int32Max));

    EXPECT_THROW(asl::narrow<uint32_t>(int32_t{-1}), asl::narrowing_error);
    EXPECT_THROW(asl::narrow<uint32_t>(int32Min), asl::narrowing_error);

    const auto negative = -42;
    EXPECT_THROW(asl::narrow<unsigned>(negative), asl::narrowing_error);

    EXPECT_TRUE(asl::narrow<std::complex<float>>(std::complex<double>(4, 2)) == std::complex<float>(4, 2));
    EXPECT_THROW(asl::narrow<std::complex<float>>(std::complex<double>(4.2)), asl::narrowing_error);

    EXPECT_TRUE(asl::narrow<int>(float(1)) == 1);
}

} // namespace asl
