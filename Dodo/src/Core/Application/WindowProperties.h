#pragma once

#include <cstdint>
#include <string>

namespace Dodo {
    enum WindowFlags {
		DodoWindowFlags_NONE			= 0,
		DodoWindowFlags_FULLSCREEN		= 1 << 0,
		DodoWindowFlags_VSYNC			= 1 << 1,
		DodoWindowFlags_IMGUI			= 1 << 2,
		DodoWindowFlags_IMGUIDOCKING	= 1 << 3,
		DodoWindowFlags_BACKFACECULL	= 1 << 4,
		DodoWindowFlags_SERIALIZESCENE  = 1 << 5,
	};

	inline WindowFlags operator|(WindowFlags a, WindowFlags b)
	{
		return static_cast<WindowFlags>(
			static_cast<uint32_t>(a) | static_cast<uint32_t>(b)
		);
	}

	inline WindowFlags& operator|=(WindowFlags& a, WindowFlags b)
	{
		a = a | b;
		return a;
	}

	struct WindowProperties {
		WindowProperties() : m_Width(0), m_Height(0), m_Title("Dodo Engine"), m_Flags(DodoWindowFlags_NONE) {}
		uint32_t m_Width;
		uint32_t m_Height;
		const char* m_Title;

		WindowFlags m_Flags;
	};

	struct PCSpecifications {
		size_t m_TotalPhysicalMemory;
		std::string m_CpuBrand;
	};
}