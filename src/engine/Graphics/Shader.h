//
// Copyright (c) GANBARION Co., Ltd. All rights reserved.
// This code is licensed under the MIT License (MIT).
//

#pragma once 

#include "engine/Graphics/GraphicsCommon.h"
#include "engine/Graphics/GraphicsContext.h"
#include <unordered_map>

namespace se
{
	class ShaderManager;

	/**
	 * シェーダリフレクション
	 */
	class ShaderReflection
	{
	private:
		ID3D11ShaderReflection* reflection_;

	public:
		ShaderReflection();
		~ShaderReflection();

		void Create(const void* data, size_t size);
		uint32_t GetVertexLayoutAttribute();
	};

	/**
	 * 頂点レイアウト
	 */
	struct VertexInputLayout
	{
		ID3D11InputLayout* layout;
		uint32_t vertexAttr;
		uint32_t shaderAttr;

		VertexInputLayout()
			: layout(nullptr)
			, vertexAttr(0)
			, shaderAttr(0)
		{
		}

		~VertexInputLayout()
		{
			COMPTR_RELEASE(layout);
		}
	};

	/**
	 * 頂点レイアウト管理
	 */
	class VertexLayoutManager
	{
	public:
		static VertexLayoutManager& Get() {
			static VertexLayoutManager instance;
			return instance;
		}
	private:
		VertexLayoutManager() {}
		~VertexLayoutManager() {}

	private:
		struct AttributeSet
		{
			uint32_t vertexAttr;
			uint32_t shaderAttr;
		};

	private:
		std::unordered_map<size_t, VertexInputLayout> layoutMap_;

	public:
		void Initialize();
		void Finalize();

		const VertexInputLayout* GetLayout(const VertexShader& shader, uint32_t vertexAttr);
	};

	/**
	 * 頂点シェーダ
	 */
	class VertexShader
	{
		friend GraphicsContext;

	private:
		ID3D11VertexShader* shader_;
		uint32_t vertexAttribute_;
		ID3DBlob* blob_;
		const void* data_;
		size_t dataSize_;

	public:
		VertexShader();
		~VertexShader();

		ID3D11VertexShader* Get() const { return shader_; }
		const void* GetByteCode() const { return data_; }
		size_t GetByteCodeSize() const { return dataSize_; }
		uint32_t GetVertexAttribute() const { return vertexAttribute_; }
		void CreateFromByteCode(const void* data, int size, ShaderReflection* reflection = nullptr);
		void CompileFromFile(const char* fileName, const char* entryPoint = "main", ShaderReflection* reflection = nullptr);
		void CompileFromString(const char* source, int length, const char* entryPoint = "main", ShaderReflection* reflection = nullptr);
		void Destroy();
	};

	/**
	 * ピクセルシェーダ
	 */
	class PixelShader
	{
		friend GraphicsContext;

	private:
		ID3D11PixelShader* shader_;

	public:
		PixelShader();
		~PixelShader();

		ID3D11PixelShader* Get() const { return shader_; }
		void CreateFromByteCode(const void* data, int size, ShaderReflection* reflection = nullptr);
		void CompileFromFile(const char* fileName, const char* entryPoint = "main", ShaderReflection* reflection = nullptr);
		void CompileFromString(const char* source, int length, const char* entryPoint = "main", ShaderReflection* reflection = nullptr);
		void Destroy();
	};


	/**
	 * シェーダセット
	 */
	class ShaderSet
	{
		friend class ShaderManager;
	private:
		VertexShader vs_;
		PixelShader ps_;

	public:
		ShaderSet() {};
		~ShaderSet() {};

		const VertexShader& GetVS() const { return vs_; }
		const PixelShader& GetPS() const { return ps_; }
	};


	/**
	 * シェーダ管理
	 */
	class ShaderManager
	{
	public:
		static ShaderManager& Get() { 
			static ShaderManager instance;
			return instance;
		}

	private:
		std::string directoryPath_;
		std::unordered_map<size_t, ShaderSet> shaderMap_;

	public:
		void Initialize(const char* directoryPath);
		void Finalize();
		void Reload();

		ShaderSet* Find(const char* name)
		{
			static std::hash<std::string> hasher;
			return Find(hasher(name));
		}
		ShaderSet* Find(size_t hash) 
		{
			auto& iter = shaderMap_.find(hash);
			return (iter != shaderMap_.end()) ? &iter->second : nullptr;
		}
	};
}