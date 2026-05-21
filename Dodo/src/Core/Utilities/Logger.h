#pragma once

#include <Core/Common.h>

#if defined(DODO_NO_LOGGER)
#define DD_INFO(...)
#define DD_WARN(...)
#define DD_ERR(...)
#define DD_FATAL(...)
#define DD_ASSERT(cond, ...)
#else

#include <spdlog/sinks/stdout_color_sinks.h>

namespace Dodo {

    class Logger {
      public:
        inline static Ref<spdlog::logger> GetLogger() { return m_Logger; }

        template <typename... Args>
        static void ErrorHandler(const char* file, int line, std::string message, const Args&... args)
        {
            std::string finalmsg = "at " + std::string(file) + ":" + std::to_string(line) + " " + message;
            m_Logger->error(finalmsg, args...);
        }

        Logger();

      private:
        static Ref<spdlog::logger> m_Logger;
    };
} // namespace Dodo

#if defined(_WIN32)
#define DD_BREAK() __debugbreak()
#else
#include <csignal>
#define DD_BREAK() raise(SIGTRAP)
#endif

// Easy access to log functions
#define DD_INFO(...) Dodo::Logger::GetLogger()->info(__VA_ARGS__)
#define DD_WARN(...) Dodo::Logger::GetLogger()->warn(__VA_ARGS__)
#define DD_ERR(...)  Dodo::Logger::ErrorHandler(__FILE__, __LINE__, __VA_ARGS__)
#define DD_FATAL(...)                                                                                                  \
    Dodo::Logger::ErrorHandler(__FILE__, __LINE__, __VA_ARGS__);                                                       \
    DD_BREAK()

#define DD_ASSERT(cond, ...)                                                                                           \
    do {                                                                                                               \
        if (!(cond)) {                                                                                                 \
            Dodo::Logger::ErrorHandler(__FILE__, __LINE__, "Assertion failed: " #cond ". " __VA_ARGS__);              \
            DD_BREAK();                                                                                                \
        }                                                                                                             \
    } while (0)

#endif