#pragma once

#include <Core/Common.h>

#ifdef DD_MATH_CRH
#include "Platform/Math/CRH/CRHMat3.h"
namespace Dodo { namespace Math {
		template<class T = float>
		using Mat3x3 = Dodo::Platform::CRHMat3x3<T>;
		using Mat3 = Mat3x3<float>;
}}
#else
#endif
