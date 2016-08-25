//
// Copyright (c) GANBARION Co., Ltd. All rights reserved.
// This code is licensed under the MIT License (MIT).
//

#pragma once

#include <windows.h>
#include <d3d11.h>

#ifndef COMPTR_RELEASE
	#define COMPTR_RELEASE(p)	if(p) { p->Release(); p = nullptr; }
#endif

#ifndef THROW_IF_FAILED
	#define THROW_IF_FAILED(hr) if (FAILED(hr)) { throw; }
#endif

namespace se
{
	// 頂点アトリビュート
	enum VertexAttribute
	{
		VERTEX_ATTR_POSITION		= 0,
		VERTEX_ATTR_NORMAL			= 1,
		VERTEX_ATTR_COLOR			= 2,
		VERTEX_ATTR_TEXCOORD0		= 3,
		VERTEX_ATTR_TEXCOORD1		= 4,
		VERTEX_ATTR_TEXCOORD2		= 5,
		VERTEX_ATTR_TEXCOORD3		= 6,
		VERTEX_ATTR_TANGENT			= 7,
		VERTEX_ATTR_BITANGENT		= 8,
	};
	enum VertexAttributeFlag
	{
		VERTEX_ATTR_FLAG_POSITION		= 1 << VERTEX_ATTR_POSITION,
		VERTEX_ATTR_FLAG_NORMAL			= 1 << VERTEX_ATTR_NORMAL,
		VERTEX_ATTR_FLAG_COLOR			= 1 << VERTEX_ATTR_COLOR,
		VERTEX_ATTR_FLAG_TEXCOORD0		= 1 << VERTEX_ATTR_TEXCOORD0,
		VERTEX_ATTR_FLAG_TEXCOORD1		= 1 << VERTEX_ATTR_TEXCOORD1,
		VERTEX_ATTR_FLAG_TEXCOORD2		= 1 << VERTEX_ATTR_TEXCOORD2,
		VERTEX_ATTR_FLAG_TEXCOORD3		= 1 << VERTEX_ATTR_TEXCOORD3,
		VERTEX_ATTR_FLAG_TANGENT		= 1 << VERTEX_ATTR_TANGENT,
		VERTEX_ATTR_FLAG_BITANGENT		= 1 << VERTEX_ATTR_BITANGENT,
	};
	typedef uint32_t VertexAttributeFlags;

	// UV数
	const uint32_t VERTEX_ATTR_TEXCOORD_NUM = 4;


	/**
	 * プリミティブタイプ
	 */
	enum PrimitiveType
	{
		PRIMITIVE_TYPE_TRIANGLE_LIST,
		PRIMITIVE_TYPE_LINE_LIST,

		PRIMITIVE_TYPE_UNKNOWN,
	};

	/**
	 * テクスチャアドレスモード
	 */
	enum TextureAddressMode
	{
		TAM_WRAP,
		TAM_CLAMP,
		TAM_MIRROR,
		TAM_BORDER,
	};

	/**
	 * サンプラフィルタ
	 */
	enum SamplerFilter
	{
		FILTER_POINT,
		FILTER_BILINEAR,
		FILTER_TRILINEAR,
		FILTER_ANISOTROPIC_POINT,
		FILTER_ANISOTROPIC_LINEAR,
	};

	/**
	 * 比較関数
	 */
	enum CompareFunction
	{
		CF_LESS,
		CF_LESSEQUAL,
		CF_GREATER,
		CF_GREATEREQUAL,
		CF_EQUAL,
		CF_NOTEQUAL,
		CF_NEVER,
		CF_ALWAYS,
	};
}