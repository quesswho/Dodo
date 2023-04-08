#pragma once

namespace Dodo {
	namespace math {
		class Noise {
			float Simplex(float x, float y);

			float SumSimplex(float x, float y, int num_iterations, float persistence, float scale);
		};
	}
}