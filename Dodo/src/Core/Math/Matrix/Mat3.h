#pragma once

#include <Core/Common.h>

#if defined(DD_MATH_COLUMN_MAJOR) && defined(DD_COORDINATE_RIGHT_HANDED)
#include "Platform/Math/CRH/CRHMat3.h"
namespace Dodo { namespace Math {
    template <class T = float> using Mat3x3 = Dodo::Platform::CRHMat3x3<T>;
    using Mat3 = Mat3x3<float>;
}} // namespace Dodo::Math
#else
#endif
