//
// Copyright (c) GANBARION Co., Ltd. All rights reserved.
// This code is licensed under the MIT License (MIT).
//

#include <bridge/DAGMaterial.h>
#include <bridge/DAGTexture.h>
#include <bridge/DAGManager.h>

// NodeDirtyコールバックを使用する
// (GUIのスライダーでの設定だとAttributeChangedにはマウスを離した時しかこないのでDirtyも使用する)
#define USE_NODE_PLUG_DIRTY_CALLBACK	1

namespace bridge {

	namespace {

		int32_t GetTextureUniformIndex(const MString& attr_name)
		{
			// dx11Shaderのテクスチャアサインユニフォーム名
			// dx11Shaderのシェーダソースに定義するテクスチャ名と一致させる必要がある
			static const char* _textureUniforms[] = {
				"Texture0",
				"Texture1",
				"Texture2",
				"Texture3",
				"Texture4",
				"Texture5",
				"Texture6",
				"Texture7",
				"Texture8",
			};

			for (int32_t i = 0; i < ARRAYSIZE(_textureUniforms); i++){
				if (attr_name == _textureUniforms[i]){
					return i;
				}
			}
			return -1;
		}

		int32_t GetNumericValue(float value[4], MPlug& plug)
		{
			MObject o; 
			plug.getValue(o);
			MFnNumericData data(o);
			MFnNumericData::Type type;
			if (!o.isNull()) {
				type = data.numericType();
			} else {
				MFnNumericAttribute nattrfn(plug.attribute());
				type = nattrfn.unitType();
			}

			int count = 0;
			switch (type) {
				case MFnNumericData::kBoolean:
					value[0] = plug.asBool() ? 1.0f : 0.0f;
					count = 1;
					break;
				case MFnNumericData::kByte:
				case MFnNumericData::kChar:
				case MFnNumericData::kShort:
				case MFnNumericData::kInt:
					value[0] = static_cast<float>(plug.asInt());
					count = 1;
					break;
				case MFnNumericData::kFloat:
				case MFnNumericData::kDouble:
					value[0] = plug.asFloat();
					count = 1;
					break;
				case MFnNumericData::k2Int:
				case MFnNumericData::k3Int:
				case MFnNumericData::k2Short:
				case MFnNumericData::k3Short:
					count = plug.numChildren();
					for (int32_t i = 0; i < count; i++) {
						value[i] = static_cast<float>(plug.child(i).asInt());
					}
					break;
				case MFnNumericData::k2Float:
				case MFnNumericData::k3Float:
				case MFnNumericData::k2Double:
				case MFnNumericData::k3Double:
				case MFnNumericData::k4Double:
					count = plug.numChildren();
					for (int32_t i = 0; i < count; i++) {
						value[i] = plug.child(i).asFloat();
					}
					break;
			}
			return count;
		}

		MString GetVertexSourceString(MString& value)
		{
			MStringArray ary;
			value.split(':', ary);
			if (ary.length() > 1) {
				return ary[ary.length() - 1];
			}
			return value;
		}

	}

	
	DAGMaterial::DAGMaterial(MObject& object, bool connect)
		: DAGNode(object)
		, shadingNodeCallbackId_(0)
		, updated_(true)
		, shader_(nullptr)
	{
		texcoordSet_.setLength(se::VERTEX_ATTR_TEXCOORD_NUM);

		// 最初にすでに接続されているものを検索する(initialShadingGroup用)
		if (connect) {
			MStatus status;
			MFnDependencyNode fn(object, &status);
			MPlug plug = fn.findPlug("surfaceShader", &status);
			MPlugArray connectedElements;
			plug.connectedTo(connectedElements, true, false, &status);
			for (uint32_t i = 0; i < connectedElements.length(); i++){
				MObject shader = connectedElements[i].node(&status);
				if (!status) return;
				ConnectSurfaceShader(shader);
			}
		}
	}


	DAGMaterial::~DAGMaterial()
	{
		// シェーダとのコネクトコールバック解除
		DisconnectSurfaceShader();
	}


	void DAGMaterial::ConnectSurfaceShader(MObject shader)
	{
		MStatus status;

		// すでに連結されている場合コールバックを解除
		DisconnectSurfaceShader();

		// shadingNodeの変更コールバックを設定する
		shadingNodeCallbackId_ = MNodeMessage::addAttributeChangedCallback(shader, ShadingNodeAttributeChangeCallcack, this, &status);
		if (!status) {
			MDisplayError("[MayaCustomViewport] / MNodeMessage::addAttributeChangedCallback");
			return;
		}
#if USE_NODE_PLUG_DIRTY_CALLBACK
		shadingNodeDirtyCallbackId_ = MNodeMessage::addNodeDirtyPlugCallback(shader, ShadingNodeDirtyPlugChangeCallcack, this, &status);
		if (!status) {
			MDisplayError("[MayaCustomViewport] / MNodeMessage::addNodeDirtyPlugCallback");
			return;
		}
#endif

		MFnDependencyNode depNode(shader, &status);
		if (depNode.typeName() == "dx11Shader") {
			// 全アトリビュートからパラメータをセット(テクニックの設定まで含んでいる)
			for (uint32_t i = 0; i < depNode.attributeCount(); i++) {
				SetParameterByPlug(MPlug(shader, depNode.attribute(i)));
			}
		} else {
			// デフォルトをセット
			SetupDefaultShader();
		}

		connectedShadingNode_ = shader;
		MDisplayDebugInfo("surfaceShader connected: %s", depNode.name().asChar());
	}


	void DAGMaterial::DisconnectSurfaceShader()
	{
		// すでに連結されている場合コールバックを解除
		if (connectedShadingNode_.isValid()) {
			MStatus status = MMessage::removeCallback(shadingNodeCallbackId_);
			if (!status) {
				MDisplayError("[MayaCustomViewport] / MMessage::removeCallback");
				return;
			}
#if USE_NODE_PLUG_DIRTY_CALLBACK
			MMessage::removeCallback(shadingNodeDirtyCallbackId_);
			if (!status) {
				MDisplayError("[MayaCustomViewport] / MMessage::removeCallback");
				return;
			}
#endif
		}
		connectedShadingNode_ = MObjectHandle();	// 無効ハンドル
	}
	

	void DAGMaterial::ConnectTexture(MObject attr, MObject texture)
	{
		// バインドインデックスの取得
		MFnAttribute fnAttr(attr);
		int32_t texIdx = GetTextureUniformIndex(fnAttr.name());
		if (texIdx >= 0) {
			DAGNode* dagTexture = DAGManager::Get()->FindDAGNode(texture);
			SetTexture(texIdx, (DAGTexture*)dagTexture);
			MDisplayDebugInfo("Texture connected: %s", MFnDependencyNode(texture).name().asChar());
		}
	}


	void DAGMaterial::DisconnectTexture(MObject attr)
	{
		// バインドインデックスの取得
		MFnAttribute fnAttr(attr);
		int32_t texIdx = GetTextureUniformIndex(fnAttr.name());
		if (texIdx >= 0) {
			SetTexture(texIdx, nullptr);
		}
	}
	

	void DAGMaterial::AttributeChanged(MNodeMessage::AttributeMessage msg, MPlug& plug, MPlug& otherPlug)
	{
		MStatus status;

		// コネクションが確立された
		if (msg & MNodeMessage::kConnectionMade) {
			auto obj = plug.attribute(&status);
			if (status) {
				MFnAttribute attr(obj);
				MString attrName = attr.name();
				// surfaceShaderに連結されているShadingNodeを保持しておく
				if (attr.name() == "surfaceShader") {
					ConnectSurfaceShader(otherPlug.node());
					updated_ = true;
				} else if (attr.name() == "dagSetMembers") {
					// メッシュオブジェクトへの接続
					MObject mesh = otherPlug.node();
					DAGNode* dagMesh = DAGManager::Get()->FindDAGNode(mesh);
					if (dagMesh) {
						Connect(dagMesh);
					}
				}
			}
		} else if (msg & MNodeMessage::kConnectionBroken) {
			// コネクション切断
			auto obj = plug.attribute(&status);
			if (status) {
				MFnAttribute attr(obj);
				MString attrName = attr.name();
				if (attr.name() == "surfaceShader") {
					DisconnectSurfaceShader();
					// デフォルトのマテリアルにしておく
					SetupDefaultShader();
				} else if (attr.name() == "dagSetMembers") {
					// メッシュオブジェクトへの接続の切断
					MObject mesh = otherPlug.node();
					DAGNode* dagMesh = DAGManager::Get()->FindDAGNode(mesh);
					if (dagMesh) {
						Disconnect(dagMesh);
					}
				}
			}
		}
	}


	/**
	 * シェーディングノードの変更時に呼ばれる.
	 * UIでの調整終了時にくる.
	 */
	void DAGMaterial::ShadingNodeAttributeChangeCallcack(MNodeMessage::AttributeMessage msg, MPlug& plug, MPlug& otherPlug, void* obj)
	{
		DAGMaterial* owner = reinterpret_cast<DAGMaterial*>(obj);
		MStatus status;

		if ((msg & MNodeMessage::kConnectionMade) && (msg & MNodeMessage::kOtherPlugSet)) {
			MObject node = otherPlug.node(&status);
			MFnDependencyNode fnNode(node);

			// マテリアルインフォ
			if (node.apiType() == MFn::kMaterialInfo) {
				// テクスチャコネクションリストを取得
				MPlugArray connectPlugs;
				MFnDependencyNode ownNode(plug.node());
				status = ownNode.getConnections(connectPlugs);
				if (!status) return;

				for (uint32_t i = 0; i < connectPlugs.length(); i++) {
					auto& plug = connectPlugs[i];
					MPlugArray connectedElements;
					MFnAttribute fnAttr(plug.attribute());
					int32_t texIdx = GetTextureUniformIndex(fnAttr.name());
					if (texIdx >= 0) {
						plug.connectedTo(connectedElements, true, false, &status);
						for (uint32_t j = 0; j < connectedElements.length(); j++) {
							MObject texture = connectedElements[j].node(&status);
							if (status) owner->ConnectTexture(plug.attribute(), texture);
						}
					}
				}
			} else if (otherPlug.node().apiType() == MFn::kFileTexture) {
				owner->ConnectTexture(plug.attribute(), otherPlug.node());
			}
		} else if (msg & MNodeMessage::kConnectionBroken) {
			// テクステャバインド解除
			if (otherPlug.node().apiType() == MFn::kFileTexture) {
				owner->DisconnectTexture(plug.attribute());
			}
		} else if (msg & MNodeMessage::kAttributeSet) {
			// パラメータアトリビュート監視
			owner->SetParameterByPlug(plug);
		}
	}



	void DAGMaterial::ShadingNodeDirtyPlugChangeCallcack(MObject& node, MPlug& plug, void* obj)
	{
		// パラメータ更新
		DAGMaterial* owner = reinterpret_cast<DAGMaterial*>(obj);
		owner->SetParameterByPlug(plug);
	}


	void DAGMaterial::SetParameterByPlug(MPlug plug)
	{
		MStatus status;
		auto attr = plug.attribute(&status);

		// ShadingNodeのアトリビュート名と名前解決してパラメータを更新する
		if (status) {
			MFnAttribute fnAttr(attr);
			MString attrName = fnAttr.name();

#if 0
			// マテリアルパラメータ取得サンプル
			if (attr.hasFn(MFn::kNumericAttribute)) {
				float value[4] = { 0 };
				if (attrName == "albedo") {
					GetNumericValue(value, plug);
				}
			}
#endif

			// サーフェイスデータ
			if (attr.hasFn(MFn::kTypedAttribute)) {
				if (attrName == "technique") {
					SetTechniqueByPlug(plug);
				} else if (attrName == "Color0_Source") {
					colorSet_ = GetVertexSourceString(plug.asString());
				} else if (attrName == "Tangent0_Source") {
					tangentSet_ = GetVertexSourceString(plug.asString());
				} else if (attrName == "Binormal0_Source") {
					binormalSet_ = GetVertexSourceString(plug.asString());
				} else if (attrName == "TexCoord0_Source") {
					texcoordSet_[0] = GetVertexSourceString(plug.asString());
				} else if (attrName == "TexCoord1_Source") {
					texcoordSet_[1] = GetVertexSourceString(plug.asString());
				} else if (attrName == "TexCoord2_Source") {
					texcoordSet_[2] = GetVertexSourceString(plug.asString());
				} else if (attrName == "TexCoord3_Source") {
					texcoordSet_[3] = GetVertexSourceString(plug.asString());
				}
				// テクニックや頂点ソースの更新があった場合はジオメトリに更新するよう通知する
				NotifyUpdateConnectionAll();
			}
		}
	}


	void DAGMaterial::SetTechniqueByPlug(MPlug plug)
	{
		MString name = plug.asString();
		if (name.length() == 0) return;

		shader_ = se::ShaderManager::Get().Find(name.asChar());
		if (!shader_) SetupDefaultShader();
		technique_ = name;
	}


	void DAGMaterial::SetupDefaultShader()
	{
		// dx11Shader以外は基本的なシェーダを設定する
		shader_ = se::ShaderManager::Get().Find("SimpleMesh");
	}


	void DAGMaterial::SetTexture(int index, DAGTexture* texture)
	{
		if (dagTextures_.size() < (index + 1)) {
			dagTextures_.resize(index + 1);
		}
		dagTextures_[index] = texture;
	}


	void DAGMaterial::Update()
	{
		if (!connectedShadingNode_.isValid()) return;
		if(!updated_) return;
		updated_ = false;
	}
}