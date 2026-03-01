#pragma once

#include <Core/Common.h>

#if defined(DD_MATH_COLUMN_MAJOR) && defined(DD_COORDINATE_RIGHT_HANDED)
#include "Platform/Math/CRH/CRHMat4.h"
namespace Dodo { namespace Math {
    template <class T = float> using Mat4x4 = Dodo::Platform::CRHMat4x4<T>;
    using Mat4 = Mat4x4<float>;
}} // namespace Dodo::Math
#else
#endif
