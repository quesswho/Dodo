#pragma once

namespace Dodo {
	enum ComponentFlag // Order needs to be the same as in the std::variant<...> inside Entity.h!
	{
		ComponentFlag_None = 0,
		ComponentFlag_ModelComponent = 1 << 0,
		ComponentFlag_Rectangle2DComponent = 2 << 0,
	};

	DEFINE_ENUM_FLAG_OPERATORS(ComponentFlag);

	inline ComponentFlag& operator |= (ComponentFlag& a, int b) throw() { return (ComponentFlag&)(((_ENUM_FLAG_SIZED_INTEGER<ComponentFlag>::type&)a) |= ((_ENUM_FLAG_SIZED_INTEGER<ComponentFlag>::type)b)); }
}