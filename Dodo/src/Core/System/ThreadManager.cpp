#include "pch.h"
#include "ThreadManager.h"

namespace Dodo {

	ThreadManager::ThreadManager(int amount)
		: m_Amount(amount), m_Index(0)
	{
		m_WorkThreads.resize(m_Amount);
	}


	// To get a return use CLASS** instance in the arguments and execute it as Task(&Dodo::func, &instance, ...)
	template<typename F, typename... Args>
	void ThreadManager::Task(F& func, Args&&... args)
	{
		while (m_WorkThreads[m_Index % m_Amount].joinable())
		{
			index++;
		}
		m_WorkThreads[m_Index % m_Amount] = std::thread(func,args);

		/*	TODO: Find out if this is faster

		while (m_WorkThreads[m_Index].joinable())
		{
			m_Index = m_Index == m_Amount ? 0 : m_Index + 1;
		}
		m_WorkThreads[m_Index] = std::thread(func, args);

		*/
	}

	void ThreadManager::Finish()
	{
		for (int i = 0; i < m_Amount; i++)
		{
			if (m_WorkThreads[i].joinable()) m_WorkThreads[i].join();
		}
	}

}
