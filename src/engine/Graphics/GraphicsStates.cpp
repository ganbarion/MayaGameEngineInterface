//
// Copyright (c) GANBARION Co., Ltd. All rights reserved.
// This code is licensed under the MIT License (MIT).
//

#include "engine/Graphics/GraphicsStates.h"
#include "engine/Graphics/GraphicsCore.h"


namespace se
{
#pragma region SamplerState

	namespace {

		D3D11_COMPARISON_FUNC TranslateD3D11CompareFunction(CompareFunction CompareFunction)
		{
			switch (CompareFunction)
			{
			case CF_LESS: return D3D11_COMPARISON_LESS;
			case CF_LESSEQUAL: return D3D11_COMPARISON_LESS_EQUAL;
			case CF_GREATER: return D3D11_COMPARISON_GREATER;
			case CF_GREATEREQUAL: return D3D11_COMPARISON_GREATER_EQUAL;
			case CF_EQUAL: return D3D11_COMPARISON_EQUAL;
			case CF_NOTEQUAL: return D3D11_COMPARISON_NOT_EQUAL;
			case CF_NEVER: return D3D11_COMPARISON_NEVER;
			default: return D3D11_COMPARISON_ALWAYS;
			};
		}

		D3D11_TEXTURE_ADDRESS_MODE TranslateD3D11AddressMode(TextureAddressMode mode)
		{
			switch (mode)
			{
			case TAM_WRAP:	 return D3D11_TEXTURE_ADDRESS_WRAP;
			case TAM_CLAMP:	 return D3D11_TEXTURE_ADDRESS_CLAMP;
			case TAM_MIRROR: return D3D11_TEXTURE_ADDRESS_MIRROR;
			case TAM_BORDER: return D3D11_TEXTURE_ADDRESS_BORDER;
			default:         return D3D11_TEXTURE_ADDRESS_WRAP;
			};
		}

	}

	SamplerState::SamplerState()
		: state_(nullptr)
	{
	}

	SamplerState::~SamplerState()
	{
		COMPTR_RELEASE(state_)
	}

	void SamplerState::Create(SamplerFilter filter, TextureAddressMode AddressU, TextureAddressMode AddressV, TextureAddressMode AddressW, CompareFunction ComparisonFunc, int32_t MipLODBias, int32_t Anisotropy, uint32_t BorderColor)
	{
		D3D11_SAMPLER_DESC sampDesc;
		ZeroMemory(&sampDesc, sizeof(sampDesc));
		sampDesc.AddressU = TranslateD3D11AddressMode(AddressU);
		sampDesc.AddressV = TranslateD3D11AddressMode(AddressV);
		sampDesc.AddressW = TranslateD3D11AddressMode(AddressW);
		sampDesc.ComparisonFunc = TranslateD3D11CompareFunction(ComparisonFunc);
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
		sampDesc.MipLODBias = static_cast<FLOAT>(MipLODBias) / 100;
		sampDesc.MaxAnisotropy = (Anisotropy > 0) ? Clamp(Anisotropy, 0, 16) : 4;	// Defalut Value 4.
		for (int32_t i = 0; i < 4; i++) {
			sampDesc.BorderColor[i] = ((BorderColor >> (8 * i)) & 0xff) / 255.0f;
		}

		bool disableCompare = (ComparisonFunc == CF_NEVER);
		switch (filter)
		{
		case FILTER_POINT:
			sampDesc.Filter = disableCompare ? D3D11_FILTER_MIN_MAG_MIP_POINT : D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
			break;
		case FILTER_BILINEAR:
			sampDesc.Filter = disableCompare ? D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT : D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
			break;
		case FILTER_TRILINEAR:
			sampDesc.Filter = disableCompare ? D3D11_FILTER_MIN_MAG_MIP_LINEAR : D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
			break;
		case FILTER_ANISOTROPIC_POINT:
		case FILTER_ANISOTROPIC_LINEAR:
			sampDesc.Filter = disableCompare ? D3D11_FILTER_ANISOTROPIC : D3D11_FILTER_COMPARISON_ANISOTROPIC;
			break;
		}

		THROW_IF_FAILED(GraphicsCore::GetDevice()->CreateSamplerState(&sampDesc, &state_));
	}

	void SamplerState::Destroy()
	{
		COMPTR_RELEASE(state_)
	}

#pragma endregion

#pragma region

	DepthStencilState DepthStencilState::templates_[DepthTypeNum];

	void DepthStencilState::Initialize()
	{
		auto* device = GraphicsCore::GetDevice();

		D3D11_DEPTH_STENCIL_DESC dsDesc;
		dsDesc.StencilEnable = false;
		dsDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
		dsDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
		dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
		dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		dsDesc.BackFace = dsDesc.FrontFace;

		{
			dsDesc.DepthEnable = false;
			dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
			dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
			HRESULT hr = device->CreateDepthStencilState(&dsDesc, &templates_[Disable].state_);
			THROW_IF_FAILED(hr);
		}
		{
			dsDesc.DepthEnable = true;
			dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
			dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
			HRESULT hr = device->CreateDepthStencilState(&dsDesc, &templates_[Enable].state_);
			THROW_IF_FAILED(hr);
		}
		{
			dsDesc.DepthEnable = false;
			dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
			HRESULT hr = device->CreateDepthStencilState(&dsDesc, &templates_[WriteDisable].state_);
			THROW_IF_FAILED(hr);
		}
		{
			dsDesc.DepthEnable = true;
			dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
			HRESULT hr = device->CreateDepthStencilState(&dsDesc, &templates_[WriteEnable].state_);
			THROW_IF_FAILED(hr);
		}
		{
			dsDesc.DepthEnable = true;
			dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			dsDesc.DepthFunc = D3D11_COMPARISON_EQUAL;
			HRESULT hr = device->CreateDepthStencilState(&dsDesc, &templates_[WriteEnableEqual].state_);
			THROW_IF_FAILED(hr);
		}
		{
			dsDesc.DepthEnable = true;
			dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			dsDesc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
			HRESULT hr = device->CreateDepthStencilState(&dsDesc, &templates_[WriteEnableReverse].state_);
			THROW_IF_FAILED(hr);
		}
	}

	DepthStencilState::DepthStencilState()
		: state_(nullptr)
	{
	}

	DepthStencilState::~DepthStencilState()
	{
		COMPTR_RELEASE(state_)
	}

#pragma endregion

#pragma region RasterizerState

	RasterizerState RasterizerState::templates_[RasterizerTypeNum];

	void RasterizerState::Initialize()
	{
		auto* device = GraphicsCore::GetDevice();

		D3D11_RASTERIZER_DESC rastDesc;
		rastDesc.AntialiasedLineEnable = false;
		rastDesc.DepthBias = 0;
		rastDesc.DepthBiasClamp = 0.0f;
		rastDesc.DepthClipEnable = true;
		rastDesc.SlopeScaledDepthBias = 0;
		rastDesc.MultisampleEnable = false;
		rastDesc.FrontCounterClockwise = true;	// 右手座標系
		rastDesc.ScissorEnable = true;

		{
			rastDesc.CullMode = D3D11_CULL_NONE;
			rastDesc.FillMode = D3D11_FILL_SOLID;
			HRESULT hr = device->CreateRasterizerState(&rastDesc, &templates_[NoCull].state_);
			THROW_IF_FAILED(hr);
		}
		{
			rastDesc.CullMode = D3D11_CULL_BACK;
			rastDesc.FillMode = D3D11_FILL_SOLID;
			HRESULT hr = device->CreateRasterizerState(&rastDesc, &templates_[BackFaceCull].state_);
			THROW_IF_FAILED(hr);
		}
		{
			rastDesc.CullMode = D3D11_CULL_FRONT;
			rastDesc.FillMode = D3D11_FILL_SOLID;
			HRESULT hr = device->CreateRasterizerState(&rastDesc, &templates_[FrontFaceCull].state_);
			THROW_IF_FAILED(hr);
		}
		{
			rastDesc.CullMode = D3D11_CULL_NONE;
			rastDesc.FillMode = D3D11_FILL_WIREFRAME;
			HRESULT hr = device->CreateRasterizerState(&rastDesc, &templates_[WireFrame].state_);
			THROW_IF_FAILED(hr);
		}
	}

	RasterizerState::RasterizerState()
		: state_(nullptr)
	{
	}

	RasterizerState::~RasterizerState()
	{
		COMPTR_RELEASE(state_)
	}

#pragma endregion

#pragma region BlendState

	namespace {
		void SetRenderTargetBlendDesc(D3D11_RENDER_TARGET_BLEND_DESC* desc, BlendState::BlendType type, uint8_t color_mask = D3D11_COLOR_WRITE_ENABLE_ALL)
		{
			D3D11_RENDER_TARGET_BLEND_DESC& ref = *desc;

			switch (type)
			{
			case BlendState::Opacity:
				ref.BlendEnable = FALSE;
				ref.SrcBlend = D3D11_BLEND_ONE;
				ref.DestBlend = D3D11_BLEND_ZERO;
				ref.BlendOp = D3D11_BLEND_OP_ADD;
				ref.SrcBlendAlpha = D3D11_BLEND_ONE;
				ref.DestBlendAlpha = D3D11_BLEND_ZERO;
				ref.BlendOpAlpha = D3D11_BLEND_OP_ADD;
				ref.RenderTargetWriteMask = color_mask;
				break;

			case BlendState::Translucent:
				ref.BlendEnable = TRUE;
				ref.SrcBlend = D3D11_BLEND_SRC_ALPHA;
				ref.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
				ref.BlendOp = D3D11_BLEND_OP_ADD;
				ref.SrcBlendAlpha = D3D11_BLEND_ONE;
				ref.DestBlendAlpha = D3D11_BLEND_ZERO;
				ref.BlendOpAlpha = D3D11_BLEND_OP_ADD;
				ref.RenderTargetWriteMask = color_mask;
				break;

			case BlendState::Additive:
				ref.BlendEnable = TRUE;
				ref.SrcBlend = D3D11_BLEND_SRC_ALPHA;
				ref.DestBlend = D3D11_BLEND_ONE;
				ref.BlendOp = D3D11_BLEND_OP_ADD;
				ref.SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
				ref.DestBlendAlpha = D3D11_BLEND_ONE;
				ref.BlendOpAlpha = D3D11_BLEND_OP_ADD;
				ref.RenderTargetWriteMask = color_mask;
				break;

			case BlendState::Modulate:
				ref.BlendEnable = TRUE;
				ref.SrcBlend = D3D11_BLEND_ZERO;
				ref.DestBlend = D3D11_BLEND_SRC_COLOR;
				ref.BlendOp = D3D11_BLEND_OP_ADD;
				ref.SrcBlendAlpha = D3D11_BLEND_ONE;
				ref.DestBlendAlpha = D3D11_BLEND_ZERO;
				ref.BlendOpAlpha = D3D11_BLEND_OP_ADD;
				ref.RenderTargetWriteMask = color_mask;
				break;

			case BlendState::Subtruct:
				ref.BlendEnable = TRUE;
				ref.SrcBlend = D3D11_BLEND_SRC_ALPHA;
				ref.DestBlend = D3D11_BLEND_ONE;
				ref.BlendOp = D3D11_BLEND_OP_REV_SUBTRACT;
				ref.SrcBlendAlpha = D3D11_BLEND_ONE;
				ref.DestBlendAlpha = D3D11_BLEND_ZERO;
				ref.BlendOpAlpha = D3D11_BLEND_OP_ADD;
				ref.RenderTargetWriteMask = color_mask;
				break;
			}
		}
	}

	BlendState BlendState::templates_[BlendTypeNum];

	void BlendState::Initialize()
	{
		for (int32_t i = 0; i < BlendTypeNum; i++) {
			templates_[i].Create(static_cast<BlendType>(i));
		}
	}

	void BlendState::Finalize()
	{
		for (int32_t i = 0; i < BlendTypeNum; i++) {
			templates_[i].Destroy();
		}
	}

	BlendState::BlendState()
		: state_(nullptr)
	{
	}

	BlendState::~BlendState()
	{
		COMPTR_RELEASE(state_);
	}

	void BlendState::Create(BlendType type)
	{
		D3D11_BLEND_DESC blendState;
		memset(&blendState, 0, sizeof(D3D11_BLEND_DESC));
		blendState.AlphaToCoverageEnable = FALSE;
		blendState.IndependentBlendEnable = TRUE;
		SetRenderTargetBlendDesc(&blendState.RenderTarget[0], Opacity, D3D11_COLOR_WRITE_ENABLE_ALL);
		THROW_IF_FAILED(GraphicsCore::GetDevice()->CreateBlendState(&blendState, &state_));
	}

	void BlendState::Destroy()
	{
		COMPTR_RELEASE(state_);
	}

#pragma endregion

}