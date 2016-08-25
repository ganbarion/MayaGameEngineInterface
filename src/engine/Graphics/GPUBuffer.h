//
// Copyright (c) GANBARION Co., Ltd. All rights reserved.
// This code is licensed under the MIT License (MIT).
//

#pragma once 

#include "engine/Graphics/GraphicsCommon.h"
#include "engine/Graphics/GraphicsContext.h"

namespace se
{
	enum BufferUsage
	{
		BUFFER_USAGE_DEFAULT,
		BUFFER_USAGE_IMMUTABLE,		// 不変バッファ
		BUFFER_USAGE_GPU_WRITE,		// GPUから書き込み可能
		BUFFER_USAGE_DYNAMIC,		// CPU, GPUから書き込み可能
	};

	enum IndexBufferStride
	{
		INDEX_BUFFER_STRIDE_U16,
		INDEX_BUFFER_STRIDE_U32,

		INDEX_BUFFER_STRIDE_UNKNOWN,
	};


	/**
	 * コンスタントバッファ
	 */
	class ConstantBuffer
	{
		friend class GraphicsContext;

	private:
		ID3D11Buffer* buffer_;
		uint32_t size_;

	public:
		ConstantBuffer();
		virtual ~ConstantBuffer();

		void Create(uint32_t size, BufferUsage usage);
		void Update(GraphicsContext& context, const void* data, uint32_t size);
	};


	/**
	 * GPUで使用されるリソース
	 */
	class GPUResource
	{
	protected:
		ID3D11Resource*				resource_;
		ID3D11ShaderResourceView*	srv_;
		ID3D11UnorderedAccessView*	uav_;

	public:
		GPUResource();
		virtual ~GPUResource();

		virtual void Destroy();

		ID3D11Resource* GetResource() const { return resource_; }
		ID3D11ShaderResourceView* GetSRV() const { return srv_; }
		ID3D11UnorderedAccessView* GetUAV() const { return uav_; }

		template <class T>
		T* Get() const { return static_cast<T*>(resource_); }
	};

	
	/**
	 * 頂点バッファ
	 */
	class VertexBuffer : public GPUResource
	{
	private:
		uint32_t stride_;
		uint32_t attributes_;

	public:
		VertexBuffer();
		virtual ~VertexBuffer();

		void Create(const void* data, uint32_t size, VertexAttributeFlags attributes, BufferUsage usage = BUFFER_USAGE_IMMUTABLE, bool unorderedAccess = false);
		virtual void Destroy() override;

		uint32_t GetStride() const { return stride_; }
		uint32_t GetAttributes() const { return attributes_; }
	};

	
	/**
	 * インデックスバッファ
	 */
	class IndexBuffer : public GPUResource
	{
		friend class GraphicsContext;

	private:
		IndexBufferStride stride_;
		uint32_t bufferSize_;
		uint32_t indexCount_;

	public:
		IndexBuffer();
		virtual ~IndexBuffer();

		void Create(const void* data, uint32_t size, IndexBufferStride stride);
		virtual void Destroy() override;

		uint32_t GetIndexCount() const { return indexCount_; }
	};


	/**
	 * ピクセルバッファ
	 */
	class PixelBuffer : public GPUResource
	{
	protected:
		uint32_t width_;
		uint32_t height_;
		uint32_t depth_;
		DXGI_FORMAT format_;

	public:
		PixelBuffer();
		virtual ~PixelBuffer();

		virtual void Destroy() override;

		uint32_t GetWidth() const { return width_; }
		uint32_t GetHeight() const { return height_; }
		uint32_t GetDepth() const { return depth_; }
	};
	

	/**
	 * カラーバッファ
	 */
	class ColorBuffer : public PixelBuffer
	{
		friend class GraphicsCore;

	private:
		ID3D11RenderTargetView* rtv_;

	private:
		void InitializeDisplayBuffer(ID3D11RenderTargetView* renderTarget);

	public:
		ColorBuffer();
		virtual ~ColorBuffer();

		void Create2D(DXGI_FORMAT format, uint32_t width, uint32_t height, uint32_t arraySize = 1u, uint32_t mips = 1u);
		void CreateFromRTV(ID3D11RenderTargetView* rtv);
		virtual void Destroy() override;

		ID3D11RenderTargetView* GetRTV() const { return rtv_; }
	};


	/**
	 * デプスステンシルバッファ
	 */
	class DepthStencilBuffer : public PixelBuffer
	{
	private:
		ID3D11DepthStencilView* dsv_;

	public:
		DepthStencilBuffer();
		virtual ~DepthStencilBuffer();

		void Create(uint32_t width, uint32_t height);
		void CreateFromDSV(ID3D11DepthStencilView* dsv);
		virtual void Destroy() override;

		ID3D11DepthStencilView* GetDSV() const { return dsv_; }
	};


	/**
	 * テクスチャリソース
	 */
	class Texture : public PixelBuffer
	{
	private:
		void SetupInfo();

	public:
		Texture();
		virtual ~Texture();
		virtual void Destroy() override;

		void CreateFromSRV(ID3D11ShaderResourceView* srv);
	};


	/**
	 * ユニフォームパラメータ
	 */
	template <class T>
	class TUniformParameter
	{
	private:
		ConstantBuffer resource_;
		T contents_;
		bool isCreated_;
		bool isUpdated_;

	public:
		TUniformParameter()
			: isCreated_(false)
			, isUpdated_(true)
		{
		}

		void Destroy()
		{
			resource_.Destroy();
			isCreated_ = false;
		}

		void Set(const T& buffer)
		{
			contents_ = buffer;
			isUpdated_ = true;
		}

		void Update(GraphicsContext& context, bool forceUpdate = false)
		{
			if (!isCreated_) {
				resource_.Create(sizeof(T), BUFFER_USAGE_DEFAULT);
				isCreated_ = true;
			}
			if (isUpdated_ || forceUpdate) {
				resource_.Update(context, &contents_, sizeof(T));
				isUpdated_ = false;
			}
		}

		T& Contents() { return contents_; }
		const T& Contents() const { return contents_; }
		void Updated() { isUpdated_ = true; }
		const ConstantBuffer& GetResource() const { return resource_; }
		bool IsCreated() const { return isCreated_; }
	};
}