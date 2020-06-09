#pragma once

#include <chrono>
namespace Dodo {

	class Measure {
	private:
		std::chrono::time_point<std::chrono::steady_clock> m_T1;
	public:

		Measure() 
		{
			m_T1 = std::chrono::high_resolution_clock::now();
		}

		unsigned int Stop()
		{
			return (std::chrono::high_resolution_clock::now() - m_T1).count();
		}
	};
}