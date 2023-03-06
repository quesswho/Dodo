#pragma once

#include <memory>

using uchar = unsigned char; // 0 - 255
using ushort = unsigned short int; // 0 - 65 535
using uint = unsigned int;   // 0 - 4 294 967 295
using ulong = unsigned long int; // 0 - 4 294 967 295
using uint64 = unsigned long long  int; // 0 - 18 446 744 073 709 551 615
using int64 = long long int; // -9 223 372 036 854 775 807 - 9 223 372 036 854 775 807

template<typename T> 
using Ref = std::shared_ptr<T>;

#if !defined(DD_API_OPENGL) //|| !defined(DD_API_DX11)
#define DD_API_OPENGL
#define DD_MATH_CRH
#endif