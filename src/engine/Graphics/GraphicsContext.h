//
// Copyright (c) GANBARION Co., Ltd. All rights reserved.
// This code is licensed under the MIT License (MIT).
//

#pragma once 

#include "engine/Graphics/GraphicsCommon.h"
#include "engine/Math/Math.h"

namespace se
{
	struct VertexInputLayout;
	class VertexShader;
	class PixelShader;
	class ConstantBuffer;
	class GPUResource;
	class VertexBuffer;
	class IndexBuffer;
	class ColorBuffer;
	class DepthStencilBuffer;
	class SamplerState;
	class BlendState;
	class DepthStencilState;
	class RasterizerState;

	/**
	 * グラフィクスコンテキスト
	 */
	class GraphicsContext
	{
	private:
		ID3D11DeviceContext* deviceContext_;

	public:
		GraphicsContext();
		~GraphicsContext();

		void Initialize(ID3D11DeviceContext* context);
		void Finalize();
		ID3D11DeviceContext* GetDeviceContext() { return deviceContext_; }

		// RenderTarget
		void SetRenderTarget(const ColorBuffer* colorBuffers, uint32_t count, const DepthStencilBuffer* depthStencil);
		void ClearRenderTarget(const ColorBuffer& target, const float4& color);
		void ClearDepthStencil(const DepthStencilBuffer& target, float depth = 1.0f);

		// Shader
		void SetVertexShader(const VertexShader& shader);
		void SetPixelShader(const PixelShader& shader);

		// RenderStates
		void SetBlendState(const BlendState& blend);
		void SetDepthStencilState(const DepthStencilState& depthStencil, uint32_t stencilRef = 0u);
		void SetRasterizerState(const RasterizerState& raster);
		void SetPrimitiveType(PrimitiveType type);
		void SetViewport(const Rect& rect, float minDepth = 0.0f, float maxDepth = 1.0f);
		void SetScissorRect(const Rect& rect);

		// Resource binding
		void SetInputLayout(const VertexInputLayout& layout);
		void SetVertexBuffer(uint32_t slot, const VertexBuffer& vb);
		void SetIndexBuffer(const IndexBuffer& ib);
		void SetVSResource(uint32_t slot, const GPUResource& resource);
		void SetPSResource(uint32_t slot, const GPUResource& resource);
		void SetPSSamplerState(uint32_t slot, const SamplerState& sampler);
		void SetVSConstantBuffer(uint32_t slot, const ConstantBuffer& buffer);
		void SetPSConstantBuffer(uint32_t slot, const ConstantBuffer& buffer);
		void SetCSConstantBuffer(uint32_t slot, const ConstantBuffer& buffer);

		// Batching
		void DrawIndexed(uint32_t indexStart, uint32_t indexCount);

		// Resource
		void UpdateSubresource(ConstantBuffer& resource, const void* data, size_t size);
	};
}