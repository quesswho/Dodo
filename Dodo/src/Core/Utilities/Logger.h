#pragma once

#if defined(DODO_NO_LOGGER)
#define DD_INFO(...)
#define DD_WARN(...)
#define DD_ERR(...)
#define DD_FATAL(...)
#else

#include <spdlog/sinks/stdout_color_sinks.h>


namespace Dodo {

	class Logger {
	public:
		inline static std::shared_ptr<spdlog::logger> GetLogger() { return m_Logger; }

		Logger();
	private:
		
		static std::shared_ptr<spdlog::logger> m_Logger;
	};
}

// Easy access to log functions
#define DD_INFO(...) Dodo::Logger::GetLogger()->info(__VA_ARGS__)
#define DD_WARN(...) Dodo::Logger::GetLogger()->warn(__VA_ARGS__)
#define DD_ERR(...) Dodo::Logger::GetLogger()->error(__VA_ARGS__)
#define DD_FATAL(...) Dodo::Logger::GetLogger()->error(__VA_ARGS__); __debugbreak()

#endif