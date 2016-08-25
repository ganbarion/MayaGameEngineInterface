//
// Copyright (c) GANBARION Co., Ltd. All rights reserved.
// This code is licensed under the MIT License (MIT).
//

#include "engine/Graphics/Shader.h"
#include "engine/Graphics/GraphicsCore.h"
#include "ext/picojson/picojson.h"

namespace se
{
	namespace
	{
		void CompileShaderFromFile(const wchar_t* szFileName, const char* szEntryPoint, const char* szShaderModel, ID3DBlob** ppBlobOut)
		{
			HRESULT hr = S_OK;
			DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
			dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

			// シェーダをファイルからコンパイル
			ID3DBlob* pErrorBlob = nullptr;
			hr = D3DCompileFromFile(
				szFileName,
				NULL,
				D3D_COMPILE_STANDARD_FILE_INCLUDE,
				szEntryPoint,
				szShaderModel,
				dwShaderFlags,
				0,
				ppBlobOut,
				&pErrorBlob
			);

			// エラーメッセージ出力
			if (FAILED(hr)) {
				if (pErrorBlob != nullptr) {
					Printf((char*)pErrorBlob->GetBufferPointer());
				}
				throw "ShaderCompileError.";
			}

			COMPTR_RELEASE(pErrorBlob);
		}

		void CompileShaderFromFile(const char* szFileName, const char* szEntryPoint, const char* szShaderModel, ID3DBlob** ppBlobOut)
		{
			size_t size = 0;
			wchar_t buffer[MAX_PATH] = { 0 };
			size_t chr_len = strlen(szFileName) + 1;
			mbstowcs_s(&size, buffer, chr_len, szFileName, _TRUNCATE);
			CompileShaderFromFile(buffer, szEntryPoint, szShaderModel, ppBlobOut);
		}

		void CompileShaderFromString(const char* str, int length, const char* szEntryPoint, const char* szShaderModel, ID3DBlob** ppBlobOut)
		{
			DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
			dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif
			ID3DBlob* pErrorBlob = nullptr;

			// シェーダをファイルからコンパイル
			HRESULT hr = D3DCompile(
				str,
				length,
				nullptr,
				nullptr,
				nullptr,
				szEntryPoint,
				szShaderModel,
				dwShaderFlags,
				0,
				ppBlobOut,
				&pErrorBlob
				);

			// エラーメッセージ出力
			if (FAILED(hr)) {
				if (pErrorBlob != nullptr) {
					Printf((char*)pErrorBlob->GetBufferPointer());
					throw;
				}
			}

			COMPTR_RELEASE(pErrorBlob);
		}
	}


	/* ********************************************************************************************* */

	ShaderReflection::ShaderReflection()
		: reflection_(nullptr)
	{
	}


	ShaderReflection::~ShaderReflection()
	{
		COMPTR_RELEASE(reflection_);
	}


	void ShaderReflection::Create(const void* data, size_t size)
	{
		HRESULT hr = D3DReflect(data, size, IID_ID3D11ShaderReflection, (void**)&reflection_);
		THROW_IF_FAILED(hr);
	}


	/**
	 * 頂点アトリビュートを取得する
	 */
	uint32_t ShaderReflection::GetVertexLayoutAttribute()
	{
		static const std::array<char*, 5> inputSemantics = {
			"POSITION",
			"NORMAL",
			"TEXCOORD",
			"TANGENT",
			"BITANGENT",
		};
		static const std::array<uint32_t, 5> inputAttribute = {
			VERTEX_ATTR_FLAG_POSITION,
			VERTEX_ATTR_FLAG_NORMAL,
			VERTEX_ATTR_FLAG_TEXCOORD0,
			VERTEX_ATTR_FLAG_TANGENT,
			VERTEX_ATTR_FLAG_BITANGENT,
		};

		uint32_t attr = 0;
		D3D11_SHADER_DESC shader_desc;
		reflection_->GetDesc(&shader_desc);
		for (uint32_t i = 0; i < shader_desc.InputParameters; i++) {
			D3D11_SIGNATURE_PARAMETER_DESC param_desc;
			reflection_->GetInputParameterDesc(i, &param_desc);
			for (uint32_t j = 0; j < inputSemantics.size(); j++) {
				if (strcmp(inputSemantics[j], param_desc.SemanticName) == 0) {
					attr |= inputAttribute[j];
					break;
				}
			}
		}
		return attr;
	}

	/* ********************************************************************************************* */

	void VertexLayoutManager::Initialize()
	{
	}

	void VertexLayoutManager::Finalize()
	{
		layoutMap_.clear();
	}

	const VertexInputLayout* VertexLayoutManager::GetLayout(const VertexShader& shader, uint32_t vertexAttr)
	{
		// 既存データから検索
		Assert(sizeof(size_t) == 8);	// 64bit only
		size_t hash = (vertexAttr | (static_cast<size_t>(shader.GetVertexAttribute()) << 32));
		auto iter = layoutMap_.find(hash);
		if (iter != layoutMap_.end()) {
			Assert((iter->second.shaderAttr = shader.GetVertexAttribute()) && iter->second.vertexAttr == vertexAttr);
			return &iter->second;
		}

		auto pair = layoutMap_.emplace(hash, VertexInputLayout());
		Assert(pair.second);
		VertexInputLayout& layout = pair.first->second;

		// 見つからなかったら生成
		D3D11_INPUT_ELEMENT_DESC desc[16];
		uint32_t count = 0;
		if (vertexAttr & VERTEX_ATTR_FLAG_POSITION) {
			D3D11_INPUT_ELEMENT_DESC t = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, count, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 };
			desc[count] = t;
			count++;
		}
		if (vertexAttr & VERTEX_ATTR_FLAG_NORMAL) {
			D3D11_INPUT_ELEMENT_DESC t = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, count, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 };
			desc[count] = t;
			count++;
		}
		if (vertexAttr & VERTEX_ATTR_FLAG_COLOR) {
			D3D11_INPUT_ELEMENT_DESC t = { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, count, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 };
			desc[count] = t;
			count++;
		}
		if (vertexAttr & VERTEX_ATTR_FLAG_TEXCOORD0) {
			D3D11_INPUT_ELEMENT_DESC t = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, count, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 };
			desc[count] = t;
			count++;
		}
		if (vertexAttr & VERTEX_ATTR_FLAG_TEXCOORD1) {
			D3D11_INPUT_ELEMENT_DESC t = { "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, count, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 };
			desc[count] = t;
			count++;
		}
		if (vertexAttr & VERTEX_ATTR_FLAG_TEXCOORD2) {
			D3D11_INPUT_ELEMENT_DESC t = { "TEXCOORD", 2, DXGI_FORMAT_R32G32_FLOAT, count, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 };
			desc[count] = t;
			count++;
		}
		if (vertexAttr & VERTEX_ATTR_FLAG_TEXCOORD3) {
			D3D11_INPUT_ELEMENT_DESC t = { "TEXCOORD", 3, DXGI_FORMAT_R32G32_FLOAT, count, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 };
			desc[count] = t;
			count++;
		}
		if (vertexAttr & VERTEX_ATTR_FLAG_TANGENT) {
			D3D11_INPUT_ELEMENT_DESC t = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, count, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 };
			desc[count] = t;
			count++;
		}
		if (vertexAttr & VERTEX_ATTR_FLAG_BITANGENT) {
			D3D11_INPUT_ELEMENT_DESC t = { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, count, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 };
			desc[count] = t;
			count++;
		}

		HRESULT hr = GraphicsCore::GetDevice()->CreateInputLayout(desc, count, shader.GetByteCode(), shader.GetByteCodeSize(), &layout.layout);
		Assert(SUCCEEDED(hr));
		layout.shaderAttr = shader.GetVertexAttribute();
		layout.vertexAttr = vertexAttr;

		return &layout;
	}

	/* ********************************************************************************************* */

	VertexShader::VertexShader()
		: shader_(nullptr)
		, blob_(nullptr)
		, data_(nullptr)
		, dataSize_(0)
	{
	}

	VertexShader::~VertexShader()
	{
		COMPTR_RELEASE(shader_);
		COMPTR_RELEASE(blob_);
	}

	void VertexShader::CreateFromByteCode(const void* data, int size, ShaderReflection* reflection)
	{
		HRESULT hr = GraphicsCore::GetDevice()->CreateVertexShader(data, size, nullptr, &shader_);
		THROW_IF_FAILED(hr);
		data_ = data;
		dataSize_ = size;

		ShaderReflection ref;
		ref.Create(data_, dataSize_);
		vertexAttribute_ = ref.GetVertexLayoutAttribute();

		if (reflection) {
			reflection->Create(data, size);
		}
	}

	void VertexShader::CompileFromFile(const char* fileName, const char* entryPoint, ShaderReflection* reflection)
	{
		CompileShaderFromFile(fileName, entryPoint, "vs_5_0", &blob_);
		HRESULT hr = GraphicsCore::GetDevice()->CreateVertexShader(blob_->GetBufferPointer(), blob_->GetBufferSize(), nullptr, &shader_);
		THROW_IF_FAILED(hr);
		data_ = blob_->GetBufferPointer();
		dataSize_ = blob_->GetBufferSize();

		ShaderReflection ref;
		ref.Create(data_, dataSize_);
		vertexAttribute_ = ref.GetVertexLayoutAttribute();

		if (reflection) {
			reflection->Create(data_, dataSize_);
		}
	}

	void VertexShader::CompileFromString(const char* source, int length, const char* entryPoint, ShaderReflection* reflection)
	{
		CompileShaderFromString(source, length, entryPoint, "vs_5_0", &blob_);
		HRESULT hr = GraphicsCore::GetDevice()->CreateVertexShader(blob_->GetBufferPointer(), blob_->GetBufferSize(), nullptr, &shader_);
		THROW_IF_FAILED(hr);
		data_ = blob_->GetBufferPointer();
		dataSize_ = blob_->GetBufferSize();

		ShaderReflection ref;
		ref.Create(data_, dataSize_);
		vertexAttribute_ = ref.GetVertexLayoutAttribute();

		if (reflection) {
			reflection->Create(data_, dataSize_);
		}
	}

	void VertexShader::Destroy()
	{
		COMPTR_RELEASE(shader_);
		COMPTR_RELEASE(blob_);
	}


	/* ********************************************************************************************* */

	PixelShader::PixelShader()
		: shader_(nullptr)
	{
	}

	PixelShader::~PixelShader()
	{
		COMPTR_RELEASE(shader_);
	}

	void PixelShader::CreateFromByteCode(const void* data, int size, ShaderReflection* reflection)
	{
		HRESULT hr = GraphicsCore::GetDevice()->CreatePixelShader(data, size, nullptr, &shader_);
		THROW_IF_FAILED(hr);

		if (reflection) {
			reflection->Create(data, size);
		}
	}

	void PixelShader::CompileFromFile(const char* fileName, const char* entryPoint, ShaderReflection* reflection)
	{
		ID3DBlob* blob = nullptr;
		CompileShaderFromFile(fileName, entryPoint, "ps_5_0", &blob);
		HRESULT hr = GraphicsCore::GetDevice()->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &shader_);
		THROW_IF_FAILED(hr);

		if (reflection) {
			reflection->Create(blob->GetBufferPointer(), blob->GetBufferSize());
		}
		COMPTR_RELEASE(blob);
	}

	void PixelShader::CompileFromString(const char* source, int length, const char* entryPoint, ShaderReflection* reflection)
	{
		ID3DBlob* blob = nullptr;
		CompileShaderFromString(source, length, entryPoint, "ps_5_0", &blob);
		HRESULT hr = GraphicsCore::GetDevice()->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &shader_);
		THROW_IF_FAILED(hr);

		if (reflection) {
			reflection->Create(blob->GetBufferPointer(), blob->GetBufferSize());
		}
		COMPTR_RELEASE(blob);
	}

	void PixelShader::Destroy()
	{
		COMPTR_RELEASE(shader_);
	}

	/* ********************************************************************************************* */

	void ShaderManager::Initialize(const char* directoryPath)
	{
		try {
			// シェーダ定義ファイル読み込み
			std::string directory = directoryPath;
			directory += "\\";
			std::string definisionFile = directory + "shaders.json";
			picojson::value json;
			std::ifstream stream(definisionFile);
			stream >> json;
			stream.close();
			picojson::array& defines = json.get<picojson::array>();

			// シェーダコンパイル
			auto hasher = std::hash<std::string>();
			for (auto& s : defines) {
				auto& obj = s.get<picojson::object>();
				std::string name = obj["Name"].get<std::string>();
				std::string fileName = directory + obj["FileName"].get<std::string>();
				std::string vs = obj["VSEntry"].get<std::string>();
				std::string ps = obj["PSEntry"].get<std::string>();

				size_t shaderHash = hasher(name);
				auto pair = shaderMap_.emplace(shaderHash, ShaderSet());
				Assert(pair.second);
				ShaderSet& shader = pair.first->second;
				
				Printf("Shader Compile / %s : %s\n", name.c_str(), fileName.c_str());
				shader.vs_.CompileFromFile(fileName.c_str(), vs.c_str());
				if (ps.length() > 0) {
					shader.ps_.CompileFromFile(fileName.c_str(), ps.c_str());
				}
			}
			directoryPath_ = directoryPath;
		} catch (...) {
			Printf("Shader Compile Failed.\n");
			shaderMap_.clear();
		}
	}

	void ShaderManager::Finalize()
	{
		shaderMap_.clear();
	}

	void ShaderManager::Reload()
	{
		try {
			// シェーダ定義ファイル読み込み
			std::string directory = directoryPath_;
			directory += "\\";
			std::string definisionFile = directory + "shaders.json";
			picojson::value json;
			std::ifstream stream(definisionFile);
			stream >> json;
			stream.close();
			picojson::array& defines = json.get<picojson::array>();

			// シェーダコンパイル
			auto hasher = std::hash<std::string>();
			for (auto& s : defines) {
				auto& obj = s.get<picojson::object>();
				std::string name = obj["Name"].get<std::string>();
				std::string fileName = directory + obj["FileName"].get<std::string>();
				std::string vs = obj["VSEntry"].get<std::string>();
				std::string ps = obj["PSEntry"].get<std::string>();

				size_t shaderHash = hasher(name);
				ShaderSet* shader = Find(shaderHash);
				if (shader) {
					// 元のシェーダを破棄
					shader->vs_.Destroy();
					shader->ps_.Destroy();

					Printf("Shader Compile / %s : %s\n", name.c_str(), fileName.c_str());
					shader->vs_.CompileFromFile(fileName.c_str(), vs.c_str());
					if (ps.length() > 0) {
						shader->ps_.CompileFromFile(fileName.c_str(), ps.c_str());
					}
				}
			}
		} catch (...) {
			Printf("Shader Compile Failed.\n");
			shaderMap_.clear();
		}
	}

}