//
// Copyright (c) GANBARION Co., Ltd. All rights reserved.
// This code is licensed under the MIT License (MIT).
//

#pragma once

#include "Common.h"

/**
 * レンダラメイン
 */
class CustomRenderOverride : public MHWRender::MRenderOverride
{
public:
	// レンダーオペレーション
	enum {
		kUserOp,			// ユーザー(独自描画エンジンの描画はここで)
		kUIItemOp,			// ワイヤーフレーム等のMayaUI
		kHUDOp,				// Maya HUD
		kToBackBufferOp,	// User, UIを描画したターゲットをバックバッファに転送
		kPresentOp,			// Viewportにpresent

		kOperationCount
	};

	// レンダーターゲット
	enum {
		kColor = 0,		// color
		kDepth,			// depth

		kTargetCount
	};

	// シェーダ
	enum {
		kCopyShader,

		kShaderCount
	};

protected:
	MString									uiName_;
	int										currentOperation_;
	MHWRender::MRenderOperation*			renderOperations_[kOperationCount];
	MHWRender::MRenderTargetDescription*	targetDescriptions_[kTargetCount];
	MHWRender::MRenderTarget*				targets_[kTargetCount];
	MHWRender::MShaderInstance*				shaders_[kShaderCount];
	MString									renderOperationNames_[kOperationCount];
	MString									targetOverrideNames_[kTargetCount];

protected:
	bool InitializeEngine();
	void FinalizeEngine();
	MStatus updateRenderTargets(MHWRender::MRenderer* theRenderer, const MHWRender::MRenderTargetManager* targetManager);
	MStatus updateShaders(const MHWRender::MShaderManager* shaderMgr);

public:
	CustomRenderOverride(const MString& name);
	virtual ~CustomRenderOverride();

	virtual MHWRender::DrawAPI supportedDrawAPIs() const override;
	virtual bool startOperationIterator() override;
	virtual MHWRender::MRenderOperation* renderOperation() override;
	virtual bool nextRenderOperation() override;
	virtual MStatus setup(const MString& destination) override;
	virtual MStatus cleanup() override;
	virtual MString uiName() const  override { return uiName_; }
};
