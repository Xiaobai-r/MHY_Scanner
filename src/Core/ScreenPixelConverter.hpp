#pragma once

#include <algorithm>
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>

namespace ScreenPixelConverterDetail
{
inline float HalfToFloat(std::uint16_t value)
{
    const std::uint32_t sign = static_cast<std::uint32_t>(value & 0x8000) << 16;
    std::uint32_t exponent = (value >> 10) & 0x1F;
    std::uint32_t mantissa = value & 0x03FF;
    std::uint32_t bits{};

    if (exponent == 0)
    {
        if (mantissa == 0)
        {
            bits = sign;
        }
        else
        {
            int normalizedExponent = -14;
            while ((mantissa & 0x0400) == 0)
            {
                mantissa <<= 1;
                --normalizedExponent;
            }
            mantissa &= 0x03FF;
            bits = sign |
                (static_cast<std::uint32_t>(normalizedExponent + 127) << 23) |
                (mantissa << 13);
        }
    }
    else if (exponent == 0x1F)
    {
        bits = sign | 0x7F800000 | (mantissa << 13);
    }
    else
    {
        bits = sign | ((exponent + 112) << 23) | (mantissa << 13);
    }

    return std::bit_cast<float>(bits);
}

inline std::uint8_t FloatToUnorm8(float value)
{
    if (std::isnan(value))
    {
        return 0;
    }
    const float clamped = std::clamp(value, 0.0F, 1.0F);
    return static_cast<std::uint8_t>(clamped * 255.0F + 0.5F);
}
}

inline bool ConvertRgba16FloatToBgra8(
    const std::byte* source,
    std::size_t sourceRowPitch,
    std::uint32_t sourceWidth,
    std::uint32_t sourceHeight,
    std::uint8_t* destination,
    std::size_t destinationSize,
    std::uint32_t destinationWidth,
    std::uint32_t destinationHeight)
{
    if (!source || !destination || sourceWidth == 0 || sourceHeight == 0 ||
        destinationWidth == 0 || destinationHeight == 0 ||
        sourceRowPitch < static_cast<std::size_t>(sourceWidth) * 8 ||
        destinationSize < static_cast<std::size_t>(destinationWidth) * destinationHeight * 4)
    {
        return false;
    }

    for (std::uint32_t y = 0; y < destinationHeight; ++y)
    {
        const std::uint32_t sourceY = static_cast<std::uint32_t>(
            static_cast<std::uint64_t>(y) * sourceHeight / destinationHeight);
        const auto* sourceRow = reinterpret_cast<const std::uint16_t*>(
            source + static_cast<std::size_t>(sourceY) * sourceRowPitch);
        auto* destinationRow = destination + static_cast<std::size_t>(y) * destinationWidth * 4;

        for (std::uint32_t x = 0; x < destinationWidth; ++x)
        {
            const std::uint32_t sourceX = static_cast<std::uint32_t>(
                static_cast<std::uint64_t>(x) * sourceWidth / destinationWidth);
            const auto* sourcePixel = sourceRow + static_cast<std::size_t>(sourceX) * 4;
            auto* destinationPixel = destinationRow + static_cast<std::size_t>(x) * 4;

            destinationPixel[0] = ScreenPixelConverterDetail::FloatToUnorm8(
                ScreenPixelConverterDetail::HalfToFloat(sourcePixel[2]));
            destinationPixel[1] = ScreenPixelConverterDetail::FloatToUnorm8(
                ScreenPixelConverterDetail::HalfToFloat(sourcePixel[1]));
            destinationPixel[2] = ScreenPixelConverterDetail::FloatToUnorm8(
                ScreenPixelConverterDetail::HalfToFloat(sourcePixel[0]));
            destinationPixel[3] = 255;
        }
    }

    return true;
}

