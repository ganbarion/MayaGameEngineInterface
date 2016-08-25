//
// Copyright (c) GANBARION Co., Ltd. All rights reserved.
// This code is licensed under the MIT License (MIT).
//

#pragma once 

#include "engine/Graphics/GraphicsCommon.h"
#include "engine/Graphics/GraphicsContext.h"

namespace se
{
	/**
	 * サンプラステート
	 */
	class SamplerState
	{
		friend class GraphicsContext;

	private:
		ID3D11SamplerState* state_;

	public:
		SamplerState();
		~SamplerState();
		SamplerState(const SamplerState&);

		void Create(SamplerFilter filter = FILTER_POINT,
						TextureAddressMode AddressU = TAM_CLAMP,
						TextureAddressMode AddressV = TAM_CLAMP,
						TextureAddressMode AddressW = TAM_CLAMP,
						CompareFunction ComparisonFunc = CF_NEVER,
						int32_t MipLODBias = 0,
						int32_t Anisotropy = 0,
						uint32_t BorderColor = 0);

		void Destroy();
	};


	/**
	 * デプスステンシルステート
	 */
	class DepthStencilState
	{
		friend class GraphicsContext;

	public:
		enum DepthType
		{
			Disable,				// テストなし
			Enable,					// テストあり
			WriteDisable,			// デプス書き込みあり、比較なし
			WriteEnable,			// デプス書き込みあり、比較あり
			WriteEnableEqual,		// デプス書き込みあり、比較は等しい場合のみパス
			WriteEnableReverse,		// デプス書き込みあり、反対方向に比較あり
		};
		static const int DepthTypeNum = WriteEnableReverse + 1;

	private:
		static DepthStencilState templates_[DepthTypeNum];

	public:
		static void Initialize();
		static const DepthStencilState& Get(DepthType type) { return templates_[type]; };

	private:
		ID3D11DepthStencilState* state_;

	private:
		DepthStencilState();
		~DepthStencilState();
		DepthStencilState(const DepthStencilState&);	// コピー禁止
	};


	/**
	 * ラスタライザステート
	 */
	class RasterizerState
	{
		friend class GraphicsContext;

	public:
		enum RasterizerType
		{
			NoCull,
			BackFaceCull,
			FrontFaceCull,
			WireFrame,
		};
		static const int RasterizerTypeNum = WireFrame + 1;

	private:
		static RasterizerState templates_[RasterizerTypeNum];

	public:
		static void Initialize();
		static const RasterizerState& Get(RasterizerType type) { return templates_[type]; };

	private:
		ID3D11RasterizerState* state_;

	private:
		RasterizerState();
		~RasterizerState();
		RasterizerState(const RasterizerState&);	// コピー禁止
	};


	/**
	 * ブレンドステート
	 */
	class BlendState
	{
		friend class GraphicsContext;

	public:
		enum BlendType
		{
			Opacity,
			Translucent,
			Additive,
			Modulate,
			Subtruct,
		};
		static const int32_t BlendTypeNum = Subtruct + 1;

	private:
		static BlendState templates_[BlendTypeNum];

	public:
		static void Initialize();
		static void Finalize();
		static const BlendState& Get(BlendType type) { return templates_[type]; };

	private:
		ID3D11BlendState* state_;

	private:
		BlendState();
		~BlendState();
		BlendState(const BlendState&);	// コピー禁止

		void Create(BlendType type);
		void Destroy();
	};
}