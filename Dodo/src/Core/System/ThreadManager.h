#pragma once

#include <thread>
#include <vector>

namespace Dodo {

	class ThreadManager {
	private:
		std::vector<std::thread> m_WorkThreads;
		int m_Amount;
		int m_Index;
	public:
		ThreadManager(int amount);
		
		template<typename F, typename... Args>
		void Task(F& ret, Args&&... args);
		
		void Finish();
	};
}