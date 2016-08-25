//
// Copyright (c) GANBARION Co., Ltd. All rights reserved.
// This code is licensed under the MIT License (MIT).
//

#include "engine/Graphics/GraphicsContext.h"
#include "engine/Graphics/Shader.h"
#include "engine/Graphics/GPUBuffer.h"
#include "engine/Graphics/GraphicsStates.h"

namespace se
{
	GraphicsContext::GraphicsContext()
		: deviceContext_(nullptr)
	{
	}

	GraphicsContext::~GraphicsContext()
	{
		COMPTR_RELEASE(deviceContext_);
	}

	void GraphicsContext::Initialize(ID3D11DeviceContext* context)
	{
		deviceContext_ = context;
	}

	void GraphicsContext::Finalize()
	{
		if (deviceContext_) {
			deviceContext_->ClearState();
		}
		COMPTR_RELEASE(deviceContext_);
	}

	void GraphicsContext::SetRenderTarget(const ColorBuffer* colorBuffers, uint32_t count, const DepthStencilBuffer* depthStencil)
	{
		ID3D11RenderTargetView* rtvs[8] = { nullptr };
		if (colorBuffers) {
			for (uint32_t i = 0; i < count; i++) {
				rtvs[i] = colorBuffers[i].GetRTV();
			}
		}
		auto* depthStencilView = depthStencil ? depthStencil->GetDSV() : nullptr;
		deviceContext_->OMSetRenderTargets(count, rtvs, depthStencilView);
	}

	void GraphicsContext::ClearRenderTarget(const ColorBuffer& target, const float4& color)
	{
		deviceContext_->ClearRenderTargetView(target.GetRTV(), color.ToFloatArray());
	}

	void GraphicsContext::ClearDepthStencil(const DepthStencilBuffer& target, float depth)
	{
		deviceContext_->ClearDepthStencilView(target.GetDSV(), D3D11_CLEAR_DEPTH, depth, 0);
	}

	void GraphicsContext::SetVertexShader(const VertexShader& shader)
	{
		deviceContext_->VSSetShader(shader.Get(), nullptr, 0);
	}

	void GraphicsContext::SetPixelShader(const PixelShader& shader)
	{
		deviceContext_->PSSetShader(shader.Get(), nullptr, 0);
	}

	void GraphicsContext::SetBlendState(const BlendState& blend)
	{
		deviceContext_->OMSetBlendState(blend.state_, 0, 0xFFFFFFFF);
	}

	void GraphicsContext::SetDepthStencilState(const DepthStencilState& depthStencil, uint32_t stencilRef)
	{
		deviceContext_->OMSetDepthStencilState(depthStencil.state_, stencilRef);
	}

	void GraphicsContext::SetRasterizerState(const RasterizerState& raster)
	{
		deviceContext_->RSSetState(raster.state_);
	}

	void GraphicsContext::SetPrimitiveType(PrimitiveType type)
	{
		static const D3D_PRIMITIVE_TOPOLOGY types[] = {
			D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
			D3D11_PRIMITIVE_TOPOLOGY_LINELIST,
		};
		deviceContext_->IASetPrimitiveTopology(types[type]);
	}

	void GraphicsContext::SetViewport(const Rect& rect, float minDepth, float maxDepth)
	{
		D3D11_VIEWPORT vp;
		vp.TopLeftX = static_cast<float>(rect.x);
		vp.TopLeftY = static_cast<float>(rect.y);
		vp.Width = static_cast<float>(rect.width);
		vp.Height = static_cast<float>(rect.height);
		vp.MinDepth = minDepth;
		vp.MaxDepth = maxDepth;
		deviceContext_->RSSetViewports(1, &vp);
	}

	void GraphicsContext::SetScissorRect(const Rect& rect)
	{
		D3D11_RECT r = { rect.x, rect.y, rect.width, rect.height };
		deviceContext_->RSSetScissorRects(1, &r);
	}

	void GraphicsContext::SetInputLayout(const VertexInputLayout& layout)
	{
		deviceContext_->IASetInputLayout(layout.layout);
	}

	void GraphicsContext::SetVertexBuffer(uint32_t slot, const VertexBuffer& vb)
	{
		ID3D11Buffer* buffers[] = { vb.Get<ID3D11Buffer>() };
		uint32_t stride = vb.GetStride();
		uint32_t offset = 0;
		deviceContext_->IASetVertexBuffers(slot, 1, buffers, &stride, &offset);
	}

	void GraphicsContext::SetIndexBuffer(const IndexBuffer& ib)
	{
		static const DXGI_FORMAT formats[] = {
			DXGI_FORMAT_R16_UINT,
			DXGI_FORMAT_R32_UINT,
		};
		deviceContext_->IASetIndexBuffer(ib.Get<ID3D11Buffer>(), formats[ib.stride_], 0);
	}

	void GraphicsContext::SetVSResource(uint32_t slot, const GPUResource& resource)
	{
		ID3D11ShaderResourceView* resources[] = { resource.GetSRV() };
		deviceContext_->VSSetShaderResources(slot, 1, resources);
	}

	void GraphicsContext::SetPSResource(uint32_t slot, const GPUResource& resource)
	{
		ID3D11ShaderResourceView* resources[] = { resource.GetSRV() };
		deviceContext_->PSSetShaderResources(slot, 1, resources);
	}

	void GraphicsContext::SetPSSamplerState(uint32_t slot, const SamplerState& sampler)
	{
		deviceContext_->PSSetSamplers(slot, 1, &sampler.state_);
	}

	void GraphicsContext::SetVSConstantBuffer(uint32_t slot, const ConstantBuffer& buffer)
	{
		deviceContext_->VSSetConstantBuffers(slot, 1, &buffer.buffer_);
	}

	void GraphicsContext::SetPSConstantBuffer(uint32_t slot, const ConstantBuffer& buffer)
	{
		deviceContext_->PSSetConstantBuffers(slot, 1, &buffer.buffer_);
	}

	void GraphicsContext::SetCSConstantBuffer(uint32_t slot, const ConstantBuffer& buffer)
	{
		deviceContext_->CSSetConstantBuffers(slot, 1, &buffer.buffer_);
	}

	void GraphicsContext::DrawIndexed(uint32_t indexStart, uint32_t indexCount)
	{
		deviceContext_->DrawIndexed(indexCount, indexStart, 0);
	}

	void GraphicsContext::UpdateSubresource(ConstantBuffer& resource, const void* data, size_t size)
	{
		deviceContext_->UpdateSubresource(resource.buffer_, 0, nullptr, data, 0, 0);
	}
}