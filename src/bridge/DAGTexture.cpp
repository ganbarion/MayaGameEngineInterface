//
// Copyright (c) GANBARION Co., Ltd. All rights reserved.
// This code is licensed under the MIT License (MIT).
//

#include "bridge/DAGTexture.h"
#include "bridge/DAGManager.h"

namespace bridge {

	namespace {

		bool GetShadingEngineByShadingNode(MPlugArray& plug_ary, MObject shader)
		{
			MStatus s;
			MFnDependencyNode fnNode(shader);
			MPlug shaderOutColor = fnNode.findPlug("outColor", &s);
			bool isConnect = shaderOutColor.isConnected(&s);
			if (isConnect) {
				shaderOutColor.connectedTo(plug_ary, false, true, &s);
				if (s == MStatus::kSuccess)
					return true;
			}
			return false;
		}

	}


	DAGTexture::DAGTexture(MObject& object)
		: DAGNode(object)
		, texture_(nullptr)
		, engineTexture_(nullptr)
		, engineSampler_(nullptr)
		, mirror_u_(false)
		, mirror_v_(false)
		, wrap_u_(false)
		, wrap_v_(false)
	{
	}


	DAGTexture::~DAGTexture()
	{
		// 確保済みテクスチャを解放
		MHWRender::MRenderer* theRenderer = MHWRender::MRenderer::theRenderer();
		if (!theRenderer) return;
		MHWRender::MTextureManager* texManager = theRenderer->getTextureManager();
		if (!texManager) return;

		if (texture_) {
			texManager->releaseTexture(texture_);
			delete engineTexture_;
			texture_ = nullptr;
			engineTexture_ = nullptr;
		}
	}


	void DAGTexture::AttributeChanged(MNodeMessage::AttributeMessage msg, MPlug & plug, MPlug & otherPlug)
	{
		MStatus s;
		if (msg & MNodeMessage::kAttributeSet) {
			// ファイルテクスチャ名(fileTextureName)
			MObject attr = plug.attribute();
			MFnAttribute fnAttr(attr);
			if (fnAttr.shortName() == "ftn") {
				filePath_ = plug.asString();
				CreateTexture(filePath_);

				// 再描画リクエスト
				M3dView::active3dView().refresh(true);
			}
		} else if (msg & MNodeMessage::kAttributeEval) {
			// 本来であればplaced2dtextureにフックかけてハンドリングするべきだが
			// ここに来ること自体多いわけではないので常にアドレスモードの再評価をしている
			MFnAttribute fnAttr(plug.attribute());
			if (fnAttr.shortName() == "oc") {	// outColor
				EvaluateAddressingMode();
			}
		} else if (msg & MNodeMessage::kOtherPlugSet) {
			// ファイルを開いた時に来る
			MFnAttribute fnAttr(plug.attribute());
			if (fnAttr.shortName() == "mu"		// mirrorU
				|| fnAttr.shortName() == "mv"	// mirrorV
				|| fnAttr.shortName() == "wu"	// wrapU
				|| fnAttr.shortName() == "wv"	// wrapV
				) {
				EvaluateAddressingMode();
			}
		}
	}


	void DAGTexture::CreateTexture(MString path)
	{
		MStatus s;
		MHWRender::MRenderer* theRenderer = MHWRender::MRenderer::theRenderer();
		if (!theRenderer) return;
		MHWRender::MTextureManager* texManager = theRenderer->getTextureManager();
		if (!texManager) return;

		// 以前のテクスチャ解放
		if (texture_) {
			texManager->releaseTexture(texture_);
			delete engineTexture_;
			texture_ = nullptr;
			engineTexture_ = nullptr;
		}

		// テクスチャの取得
		// ここでは簡略化のためMHWRender::MTextureDescriptionを介してテクスチャを取得しているが、
		// ファイルパスはわかっているので描画エンジンが自前で生成してもよい.
		texture_ = texManager->acquireTexture(path);
		if (texture_) {
			MHWRender::MTextureDescription texDesc;
			texture_->textureDescription(texDesc);
			engineTexture_ = new se::Texture();

			auto* srv = static_cast<ID3D11ShaderResourceView*>(texture_->resourceHandle());
			engineTexture_->CreateFromSRV(srv);

			// テクスチャアドレッシングモードの評価
			EvaluateAddressingMode();
		}
	}


	void DAGTexture::EvaluateAddressingMode()
	{
		if (engineTexture_) {
			MStatus s;
			MFnDependencyNode fileNode(handle_.object());
			MPlug mirrorUPlug = fileNode.findPlug("mirrorU", &s);
			if (s != MStatus::kSuccess) return;
			MPlug mirrorVPlug = fileNode.findPlug("mirrorV", &s);
			if (s != MStatus::kSuccess) return;
			MPlug wrapUPlug = fileNode.findPlug("wrapU", &s);
			if (s != MStatus::kSuccess) return;
			MPlug wrapVPlug = fileNode.findPlug("wrapV", &s);
			if (s != MStatus::kSuccess) return;
			if (!mirrorUPlug.isNull()) mirrorUPlug.getValue(mirror_u_);
			if (!mirrorVPlug.isNull()) mirrorVPlug.getValue(mirror_v_);
			if (!wrapUPlug.isNull()) wrapUPlug.getValue(wrap_u_);
			if (!wrapVPlug.isNull()) wrapVPlug.getValue(wrap_v_);
			auto texAddrS = se::TAM_CLAMP;
			auto texAddrT = se::TAM_CLAMP;
			if (mirror_u_) texAddrS = se::TAM_MIRROR;
			else if (wrap_u_) texAddrS = se::TAM_WRAP;
			if (mirror_v_) texAddrT = se::TAM_MIRROR;
			else if (wrap_v_) texAddrT = se::TAM_WRAP;

			// サンプラ再生成
			if (engineSampler_) {
				delete engineSampler_;
				engineSampler_ = nullptr;
			}
			engineSampler_ = new se::SamplerState();
			engineSampler_->Create(se::FILTER_ANISOTROPIC_LINEAR, texAddrS, texAddrT);
		}
	}


	void DAGTexture::Update()
	{
		// do nothing
	}

}
