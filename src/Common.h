//
// Copyright (c) GANBARION Co., Ltd. All rights reserved.
// This code is licensed under the MIT License (MIT).
//

#pragma once

#include <maya/MGlobal.h>
#include <maya/MRenderTargetManager.h>
#include <cstdint>
#include <stdio.h>
#include "engine/engine.h"


// Maya出力
#define MDisplayInfo(...)		{char _buf[512];_snprintf(_buf,ARRAYSIZE(_buf)-1,__VA_ARGS__);_buf[ARRAYSIZE(_buf)-1]='\0';MGlobal::displayInfo(MString(_buf));}
#define MDisplayWarning(...)	{char _buf[512];_snprintf(_buf,ARRAYSIZE(_buf)-1,__VA_ARGS__);_buf[ARRAYSIZE(_buf)-1]='\0';MGlobal::displayWarning(MString(_buf));}
#define MDisplayError(...)		{char _buf[512];_snprintf(_buf,ARRAYSIZE(_buf)-1,__VA_ARGS__);_buf[ARRAYSIZE(_buf)-1]='\0';MGlobal::displayError(MString(_buf));}
#if _DEBUG
	#define MDisplayDebugInfo(...)	{char _buf[512];_snprintf(_buf,ARRAYSIZE(_buf)-1,__VA_ARGS__);_buf[ARRAYSIZE(_buf)-1]='\0';MGlobal::displayInfo(MString(_buf));}
#else
	#define MDisplayDebugInfo(...)
#endif

// MayaAPIバージョンによる切り替え
#if MAYA_API_VERSION >= 201600
	typedef MHWRender::MRenderTarget* const* MayaRenderTargets;
#else
	typedef MHWRender::MRenderTarget** MayaRenderTargets;
#endif


// シェーディングパス
enum class ShadingPath 
{
	MainPath,
	UIPath,
};


// Math
typedef se::Matrix44 Matrix44;
typedef se::Vector3 Vector3;
typedef se::Vector4 Vector4;
