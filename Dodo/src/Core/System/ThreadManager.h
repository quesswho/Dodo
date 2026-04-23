#pragma once

#include <atomic>
#include <deque>
#include <thread>
#include <vector>
#include <functional>
#include <condition_variable>

namespace Dodo {
    class ThreadManager {
      private:
        int m_NumThreads;

        std::mutex m_Mutex;
        std::condition_variable m_WorkConditional;
        std::condition_variable m_MainConditional;
        std::deque<std::function<void()>> m_Queue;

        std::atomic<bool> m_Terminate;
        std::atomic<int> m_NumActiveJobs;

      public:
        std::vector<std::thread> m_WorkThreads;
        ThreadManager(int numThreads);
        ~ThreadManager();

        void WaitMain();
        void Task(std::function<void()> task);
        void Terminate();
        size_t GetQueueSize();

        int GetActiveJobs() const { return m_NumActiveJobs.load(); }

      private:
        void Loop();
    };
} // namespace Dodo