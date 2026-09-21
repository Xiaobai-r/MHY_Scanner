#include <array>
#include <cstddef>
#include <cstdint>

#include <gtest/gtest.h>

#include "ScreenPixelConverter.hpp"

TEST(ScreenPixelConverter, ConvertsRgba16FloatToBgra8)
{
    constexpr std::array<std::uint16_t, 8> source{
        0x3C00, 0x0000, 0x0000, 0x3C00,
        0x0000, 0x3C00, 0x0000, 0x3C00
    };
    std::array<std::uint8_t, 8> destination{};

    const bool converted = ConvertRgba16FloatToBgra8(
        reinterpret_cast<const std::byte*>(source.data()),
        source.size() * sizeof(std::uint16_t),
        2,
        1,
        destination.data(),
        destination.size(),
        2,
        1);

    EXPECT_TRUE(converted);
    EXPECT_EQ(destination, (std::array<std::uint8_t, 8>{
                               0, 0, 255, 255,
                               0, 255, 0, 255 }));
}

TEST(ScreenPixelConverter, DownscalesWhileConverting)
{
    constexpr std::array<std::uint16_t, 16> source{
        0x0000, 0x0000, 0x0000, 0x3C00,
        0x3800, 0x3800, 0x3800, 0x3C00,
        0x3C00, 0x3C00, 0x3C00, 0x3C00,
        0x0000, 0x0000, 0x0000, 0x3C00
    };
    std::array<std::uint8_t, 8> destination{};

    const bool converted = ConvertRgba16FloatToBgra8(
        reinterpret_cast<const std::byte*>(source.data()),
        source.size() * sizeof(std::uint16_t),
        4,
        1,
        destination.data(),
        destination.size(),
        2,
        1);

    EXPECT_TRUE(converted);
    EXPECT_EQ(destination, (std::array<std::uint8_t, 8>{
                               0, 0, 0, 255,
                               255, 255, 255, 255 }));
}

TEST(ScreenPixelConverter, UsesSourceRowPitch)
{
    std::array<std::uint16_t, 16> source{};
    source[3] = 0x3C00;
    source[8] = 0x3C00;
    source[9] = 0x3C00;
    source[10] = 0x3C00;
    source[11] = 0x3C00;
    std::array<std::uint8_t, 8> destination{};

    const bool converted = ConvertRgba16FloatToBgra8(
        reinterpret_cast<const std::byte*>(source.data()),
        16,
        1,
        2,
        destination.data(),
        destination.size(),
        1,
        2);

    EXPECT_TRUE(converted);
    EXPECT_EQ(destination, (std::array<std::uint8_t, 8>{
                               0, 0, 0, 255,
                               255, 255, 255, 255 }));
}

TEST(ScreenPixelConverter, RejectsInsufficientDestinationBuffer)
{
    constexpr std::array<std::uint16_t, 4> source{ 0x0000, 0x0000, 0x0000, 0x3C00 };
    std::array<std::uint8_t, 3> destination{};

    EXPECT_FALSE(ConvertRgba16FloatToBgra8(
        reinterpret_cast<const std::byte*>(source.data()),
        source.size() * sizeof(std::uint16_t),
        1,
        1,
        destination.data(),
        destination.size(),
        1,
        1));
}

