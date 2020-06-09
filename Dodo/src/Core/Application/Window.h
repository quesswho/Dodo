#pragma once

#ifdef DD_API_WIN32
#include "Platform/WindowAPI/Win32/Win32Window.h"
namespace Dodo { typedef Platform::Win32Window Window; }
#endif
