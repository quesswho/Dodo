#pragma once

#include <chrono>

namespace Dodo {

	class Measure {
	private:
		std::chrono::time_point<std::chrono::steady_clock> m_T1;
	public:

		Measure() 
		{
			m_T1 = std::chrono::steady_clock::now();
		}

		float Stop()
		{
			return std::chrono::duration<float>(std::chrono::steady_clock::now() - m_T1).count();
		}
	};
}