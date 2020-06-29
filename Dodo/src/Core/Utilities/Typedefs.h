#pragma once

typedef unsigned char uchar; // 0 - 255
typedef unsigned short int ushort; // 0 - 65 535
typedef unsigned int uint;   // 0 - 4 294 967 295
typedef unsigned long int ulong; // 0 - 4 294 967 295
typedef unsigned long long  int uint64; // 0 - 18 446 744 073 709 551 615
typedef long long int int64; // -9 223 372 036 854 775 807 - 9 223 372 036 854 775 807

#if !defined(DD_API_OPENGL) //|| !defined(DD_API_DX11)
#define DD_API_OPENGL
#endif