#pragma once

namespace Dodo {
	enum ComponentFlag
	{
		ComponentFlag_None = 0,
		ComponentFlag_ModelComponent = 1 << 0
	};

	DEFINE_ENUM_FLAG_OPERATORS(ComponentFlag);

	inline ComponentFlag& operator |= (ComponentFlag& a, int b) throw() { return (ComponentFlag&)(((_ENUM_FLAG_SIZED_INTEGER<ComponentFlag>::type&)a) |= ((_ENUM_FLAG_SIZED_INTEGER<ComponentFlag>::type)b)); }
}