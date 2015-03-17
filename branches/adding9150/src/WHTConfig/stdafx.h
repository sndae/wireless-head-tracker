#pragma once

#define _USE_MATH_DEFINES

// C
#include <cstdint>
#include <cassert>
#include <cmath>

// C++
#include <string>
#include <vector>
#include <algorithm>
#include <set>

// Windows
#define NOMINMAX
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <shellapi.h>

// DirectX
#ifdef _DEBUG
# define D3D_DEBUG_INFO
#endif

#include <d3d9.h>
#include <d3dx9.h>
