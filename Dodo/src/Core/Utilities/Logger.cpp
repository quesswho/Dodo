#include "Logger.h"

namespace Dodo {

	std::shared_ptr<spdlog::logger> Logger::m_Logger;

	Logger::Logger()
	{
		m_Logger = spdlog::stdout_color_mt("console");
		m_Logger->set_pattern("%^[%T %l]: %^%v%$");
	}
}