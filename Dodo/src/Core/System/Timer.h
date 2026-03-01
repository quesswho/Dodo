#pragma once

#include <chrono>

namespace Dodo {

    class Timer {
      private:
        std::chrono::time_point<std::chrono::high_resolution_clock> m_Start;

        float m_LastElapsed;

      public:
        Timer() : m_LastElapsed(0.0f)
        {}

        void Start()
        {
            m_Start = std::chrono::high_resolution_clock::now();
        }
        float End()
        {
            return std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - m_Start).count();
        } // Returns x seconds after Start() was called
    };
} // namespace Dodo