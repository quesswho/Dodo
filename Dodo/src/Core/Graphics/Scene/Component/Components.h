#pragma once

#include <type_traits>

namespace Dodo {
	enum ComponentFlag // Order needs to be the same as in the std::variant<...> inside Entity.h!
	{
		ComponentFlag_None = 0,
		ComponentFlag_ModelComponent = 1 << 0,
		ComponentFlag_Rectangle2DComponent = 2 << 0,
	};

	inline ComponentFlag operator|(ComponentFlag a, ComponentFlag b) noexcept {
        using underlying = std::underlying_type_t<ComponentFlag>;
        return static_cast<ComponentFlag>(static_cast<underlying>(a) | static_cast<underlying>(b));
    }

	inline ComponentFlag& operator|=(ComponentFlag& a, ComponentFlag b) noexcept {
        a = a | b;
        return a;
    }
}