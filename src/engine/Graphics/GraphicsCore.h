//
// Copyright (c) GANBARION Co., Ltd. All rights reserved.
// This code is licensed under the MIT License (MIT).
//

#pragma once 

#include "engine/Graphics/GraphicsCommon.h"
#include "engine/Graphics/GraphicsContext.h"
#include "engine/Graphics/GPUBuffer.h"

namespace se
{
	class GraphicsCore
	{
	private:
		static D3D_DRIVER_TYPE			driverType_;
		static D3D_FEATURE_LEVEL		featureLevel_;
		static ID3D11Device*			device_;
		static IDXGISwapChain*			swapChain_;

		static GraphicsContext			immediateContext_;
		static ColorBuffer				displayBuffer_;
		static DepthStencilBuffer		displayDepthBuffer_;

	public:
		static void Initialize(HWND hwnd);
		static void InitializeByExternalDevice(ID3D11Device* device);
		static void Finalize();

		static ID3D11Device* GetDevice() { return device_; }
		static GraphicsContext& GetImmediateContext() { return immediateContext_; }

		static void Present(uint32_t syncInterval, uint32_t flags);

		static const ColorBuffer& GetDisplayColorBuffer() { return displayBuffer_; }
		static const DepthStencilBuffer& GetDisplayDepthStencilBuffer() { return displayDepthBuffer_; }
	};
}