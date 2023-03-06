#pragma once

#ifdef DD_API_WIN32
#include "Platform/WindowAPI/Win32/Win32Window.h"
namespace Dodo { using Window = Platform::Win32Window; }
#endif
