#pragma once

#include <thread>
#include <vector>

namespace Dodo {

	class ThreadManager {
	private:
		int m_Amount;

		std::mutex m_Mutex;
		std::condition_variable m_WorkConditional;
		std::condition_variable m_MainConditional;
		std::vector<std::function<void()>> m_Queue;

		bool m_Terminate;
	public:
		std::vector<std::thread> m_WorkThreads;
		ThreadManager(int amount);
		~ThreadManager();

		void WaitMain();
		void Task(std::function<void()> task);
		void Terminate();


		void Loop();
	};
}