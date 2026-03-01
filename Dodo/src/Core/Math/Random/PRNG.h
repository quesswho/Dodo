#pragma once

#include <Core/Common.h>

#include <chrono>
#include <time.h>

namespace Dodo { namespace Math {

    //
    // (P)seudo (R)andom (N)umber (G)enerator
    //

    static Random64 s_Random(std::chrono::system_clock::now().time_since_epoch().count());

    class Random64 {
        Xoshiro256Plus m_Generator;

      public:
        Random64(uint64 seed) : m_Generator(GenState(seed))
        {}

        inline constexpr uint64 Next()
        {
            return m_Generator.Next();
        }

      private:
        constexpr uint64 *GenState(uint64 seed)
        {
            uint64 *state = new uint64[4];
            state[0] = seed;
            // Splitmix64 for generating states
            for (int i = 0; i < 3; i++)
            {
                uint64 z = (state[i] + 0x9e3779b97f4a7c15);
                z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
                z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
                state[i] = z ^ (z >> 31);
            }
            return state;
        }
    };

    // Original Implementation https://prng.di.unimi.it/xoshiro256plus.c
    class Xoshiro256Plus {
        uint64 *m_State;

      public:
        // (uint64 state[4])
        Xoshiro256Plus(uint64 *state) : m_State(state)
        {}

        // Get next random number
        inline constexpr uint64 Next() noexcept
        {
            const uint64 result = m_State[0] + m_State[3];
            const uint64 t = m_State[1] << 17;
            m_State[2] ^= m_State[0];
            m_State[3] ^= m_State[1];
            m_State[1] ^= m_State[2];
            m_State[0] ^= m_State[3];
            m_State[2] ^= t;
            m_State[3] = RotL(m_State[3], 45);
            return result;
        }

      private:
        // Rotate left
        static constexpr uint64 RotL(const uint64 x, const int s) noexcept
        {
            return (x << s) | (x >> (64 - s));
        }
    };
}} // namespace Dodo::Math