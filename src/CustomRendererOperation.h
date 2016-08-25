//
// Copyright (c) GANBARION Co., Ltd. All rights reserved.
// This code is licensed under the MIT License (MIT).
//

#pragma once

#include "Common.h"
#include "MainScene.h"


/**
 * User custom operation
 */
class CustomRenderOperation : public MHWRender::MUserRenderOperation
{
protected:
	MHWRender::MRenderTarget** targets_;
	MainScene scene_;
	MString panelName_;

	se::ColorBuffer renderTarget_;
	se::SamplerState samplerState_;
	se::IndexBuffer quadFillIndexBuffer_;

	bool fxaaEnable_;

private:
	void updateRenderSettings();
	void updateIsolateSelect();

public:
	CustomRenderOperation(const MString& name);
	virtual ~CustomRenderOperation();

	virtual MStatus execute(const MHWRender::MDrawContext& drawContext) override;
	virtual MayaRenderTargets targetOverrideList(unsigned int& listSize) override;

	void setRenderTargets(MHWRender::MRenderTarget** targets, int32_t num);
	void setPanelName(const MString& str) { panelName_ = str; }
};


/**
 * UI items render
 */
class UIItemsRenderOperation : public MHWRender::MSceneRender
{
protected:
	MHWRender::MRenderTarget** targets_;
	uint32_t targetNum_;
	MString panelName_;

public:
	UIItemsRenderOperation(const MString& name)
		: MSceneRender(name)
		, targets_(nullptr)
		, targetNum_(0)
	{
		float val[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
		mClearOperation.setClearColor(val);
	}

	virtual ~UIItemsRenderOperation()
	{
		targets_ = nullptr;
	}

	virtual MayaRenderTargets targetOverrideList(unsigned int& listSize) override
	{
		if (targets_) {
			listSize = targetNum_;
			return targets_;
		}
		return nullptr;
	}

	virtual MHWRender::MClearOperation& clearOperation() override
	{
		mClearOperation.setMask(0);
		return mClearOperation;
	}

	virtual MHWRender::MSceneRender::MSceneFilterOption renderFilterOverride() override
	{
		return MHWRender::MSceneRender::kRenderUIItems;
	}

	void setRenderTargets(MHWRender::MRenderTarget** targets, uint32_t num) { 
		targets_ = targets; 
		targetNum_ = num;
	}
	void setPanelName(const MString& str) { panelName_ = str; }
};


/**
 * HUD render
 */
class HUDRenderOperation : public MHWRender::MHUDRender
{
protected:
	MHWRender::MRenderTarget** targets_;
	uint32_t targetNum_;

public:
	HUDRenderOperation(const MString& name) 
		: targets_(nullptr)
		, targetNum_(0)
	{
	}
	~HUDRenderOperation() {}

	virtual MayaRenderTargets targetOverrideList(unsigned int& listSize) override
	{
		if (targets_) {
			listSize = targetNum_;
			return targets_;
		}
		return nullptr;
	}

	void setRenderTargets(MHWRender::MRenderTarget** targets, uint32_t num) {
		targets_ = targets;
		targetNum_ = num;
	}
};


/**
 * Quad render
 */
class ToBackBufferOperation : public MHWRender::MQuadRender
{
protected:
	MHWRender::MShaderInstance* shader_;

public:
	ToBackBufferOperation(const MString& name)
		: MQuadRender(name)
		, shader_(nullptr)
	{
	}

	~ToBackBufferOperation()
	{
		shader_ = nullptr;
	}

	virtual MHWRender::MClearOperation& clearOperation() override
	{
		mClearOperation.setMask((unsigned int)MHWRender::MClearOperation::kClearAll);
		return mClearOperation;
	}

	virtual const MHWRender::MShaderInstance* shader() override { return shader_; }

	void setShader(MHWRender::MShaderInstance* shader) { shader_ = shader; }
};


/**
 * Present operation to present to screen
 */
class PresentOperation : public MHWRender::MPresentTarget
{
public:
	PresentOperation(const MString& name)
		: MPresentTarget(name)
	{}
	virtual ~PresentOperation() {}
};