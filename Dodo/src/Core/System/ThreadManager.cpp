#include "pch.h"
#include "ThreadManager.h"
#include <mutex>

namespace Dodo {

	ThreadManager::ThreadManager(int amount)
		: m_Terminate(false)
	{
		m_WorkThreads.resize(amount);
		for (int i = 0; i < amount; i++)
			m_WorkThreads[i] = std::thread(&ThreadManager::Loop, this);
	}

	ThreadManager::~ThreadManager()
	{
		Terminate();
	}


	void ThreadManager::WaitMain()
	{
		std::unique_lock<std::mutex> lock(m_Mutex);
		m_MainConditional.wait(lock, [&]() {		// Make Main thread wait until workthreads are done
			return m_Queue.empty();
			});
	}

	void ThreadManager::Task(std::function<void()> task)
	{
		{
			std::unique_lock<std::mutex> lock(m_Mutex);
			m_Queue.push_back(task);
		}
		m_WorkConditional.notify_one(); // Tell one thread to check the queue
	}
	

	void ThreadManager::Terminate()
	{
		{
			std::unique_lock<std::mutex> lock(m_Mutex);
			m_Terminate = true; // Flag to tell threads to wake up and exit
		}

		m_WorkConditional.notify_all();

		for (std::thread& thread : m_WorkThreads)
			thread.join();

		m_WorkThreads.clear();
	}

	void ThreadManager::Loop()
	{
		std::function<void()> job;
		while (true)
		{
			{
				std::unique_lock<std::mutex> lock(m_Mutex);
				m_WorkConditional.wait(lock, [&]() { 
					return !m_Queue.empty() || m_Terminate;
					});
				if (m_Terminate) return;
				job = m_Queue.back();
				m_Queue.pop_back();
			}
			job();
		}
	}
}
