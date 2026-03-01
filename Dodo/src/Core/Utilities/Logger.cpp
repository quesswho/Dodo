#include "pch.h"

#if !defined(DODO_NO_LOGGER)
#include "Logger.h"

namespace Dodo {

    Ref<spdlog::logger> Logger::m_Logger;

    Logger::Logger()
    {
        m_Logger = spdlog::stdout_color_mt("console");
        m_Logger->set_pattern("%^[%T %l]: %^%v%$");
    }
} // namespace Dodo
#endif