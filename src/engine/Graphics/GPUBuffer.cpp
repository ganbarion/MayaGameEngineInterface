//
// Copyright (c) GANBARION Co., Ltd. All rights reserved.
// This code is licensed under the MIT License (MIT).
//

#include "engine/Graphics/GPUBuffer.h"
#include "engine/Graphics/GraphicsCore.h"
#include "engine/Graphics/GraphicsContext.h"
#include "engine/Graphics/Shader.h"

namespace se
{
#pragma region ConstantBuffer

	ConstantBuffer::ConstantBuffer()
		: buffer_(nullptr)
	{
	}

	ConstantBuffer::~ConstantBuffer()
	{
		COMPTR_RELEASE(buffer_);
	}

	void ConstantBuffer::Create(uint32_t size, BufferUsage usage)
	{
		Assert(size % 16 == 0);
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.ByteWidth = size;
		switch (usage)
		{
		case BUFFER_USAGE_DYNAMIC:
			bd.Usage = D3D11_USAGE_DYNAMIC;
			bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			break;
		default:
			bd.Usage = D3D11_USAGE_DEFAULT;
			bd.CPUAccessFlags = 0;
			break;
		}

		THROW_IF_FAILED(GraphicsCore::GetDevice()->CreateBuffer(&bd, nullptr, &buffer_));
		size_ = size;
	}

	void ConstantBuffer::Update(GraphicsContext& context, const void* data, uint32_t size)
	{
		context.UpdateSubresource(*this, data, size);
	}

#pragma endregion


#pragma region GPUResource

	GPUResource::GPUResource()
		: resource_(nullptr)
		, srv_(nullptr)
		, uav_(nullptr)
	{
	}

	GPUResource::~GPUResource()
	{
		COMPTR_RELEASE(srv_);
		COMPTR_RELEASE(uav_);
		COMPTR_RELEASE(resource_);
	}

	void GPUResource::Destroy()
	{
		COMPTR_RELEASE(srv_);
		COMPTR_RELEASE(uav_);
		COMPTR_RELEASE(resource_);
	}

#pragma endregion

#pragma region VertexBuffer

	namespace
	{
		uint32_t GetVertexStride(uint32_t attr)
		{
			uint32_t size = 0;
			if (attr & VERTEX_ATTR_FLAG_POSITION) size += 12;
			if (attr & VERTEX_ATTR_FLAG_NORMAL) size += 12;
			if (attr & VERTEX_ATTR_FLAG_COLOR) size += 16;
			if (attr & VERTEX_ATTR_FLAG_TEXCOORD0) size += 8;
			if (attr & VERTEX_ATTR_FLAG_TEXCOORD1) size += 8;
			if (attr & VERTEX_ATTR_FLAG_TEXCOORD2) size += 8;
			if (attr & VERTEX_ATTR_FLAG_TEXCOORD3) size += 8;
			if (attr & VERTEX_ATTR_FLAG_TANGENT) size += 12;
			if (attr & VERTEX_ATTR_FLAG_BITANGENT) size += 12;
			return size;
		}
	}

	VertexBuffer::VertexBuffer()
		: stride_(0)
		, attributes_(0)
	{
	}

	VertexBuffer::~VertexBuffer()
	{
	}

	void VertexBuffer::Create(const void* data, uint32_t size, VertexAttributeFlags attributes, BufferUsage usage, bool unorderedAccess)
	{
		Assert(!resource_);

		// 頂点バッファの設定
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.CPUAccessFlags = 0;
		bd.ByteWidth = size;

		if (unorderedAccess) {
			bd.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_UNORDERED_ACCESS;
			bd.Usage = D3D11_USAGE_DEFAULT;
			bd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
		} else {
			bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			if (usage == BUFFER_USAGE_DYNAMIC) {
				bd.Usage = D3D11_USAGE_DYNAMIC;
				bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			} else if (usage == BUFFER_USAGE_DEFAULT) {
				bd.Usage = D3D11_USAGE_DEFAULT;
			} else {
				bd.Usage = D3D11_USAGE_IMMUTABLE;
			}
		}

		// サブリソースの設定
		D3D11_SUBRESOURCE_DATA* pInit = nullptr;
		D3D11_SUBRESOURCE_DATA initData;
		if (data) {
			ZeroMemory(&initData, sizeof(initData));
			initData.pSysMem = data;
			pInit = &initData;
		}

		// 頂点バッファ生成
		ID3D11Buffer* buffer;
		THROW_IF_FAILED(GraphicsCore::GetDevice()->CreateBuffer(&bd, pInit, &buffer));

		resource_ = buffer;
		stride_ = GetVertexStride(attributes);
		attributes_ = attributes;

		// アンオーダードアクセスビューを生成
		if (unorderedAccess) {
			D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
			uavDesc.Buffer.FirstElement = 0;
			uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
			uavDesc.Buffer.NumElements = size / 4;
			uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
			uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
			THROW_IF_FAILED(GraphicsCore::GetDevice()->CreateUnorderedAccessView(resource_, &uavDesc, &uav_));
		}
	}

	void VertexBuffer::Destroy()
	{
		GPUResource::Destroy();
	}

#pragma endregion

#pragma region IndexBuffer

	IndexBuffer::IndexBuffer()
		: stride_(INDEX_BUFFER_STRIDE_UNKNOWN)
		, bufferSize_(0)
		, indexCount_(0)
	{
	}

	IndexBuffer::~IndexBuffer()
	{
	}

	void IndexBuffer::Create(const void* data, uint32_t size, IndexBufferStride stride)
	{
		static uint32_t strides[] = { 2, 4 };
		D3D11_BUFFER_DESC ibd;
		ibd.Usage = D3D11_USAGE_IMMUTABLE;
		ibd.ByteWidth = size;
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibd.CPUAccessFlags = 0;
		ibd.MiscFlags = 0;
		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = data;

		ID3D11Buffer* buffer;
		HRESULT hr = GraphicsCore::GetDevice()->CreateBuffer(&ibd, &initData, &buffer);
		THROW_IF_FAILED(hr);
		resource_ = buffer;

		stride_ = stride;
		bufferSize_ = size;
		indexCount_ = size / strides[stride];
	}

	void IndexBuffer::Destroy()
	{
		GPUResource::Destroy();
	}

#pragma endregion

#pragma region PixelBuffer

	PixelBuffer::PixelBuffer()
		: width_(0)
		, height_(0)
		, depth_(0)
		, format_(DXGI_FORMAT_UNKNOWN)
	{
	}
	PixelBuffer::~PixelBuffer()
	{
	}

	void PixelBuffer::Destroy()
	{
		GPUResource::Destroy();
	}

#pragma endregion

#pragma region ColorBuffer

	ColorBuffer::ColorBuffer()
		: rtv_(nullptr)
	{
	}

	ColorBuffer::~ColorBuffer()
	{
		COMPTR_RELEASE(rtv_);
	}

	void ColorBuffer::InitializeDisplayBuffer(ID3D11RenderTargetView* renderTarget)
	{
		rtv_ = renderTarget;
	}

	void ColorBuffer::Create2D(DXGI_FORMAT format, uint32_t width, uint32_t height, uint32_t arraySize, uint32_t mips)
	{
		Assert(!resource_);
		mips = se::Max<uint32_t>(1, mips);
		width_ = width;
		height_ = height;
		depth_ = arraySize;
		format_ = format;

		// テクスチャ生成
		D3D11_TEXTURE2D_DESC objdesc;
		ZeroMemory(&objdesc, sizeof(objdesc));
		objdesc.Width = width;
		objdesc.Height = height;
		objdesc.MipLevels = mips;
		objdesc.ArraySize = arraySize;
		objdesc.SampleDesc.Count = 1;
		objdesc.SampleDesc.Quality = 0;
		objdesc.MiscFlags = 0;
		objdesc.Format = format;
		objdesc.Usage = D3D11_USAGE_DEFAULT;
		objdesc.CPUAccessFlags = 0;
		objdesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;

		//	テクスチャ生成
		auto* device = GraphicsCore::GetDevice();
		ID3D11Texture2D* texture;
		THROW_IF_FAILED(device->CreateTexture2D(&objdesc, nullptr, &texture));
		resource_ = texture;

		//	シェーダリソースビュー
		bool isArray = (arraySize > 1);
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		memset(&srvDesc, 0, sizeof(srvDesc));
		srvDesc.Format = format;
		if (isArray) {
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
			srvDesc.Texture2DArray.ArraySize = arraySize;
			srvDesc.Texture2DArray.MipLevels = mips;
		} else {
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = mips;
		}
		THROW_IF_FAILED(device->CreateShaderResourceView(texture, &srvDesc, &srv_));

		// レンダーターゲットビュー
		D3D11_RENDER_TARGET_VIEW_DESC rdesc;
		rdesc.Format = format;
		rdesc.ViewDimension = (arraySize > 1 || mips > 1) ? D3D11_RTV_DIMENSION_TEXTURE2DARRAY : D3D11_RTV_DIMENSION_TEXTURE2D;
		rdesc.Texture2DArray.ArraySize = 1;
		rdesc.Texture2DArray.MipSlice = 0;
		rdesc.Texture2DArray.FirstArraySlice = 0;
		THROW_IF_FAILED(device->CreateRenderTargetView(texture, &rdesc, &rtv_));
	}

	void ColorBuffer::CreateFromRTV(ID3D11RenderTargetView* rtv)
	{
		rtv->AddRef();
		rtv_ = rtv;
		rtv->GetResource(&resource_);

		auto* texture = static_cast<ID3D11Texture2D*>(resource_);
		D3D11_TEXTURE2D_DESC desc;
		texture->GetDesc(&desc);
		width_ = desc.Width;
		height_ = desc.Height;
		depth_ = desc.ArraySize;
		format_ = desc.Format;
	}

	void ColorBuffer::Destroy()
	{
		PixelBuffer::Destroy();
		COMPTR_RELEASE(rtv_);
	}

#pragma endregion

#pragma region DepthStencilBuffer

	DepthStencilBuffer::DepthStencilBuffer()
	{
	}

	DepthStencilBuffer::~DepthStencilBuffer()
	{
		COMPTR_RELEASE(dsv_);
	}

	void DepthStencilBuffer::Create(uint32_t width, uint32_t height)
	{
		auto* device = GraphicsCore::GetDevice();

		// デプスステンシルテクスチャ
		D3D11_TEXTURE2D_DESC descDepth;
		ZeroMemory(&descDepth, sizeof(descDepth));
		descDepth.Width = width;
		descDepth.Height = height;
		descDepth.MipLevels = 1;
		descDepth.ArraySize = 1;
		descDepth.Format = DXGI_FORMAT_R24G8_TYPELESS;
		descDepth.SampleDesc.Count = 1;
		descDepth.SampleDesc.Quality = 0;
		descDepth.Usage = D3D11_USAGE_DEFAULT;
		descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		descDepth.CPUAccessFlags = 0;
		descDepth.MiscFlags = 0;
		ID3D11Texture2D* texture2d;
		auto hr = device->CreateTexture2D(&descDepth, nullptr, &texture2d);
		THROW_IF_FAILED(hr);
		resource_ = texture2d;

		// デプスステンシルビュー
		D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
		ZeroMemory(&descDSV, sizeof(descDSV));
		descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		descDSV.Texture2D.MipSlice = 0;
		hr = device->CreateDepthStencilView(texture2d, &descDSV, &dsv_);
		THROW_IF_FAILED(hr);
	}

	void DepthStencilBuffer::CreateFromDSV(ID3D11DepthStencilView* dsv)
	{
		dsv->AddRef();
		dsv_ = dsv;
		dsv->GetResource(&resource_);

		auto* texture = static_cast<ID3D11Texture2D*>(resource_);
		D3D11_TEXTURE2D_DESC desc;
		texture->GetDesc(&desc);
		width_ = desc.Width;
		height_ = desc.Height;
		depth_ = desc.ArraySize;
		format_ = desc.Format;
	}

	void DepthStencilBuffer::Destroy()
	{
		PixelBuffer::Destroy();
		COMPTR_RELEASE(dsv_);
	}

#pragma endregion

#pragma region Texture

	Texture::Texture()
	{
	}

	Texture::~Texture()
	{
	}

	void Texture::Destroy()
	{
		PixelBuffer::Destroy();
	}

	void Texture::SetupInfo()
	{
		Assert(resource_);
		D3D11_RESOURCE_DIMENSION dimension;
		resource_->GetType(&dimension);
		switch (dimension)
		{
		case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
		{
			auto* texture = static_cast<ID3D11Texture2D*>(resource_);
			D3D11_TEXTURE2D_DESC desc;
			texture->GetDesc(&desc);
			width_ = desc.Width;
			height_ = desc.Height;
			depth_ = desc.ArraySize;
			format_ = desc.Format;
		}
		break;

		case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
		{
			auto* texture = static_cast<ID3D11Texture3D*>(resource_);
			D3D11_TEXTURE3D_DESC desc;
			texture->GetDesc(&desc);
			width_ = desc.Width;
			height_ = desc.Height;
			depth_ = desc.Depth;
			format_ = desc.Format;
		}
		break;

		default:
			throw "Invalid texture.";
		}
	}

	void Texture::CreateFromSRV(ID3D11ShaderResourceView* srv)
	{
		Assert(!resource_);
		srv->AddRef();
		srv_ = srv;
		srv_->GetResource(&resource_);
		SetupInfo();
	}

#pragma endregion
}