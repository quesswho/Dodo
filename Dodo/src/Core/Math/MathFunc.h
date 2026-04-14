#pragma once

#include <Core/Common.h>

#include <cmath>
#include <cstring>

#define MATH_PI 3.1415926535897932f

namespace Dodo::Math {

    /**
     * Converts a 32-bit float to a 16-bit half-float (IEEE 754 binary16).
     * Subnormals underflow to zero. Values beyond the half-float range saturate to infinity.
     */
    static inline uint16_t FloatToHalf(float value)
    {
        uint32_t bits;
        memcpy(&bits, &value, sizeof(bits));
        uint16_t sign = (bits >> 16) & 0x8000;
        int32_t exp = ((bits >> 23) & 0xFF) - 127 + 15;
        uint32_t mantissa = bits & 0x7FFFFF;
        if (exp <= 0) return sign;
        if (exp >= 31) return sign | 0x7C00;
        return sign | (uint16_t)(exp << 10) | (uint16_t)(mantissa >> 13);
    }

    static inline constexpr float ToRadians(int degrees)
    {
        return degrees * MATH_PI / 180.0f;
    }

    static inline constexpr float ToRadians(float degrees)
    {
        return degrees * MATH_PI / 180.0f;
    }

    static inline constexpr float ToDegrees(float radians)
    {
        return radians * 180.0f / MATH_PI;
    }

    // Use only positive numbers
    static inline constexpr int FastMod(const int input, const int ceil)
    {
        return input >= ceil ? input % ceil : input;
    }

    static constexpr unsigned int FloorLog2(unsigned int x)
    {
        return x == 1 ? 0 : 1 + FloorLog2(x >> 1);
    }
} // namespace Dodo::Math