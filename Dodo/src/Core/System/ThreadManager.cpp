#include "ThreadManager.h"
#include "pch.h"

#include <mutex>

namespace Dodo {

    ThreadManager::ThreadManager(int numThreads) : m_Terminate(false), m_NumThreads(numThreads), m_NumActiveJobs(0)
    {
        m_WorkThreads.resize(numThreads);
        for (int i = 0; i < numThreads; i++)
            m_WorkThreads[i] = std::thread(&ThreadManager::Loop, this);
    }

    ThreadManager::~ThreadManager()
    {
        Terminate();
    }

    void ThreadManager::WaitMain()
    {
        std::unique_lock<std::mutex> lock(m_Mutex);
        m_MainConditional.wait(lock, [&]() { // Make Main thread wait until workthreads are done
            return m_Queue.empty() && m_NumActiveJobs.load() == 0;
        });
    }

    void ThreadManager::Task(std::function<void()> task)
    {
        {
            std::unique_lock<std::mutex> lock(m_Mutex);
            m_Queue.push_back(std::move(task));
        }
        m_WorkConditional.notify_one(); // Tell one thread to check the queue
    }

    size_t ThreadManager::GetQueueSize()
    {
        std::unique_lock<std::mutex> lock(m_Mutex);
        return m_Queue.size();
    }

    void ThreadManager::Terminate()
    {
        {
            std::unique_lock<std::mutex> lock(m_Mutex);
            m_Terminate.store(true);
        }

        m_WorkConditional.notify_all();

        for (std::thread& thread : m_WorkThreads)
            if(thread.joinable()) thread.join();

        m_WorkThreads.clear();
    }

    void ThreadManager::Loop()
    {
        while (true) {
            std::function<void()> job;
            {
                std::unique_lock<std::mutex> lock(m_Mutex);
                m_WorkConditional.wait(lock, [&]() { return !m_Queue.empty() || m_Terminate.load(); });
                if (m_Terminate.load()) return;

                if (m_Terminate.load() && m_Queue.empty()) {
                    return;
                }

                if (!m_Queue.empty()) {
                    job = std::move(m_Queue.front());
                    m_Queue.pop_front();
                    m_NumActiveJobs.fetch_add(1);
                }
            }
            if (job) {
                job();
                int remaining = m_NumActiveJobs.fetch_sub(1) - 1;
                if (remaining == 0) {
                    // Double-check that queue is also empty before notifying
                    std::unique_lock<std::mutex> lock(m_Mutex);
                    if (m_Queue.empty()) {
                        m_MainConditional.notify_all();
                    }
                }
            }
        }
    }
} // namespace Dodo
