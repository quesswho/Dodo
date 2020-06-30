#pragma once

#include "Core/Common.h"

#ifdef DD_MATH_CRH
#include "Platform/Math/CRH/CRHMat2.h"
namespace Dodo { namespace Math {
		template<class T = float>
		using Mat2x2 = Dodo::Platform::CRHMat2x2<T>;
		using Mat2 = Mat2x2<float>;
}}
#else
#endif