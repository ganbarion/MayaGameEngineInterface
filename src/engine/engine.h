//
// Copyright (c) GANBARION Co., Ltd. All rights reserved.
// This code is licensed under the MIT License (MIT).
//

/**
 * Simple Engine
 * namespase se;
 */
#pragma once

#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "D3D11.lib")
#pragma comment(lib, "DXGI.lib")
#pragma comment(lib, "d3dcompiler.lib")

// プラットフォームヘッダ
#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <assert.h>
#include <array>
#include <algorithm>
#include <fstream>
#include <DirectXMath.h>
#include "engine/Graphics/Graphics.h"
#include "engine/Math/Math.h"

#ifdef _DEBUG

	// デバッグ出力用
	inline void DebugPrintf(const char *fmt, ...)
	{
		char buf[1024];
		va_list list;
		va_start(list, fmt);
		vsnprintf(buf, ARRAYSIZE(buf) - 1, fmt, list);
		va_end(list);
		buf[ARRAYSIZE(buf) - 1] = '\0';
		OutputDebugStringA(buf);
	}

	#define Printf(...)			DebugPrintf(__VA_ARGS__)
	//#define Printf(...)			printf(__VA_ARGS__)
	#define Assert(expr)		assert(expr)
#else
	#define Assert(expr)
	#define Printf(...)
#endif

