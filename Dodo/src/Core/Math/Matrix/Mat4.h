#pragma once

#include "Core/Common.h"

#ifdef DD_MATH_CRH
#include "Platform/Math/CRH/CRHMat4.h"
namespace Dodo { namespace Math { 
		template<class T = float>
		using Mat4x4 = Dodo::Platform::CRHMat4x4<T>;
		using Mat4 = Mat4x4<float>;
}}
#else
#endif
