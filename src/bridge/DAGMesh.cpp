//
// Copyright (c) GANBARION Co., Ltd. All rights reserved.
// This code is licensed under the MIT License (MIT).
//

#include "bridge/DAGMesh.h"
#include "bridge/DAGMaterial.h"
#include "bridge/DAGTexture.h"
#include "bridge/DAGTransform.h"
#include "bridge/DAGManager.h"
#include "Utility.h"

namespace bridge {

	namespace {
		// 指定したシェーダがアサインされているポリゴンを取得する
		void GetShaderAsignPolygonIndex(MIntArray* output, MIntArray& poly_shader_ids, int32_t shader_id)
		{
			int32_t numPolys = poly_shader_ids.length();

			// 一旦数を数え上げ
			int32_t shaderPolyCount = 0;
			for (int32_t i = 0; i < numPolys; i++) {
				if (poly_shader_ids[i] == shader_id) {
					shaderPolyCount++;
				}
			}
			// バッファにプッシュ
			int32_t count = 0;
			output->setLength(shaderPolyCount);
			for (int32_t i = 0; i < numPolys; i++) {
				if (poly_shader_ids[i] == shader_id) {
					(*output)[count++] = i;
				}
			}
		}

		// アトリビュートを持っているか
		inline bool HasAttr(uint32_t flag, uint32_t id)
		{
			return (flag & (1 << id)) != 0;
		}

		// ノードのVisibilityを取得
		inline bool GetNodeVisible(const DAGTransform* node)
		{
			return node->IsVisible()
				&& (!DAGManager::Get()->IsIsolateSelected() || node->IsIsolateSelected());
		}
	}

	DAGMesh::DAGMesh(MObject& object)
		: DAGNode(object, true)
		, updated_(false)
	{
	}


	DAGMesh::~DAGMesh()
	{
	}


	void DAGMesh::AttributeChanged(MNodeMessage::AttributeMessage msg, MPlug& plug, MPlug& otherPlug)
	{
		// マテリアルとの接続チェック
		MStatus status;
		if (msg & MNodeMessage::kConnectionMade) {
			auto obj = otherPlug.attribute(&status);
			if (status) {
				MFnAttribute otherAttr(obj);
				if (otherPlug.node().apiType() == MFn::kShadingEngine) {
					MString attrName = otherAttr.name();
					if (otherAttr.name() == "dagSetMembers") {
						DAGNode* dagMaterial = DAGManager::Get()->FindDAGNode(otherPlug.node());
						if (dagMaterial) {
							Connect(dagMaterial);
						}
					}
				}
			}
		} else if (msg & MNodeMessage::kConnectionBroken) {
			auto obj = otherPlug.attribute(&status);
			if (status) {
				MFnAttribute otherAttr(obj);
				if (otherPlug.node().apiType() == MFn::kShadingEngine) {
					if (otherAttr.name() == "dagSetMembers") {
						DAGNode* dagMaterial = DAGManager::Get()->FindDAGNode(otherPlug.node());
						if (dagMaterial) {
							Disconnect(dagMaterial);
						}
					}
				}
			}
		}

		// ジオメトリの更新フラグを立てる
		updated_ = true;
	}

	void DAGMesh::NodeDirtyPlug(MObject& node, MPlug& plug)
	{
		// スキニング等でデフォームした場合inMesh(shortname:i)の更新通知がくる
		MFnAttribute aFn(plug.attribute());
		if (aFn.shortName() == "i") {
			updated_ = true;
		}
	}

	void DAGMesh::Update()
	{
		if (!handle_.isValid()) return;


		for (auto& pair : uniformMap_) {
			if (!pair.first->IsVisible()) continue;

			// 更新があるならモデル更新
			if (updated_) {
				UpdateGeometry();
				updated_ = false;
			}

			// トランスフォーム更新
			auto& data = pair.second;
			if (data.updated) {
				data.uniforms.Contents().localToWorld = Matrix44::Transpose(pair.first->GetWorldMatrix());
				data.uniforms.Updated();
				pair.second.updated = false;
			}
		}
	}

	void DAGMesh::Draw(se::GraphicsContext& context, ShadingPath path)
	{
		if (path != ShadingPath::MainPath) return;

		// ユニフォーム更新
		for (auto& pair : uniformMap_) {
			if (GetNodeVisible(pair.first)) {
				pair.second.uniforms.Update(context);
			}
		}

		// メッシュを描画
		auto iter = meshes_.begin();
		for (; iter != meshes_.end(); iter++) {
			DAGMaterial* material = iter->material;
			const se::ShaderSet* shader = material->GetEngineShader();
			if (!shader) continue;

			context.SetVertexShader(shader->GetVS());
			context.SetPixelShader(shader->GetPS());
			context.SetIndexBuffer(iter->indexBuffer);
			for(uint32_t i = 0; i < static_cast<uint32_t>(iter->vertexBuffers.size()); i++) {
				context.SetVertexBuffer(i, iter->vertexBuffers[i]);
			}
			context.SetInputLayout(*iter->layout);
			context.SetPrimitiveType(se::PRIMITIVE_TYPE_TRIANGLE_LIST);
			for (uint32_t i = 0; i < material->GetDAGTextureNum(); i++) {
				auto* texture = material->GetDAGTexture(i);
				if (!texture || !texture->GetEngineTexture() || !texture->GetEngineSampler()) continue;
				
				context.SetPSResource(i, *texture->GetEngineTexture());
				context.SetPSSamplerState(i, *texture->GetEngineSampler());
			}

			// ステート
			context.SetBlendState(se::BlendState::Get(se::BlendState::Opacity));
			context.SetDepthStencilState(se::DepthStencilState::Get(se::DepthStencilState::WriteEnable));
			context.SetRasterizerState(se::RasterizerState::Get(se::RasterizerState::BackFaceCull));

			// 全インスタンス分描画
			for (auto& pair : uniformMap_) {
				if (GetNodeVisible(pair.first)) {
					context.SetVSConstantBuffer(1, pair.second.uniforms.GetResource());
					context.DrawIndexed(0, iter->indexBuffer.GetIndexCount());
				}
			}
		}
	}


	void DAGMesh::NotifyUpdateConnection(const DAGNode* node)
	{
		// 外部から変更通知があった場合頂点レイアウトに影響を及ぼすので更新する
		updated_ = true;
	}

	void DAGMesh::LinkParent(const DAGNode* parent)
	{
		const DAGTransform* transform = static_cast<const DAGTransform*>(parent);
		auto pair = uniformMap_.emplace(transform, TransformData());
		Assert(pair.second);	// すでにあるのはエラー

		auto& uniforms = pair.first->second;
		uniforms.updated = true;
	}

	void DAGMesh::UnlinkParent(const DAGNode* parent)
	{
		const DAGTransform* transform = static_cast<const DAGTransform*>(parent);
		auto iter = uniformMap_.find(transform);
		Assert(iter != uniformMap_.end());
		uniformMap_.erase(iter);
	}

	void DAGMesh::NotifyParentTransformUpdated(const DAGNode* parent)
	{
		const DAGTransform* transform = static_cast<const DAGTransform*>(parent);
		auto iter = uniformMap_.find(transform);
		Assert(iter != uniformMap_.end());

		auto& uniforms = iter->second;
		uniforms.updated = true;
	}


	void DAGMesh::UpdateGeometry()
	{
		MStatus status;
		MFnMesh mesh(handle_.objectRef(), &status);
		if (!status) {
			MDisplayError("[MayaCustomViewport] / MFnMesh()\n");
			return;
		}
		MDagPath dagPath;
		status = mesh.getPath(dagPath);
		if (!status) {
			MDisplayError("[MayaCustomViewport] / DAGMesh::UpdateGeometry / mesh.getPath()");
			return;
		}

		// ポリゴン各面にアサインされているシェーダリストを取得する
		MObjectArray shaders;
		MIntArray shaderIndices;
		shaders.clear();
		shaderIndices.clear();
		if (!mesh.getConnectedShaders(0, shaders, shaderIndices)) {
			MDisplayError("[MayaCustomViewport] / %s / DAGMesh::UpdateGeometry / mesh.getConnectedShaders()", mesh.name().asChar());
			return;
		}
		int numShaders = shaders.length();
		if (numShaders <= 0) return;

		// シェーダ数だけメッシュを作成
		meshes_.clear();
		meshes_.resize(numShaders);
		for (int32_t i = 0; i < numShaders; i++) {
			// shadingEngine取得
			MObject shader = shaders[i];
			// コネクションリストからマテリアルインスタンスを取得
			DAGMaterial* dagMat = static_cast<DAGMaterial*>(FindConnectedItem(shader));
			if (!dagMat || !dagMat->GetEngineShader()) {
				MDisplayError("[MayaCustomViewport] / メッシュに接続されているシェーディングエンジンから必要なミラーノード見つかりませんでした。%s / %s"
					, Name().asChar(), MFnDependencyNode(shader).name().asChar());
				meshes_.clear();
				return;
			}
			meshes_[i].material = dagMat;
			uint32_t attrFlag = dagMat->GetEngineShader()->GetVS().GetVertexAttribute();

			// シェーダがアサインされているポリゴンリストを取得
			MIntArray shadingPolyIds;
			shadingPolyIds.clear();
			GetShaderAsignPolygonIndex(&shadingPolyIds, shaderIndices, i);

			MHWRender::MGeometryRequirements requirements;
			auto& vb = meshes_[i].vertexBuffers;
			auto& ib = meshes_[i].indexBuffer;

			// インデックス要項
			MFnSingleIndexedComponent comp;
			MObject compObj = comp.create(MFn::kMeshPolygonComponent);
			comp.addElements(shadingPolyIds);	// IDによる制御
			MHWRender::MIndexBufferDescriptor triangleDesc(MHWRender::MIndexBufferDescriptor::kTriangle, "", MHWRender::MGeometry::kTriangles, 3, compObj);
			requirements.addIndexingRequirement(triangleDesc);

			// 頂点要項
			MHWRender::MVertexBufferDescriptor posDesc("", MHWRender::MGeometry::kPosition, MHWRender::MGeometry::kFloat, 3);
			MHWRender::MVertexBufferDescriptor normalDesc("", MHWRender::MGeometry::kNormal, MHWRender::MGeometry::kFloat, 3);
			MHWRender::MVertexBufferDescriptor tangentDesc(dagMat->GetTangentSet(), MHWRender::MGeometry::kTangent, MHWRender::MGeometry::kFloat, 3);
			MHWRender::MVertexBufferDescriptor binormalDesc(dagMat->GetBinormalSet(), MHWRender::MGeometry::kBitangent, MHWRender::MGeometry::kFloat, 3);
			MHWRender::MVertexBufferDescriptor colorDesc(dagMat->GetColorSet(), MHWRender::MGeometry::kColor, MHWRender::MGeometry::kFloat, 4);
			MHWRender::MVertexBufferDescriptor uvDesc[se::VERTEX_ATTR_TEXCOORD_NUM];
			for (int32_t j = 0; j < ARRAYSIZE(uvDesc); j++){
				uvDesc[j] = MHWRender::MVertexBufferDescriptor(dagMat->GetTexcoordSet(j), MHWRender::MGeometry::kTexture, MHWRender::MGeometry::kFloat, 2);
			}

			requirements.addVertexRequirement(posDesc);
			if (HasAttr(attrFlag, se::VERTEX_ATTR_NORMAL)) {
				requirements.addVertexRequirement(normalDesc);
			}
			if (HasAttr(attrFlag, se::VERTEX_ATTR_TANGENT)) {
				requirements.addVertexRequirement(tangentDesc);
			}
			if (HasAttr(attrFlag, se::VERTEX_ATTR_BITANGENT)) {
				requirements.addVertexRequirement(binormalDesc);
			}
			if (HasAttr(attrFlag, se::VERTEX_ATTR_COLOR)) {
				requirements.addVertexRequirement(colorDesc);
			}
			for (uint32_t j = 0; j < se::VERTEX_ATTR_TEXCOORD_NUM; j++) {
				if (!HasAttr(attrFlag, se::VERTEX_ATTR_TEXCOORD0 + j)) break;
				requirements.addVertexRequirement(uvDesc[j]);
			}

			// 抽出器
			MHWRender::MGeometryExtractor extractor(requirements, dagPath, true, &status);
			if (!status) {
				MDisplayError("[MayaCustomViewport] / MHWRender::MGeometryExtractor()");
				meshes_.clear();
				return;
			}
			uint32_t numVertices = extractor.vertexCount();
			uint32_t numTriangles = extractor.primitiveCount(triangleDesc);
			uint32_t minBufferSize = extractor.minimumBufferSize(numTriangles, triangleDesc.primitive());
			//MDisplayInfo("material index: %d / triNum: %d / instanceCount: %d", i, numTriangles, mesh.parentCount());

			// 頂点データを抽出器から取得
			std::unique_ptr<uint32_t[]> triangleIdx(new uint32_t[minBufferSize]);
			std::unique_ptr<float[]> vertices(new float[numVertices * posDesc.stride()]);
			std::unique_ptr<float[]> normals;
			std::unique_ptr<float[]> colors;
			std::unique_ptr<float[]> tangents;
			std::unique_ptr<float[]> binormals;
			std::unique_ptr<float[]> uvs[se::VERTEX_ATTR_TEXCOORD_NUM];

			vb.resize(requirements.vertexRequirements().length());
			{
				// インデックスバッファ取得
				if (numTriangles != 0) {
					if (!extractor.populateIndexBuffer(triangleIdx.get(), numTriangles, triangleDesc)) {
						MDisplayError("[MayaCustomViewport] / Failed populateIndexBuffer.");
						meshes_.clear();
						return;
					}
					ib.Create(triangleIdx.get(), sizeof(uint32_t) * numTriangles * 3, se::INDEX_BUFFER_STRIDE_U32);
				}

				// 各種バッファ取得
				uint32_t bufferCounter = 0;
				// pos
				if (!extractor.populateVertexBuffer(vertices.get(), numVertices, posDesc)) {
					MDisplayError("[MayaCustomViewport] / Failed populateVertexBuffer / position.");
					meshes_.clear();
					return;
				}
				vb[bufferCounter].Create(vertices.get(), sizeof(float) * 3 * numVertices, se::VERTEX_ATTR_FLAG_POSITION, se::BUFFER_USAGE_DEFAULT, true);
				bufferCounter++;

				// normal
				if (HasAttr(attrFlag, se::VERTEX_ATTR_NORMAL)) {
					normals.reset(new float[numVertices * normalDesc.stride()]);
					if (!extractor.populateVertexBuffer(normals.get(), numVertices, normalDesc)) {
						MDisplayWarning("[MayaCustomViewport] / Failed populateVertexBuffer / normal.");
					}
					vb[bufferCounter].Create(normals.get(), sizeof(float) * 3 * numVertices, se::VERTEX_ATTR_FLAG_NORMAL);
					bufferCounter++;
				}

				// color
				if (HasAttr(attrFlag, se::VERTEX_ATTR_COLOR)) {
					colors.reset(new float[numVertices * colorDesc.stride()]);
					if (!extractor.populateVertexBuffer(colors.get(), numVertices, colorDesc)) {
						MDisplayWarning("[MayaCustomViewport] / Failed populateVertexBuffer / color.");
						// 初期値でバッファを埋める
						memset(colors.get(), 0, sizeof(float) * colorDesc.stride() * numVertices);
					}
					vb[bufferCounter].Create(colors.get(), sizeof(float) * 4 * numVertices, se::VERTEX_ATTR_FLAG_COLOR);
					bufferCounter++;
				}

				// uv
				for (int32_t j = 0; j < se::VERTEX_ATTR_TEXCOORD_NUM; j++) {
					if (!HasAttr(attrFlag, se::VERTEX_ATTR_TEXCOORD0 + j)) break;

					int stride = uvDesc[j].stride();
					uvs[j].reset(new float[numVertices * stride]);
					float* ptr = uvs[j].get();
					if (!extractor.populateVertexBuffer(ptr, numVertices, uvDesc[j])) {
						MDisplayWarning("[MayaCustomViewport] / Failed populateVertexBuffer / texcoord%d.", j);
					}
					// UVをOpenGL->DirectX変換するためにVを反転(TODO:コンピュートシェーダに移動した方がCPUを圧迫しない)
					int num = numVertices * stride;
					for (int32_t k = 1; k < num; k += stride) {
						ptr[k] = 1.0f - ptr[k];
					}
					vb[bufferCounter].Create(uvs[j].get(), 
											sizeof(float) * 2 * numVertices, 
											1 << (se::VERTEX_ATTR_TEXCOORD0 + j),
											se::BUFFER_USAGE_DEFAULT,
											true);
					bufferCounter++;
				}

				// tangent
				if (HasAttr(attrFlag, se::VERTEX_ATTR_TANGENT)) {
					tangents.reset(new float[numVertices * tangentDesc.stride()]);
					if (!extractor.populateVertexBuffer(tangents.get(), numVertices, tangentDesc)) {
						MDisplayWarning("[MayaCustomViewport] / Failed populateVertexBuffer / tangent.");
					}
					vb[bufferCounter].Create(tangents.get(), sizeof(float) * 3 * numVertices, se::VERTEX_ATTR_FLAG_TANGENT);
					bufferCounter++;
				}

				// binormal
				if (HasAttr(attrFlag, se::VERTEX_ATTR_BITANGENT)) {
					binormals.reset(new float[numVertices * binormalDesc.stride()]);
					if (!extractor.populateVertexBuffer(binormals.get(), numVertices, binormalDesc)) {
						MDisplayWarning("[MayaCustomViewport] / Failed populateVertexBuffer / binormal.");
					}
					vb[bufferCounter].Create(binormals.get(), sizeof(float) * 3 * numVertices, se::VERTEX_ATTR_FLAG_BITANGENT);
					bufferCounter++;
				}

				// 頂点レイアウト算出
				auto& vs = dagMat->GetEngineShader()->GetVS();
				meshes_[i].layout = se::VertexLayoutManager::Get().GetLayout(vs, attrFlag);
				Assert(meshes_[i].layout);
			}
		}
	}

}
