#pragma once

namespace Dodo::Math {
    class Noise {
      public:
        static float Simplex(float x, float y);

        static float SumSimplex(float x, float y, int num_iterations, float persistence, float scale);
    };
} // namespace Dodo::Math