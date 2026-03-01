#pragma once

#include <cstdint>
#include <string>

namespace Dodo {
    struct WindowSettings {
        bool fullscreen = false;
        bool vsync = false;
        bool imgui = false;
        bool imguiDocking = false;
        bool backfaceCull = true;
        bool serializeScene = false;
    };

    struct WindowProperties {
        WindowProperties()
            : m_Width(0), m_Height(0), m_FrameBufferWidth(0), m_FrameBufferHeight(0), m_PosX(0), m_PosY(0),
              m_Title("Dodo Engine")
        {}
        uint32_t m_Width;
        uint32_t m_Height;
        uint32_t m_FrameBufferWidth;
        uint32_t m_FrameBufferHeight;
        uint32_t m_PosX;
        uint32_t m_PosY;

        const char *m_Title;

        WindowSettings m_Settings;
    };

    struct PCSpecifications {
        size_t m_TotalPhysicalMemory;
        std::string m_CpuBrand;
    };
} // namespace Dodo