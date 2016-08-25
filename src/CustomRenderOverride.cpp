//
// Copyright (c) GANBARION Co., Ltd. All rights reserved.
// This code is licensed under the MIT License (MIT).
//

#include "CustomRenderOverride.h"
#include "CustomRendererOperation.h"
#include "bridge/DAGManager.h"
#include <shlobj.h>


CustomRenderOverride::CustomRenderOverride(const MString& name)
	: MRenderOverride(name)
	, uiName_("MayaCustomViewport")
{
	// エンジン初期化
	InitializeEngine();

	// DAG管理
	bridge::DAGManager::CreateInstance();

	for (int i = 0; i < kShaderCount; i++) {
		shaders_[i] = nullptr;
	}

	// レンダーターゲット
	targetOverrideNames_[kColor] = MString("__CustomRenderOverride_Color_Target1__");
	targetDescriptions_[kColor] = new MHWRender::MRenderTargetDescription(targetOverrideNames_[kColor], 256, 256, 1, MHWRender::kR8G8B8A8_UNORM, 0, false);
	targets_[kColor] = nullptr;
	targetOverrideNames_[kDepth] = MString("__CustomRenderOverride_Depth_Target__");
	targetDescriptions_[kDepth] = new MHWRender::MRenderTargetDescription(targetOverrideNames_[kDepth], 256, 256, 1, MHWRender::kD32_FLOAT, 0, false);
	targets_[kDepth] = nullptr;

	// オペレーション生成
	currentOperation_ = -1;
	renderOperationNames_[kUserOp] = "_CustomRenderOverride_User";
	renderOperations_[kUserOp] = new CustomRenderOperation(renderOperationNames_[kUserOp]);
	renderOperationNames_[kUIItemOp] = "_CustomRenderOverride_UIItemRender";
	renderOperations_[kUIItemOp] = new UIItemsRenderOperation(renderOperationNames_[kUIItemOp]);
	renderOperationNames_[kHUDOp] = "_CustomRenderOverride_HUD";
	renderOperations_[kHUDOp] = new HUDRenderOperation(renderOperationNames_[kHUDOp]);
	renderOperationNames_[kToBackBufferOp] = "_CustomRenderOverride_ToBackBuffer";
	renderOperations_[kToBackBufferOp] = new ToBackBufferOperation(renderOperationNames_[kToBackBufferOp]);
	renderOperationNames_[kPresentOp] = "_CustomRenderOverride_PresentTarget";
	renderOperations_[kPresentOp] = new PresentOperation(renderOperationNames_[kPresentOp]);
}

CustomRenderOverride::~CustomRenderOverride()
{
	for (uint32_t i = 0; i < kOperationCount; i++) {
		delete renderOperations_[i];
		renderOperations_[i] = nullptr;
	}

	MHWRender::MRenderer* theRenderer = MHWRender::MRenderer::theRenderer();
	if (theRenderer) {
		// Release shaders
		const MHWRender::MShaderManager* shaderMgr = theRenderer->getShaderManager();
		for (uint32_t i = 0; i < kShaderCount; i++) {
			if (shaders_[i]) {
				if (shaderMgr)
					shaderMgr->releaseShader(shaders_[i]);
				shaders_[i] = nullptr;
			}
		}

		// Release targets
		const MHWRender::MRenderTargetManager* targetManager = theRenderer->getRenderTargetManager();
		for (uint32_t i = 0; i < kTargetCount; i++) {
			if (targetDescriptions_[i]) {
				delete targetDescriptions_[i];
				targetDescriptions_[i] = nullptr;
			}

			if (targets_[i]) {
				if (targetManager) {
					targetManager->releaseRenderTarget(targets_[i]);
				}
				targets_[i] = nullptr;
			}
		}
	}

	bridge::DAGManager::DestroyInstance();
	FinalizeEngine();
}

bool CustomRenderOverride::InitializeEngine()
{
	MHWRender::MRenderer* theRenderer = MHWRender::MRenderer::theRenderer();
	if (theRenderer->drawAPI() != MHWRender::DrawAPI::kDirectX11) {
		MDisplayError("[MayaCustomViewport] / MayaのレンダリングエンジンがDirectX11ではありません.");
		throw "MayaのレンダリングエンジンがDirectX11ではありません.";
		return false;
	}

	// データディレクトリ
	TCHAR myDocuments[MAX_PATH];
	::SHGetFolderPath(nullptr, CSIDL_MYDOCUMENTS, nullptr, 0, myDocuments);
	std::string dataDirectory(myDocuments);
	dataDirectory += "\\maya\\plug-ins\\MayaCustomViewport";

	ID3D11Device* dxDevice = (ID3D11Device*)theRenderer->GPUDeviceHandle();
	if (!dxDevice) return false;

	// エンジン
	se::GraphicsCore::InitializeByExternalDevice(dxDevice);
	std::string shaderDirectory = dataDirectory + "\\shaders";
	se::ShaderManager::Get().Initialize(shaderDirectory.c_str());
	MDisplayInfo("MayaCustomViewport initialized. / %s", dataDirectory.c_str());

	return true;
}

void CustomRenderOverride::FinalizeEngine()
{
	se::ShaderManager::Get().Finalize();
	se::GraphicsCore::Finalize();
	MDisplayInfo("MayaCustomViewport Finalized.");
}

MHWRender::DrawAPI CustomRenderOverride::supportedDrawAPIs() const
{
	return MHWRender::kDirectX11;
}

bool CustomRenderOverride::startOperationIterator()
{
	currentOperation_ = 0;
	return true;
}

MHWRender::MRenderOperation* CustomRenderOverride::renderOperation()
{
	if (currentOperation_ < kOperationCount) {
		return renderOperations_[currentOperation_];
	}
	return nullptr;
}

bool CustomRenderOverride::nextRenderOperation()
{
	currentOperation_++;
	if (currentOperation_ < kOperationCount) {
		return true;
	}
	return false;
}

MStatus CustomRenderOverride::updateRenderTargets(MHWRender::MRenderer* theRenderer, const MHWRender::MRenderTargetManager* targetManager)
{
	if (!targetManager || !theRenderer)
		return MStatus::kFailure;

	// 現在のターゲットのサイズを取得
	uint32_t targetWidth, targetHeight;
	theRenderer->outputTargetSize(targetWidth, targetHeight);

	// ターゲットのリサイズ
	for (uint32_t targetId = 0; targetId < kTargetCount; ++targetId) {
		targetDescriptions_[targetId]->setWidth(targetWidth);
		targetDescriptions_[targetId]->setHeight(targetHeight);

		if (!targets_[targetId]) {
			targets_[targetId] = targetManager->acquireRenderTarget(*targetDescriptions_[targetId]);
		} else {
			targets_[targetId]->updateDescription(*targetDescriptions_[targetId]);
		}
	}

	// レンダーターゲットを各オペレーションにセット
	CustomRenderOperation* userOp = (CustomRenderOperation*)renderOperations_[kUserOp];
	userOp->setRenderTargets(targets_, kTargetCount);
	UIItemsRenderOperation* uiItem = (UIItemsRenderOperation*)renderOperations_[kUIItemOp];
	uiItem->setRenderTargets(targets_, kTargetCount);
	HUDRenderOperation* hudOp = (HUDRenderOperation*)renderOperations_[kHUDOp];
	hudOp->setRenderTargets(targets_, kTargetCount);

	return MStatus::kSuccess;
}

MStatus CustomRenderOverride::updateShaders(const MHWRender::MShaderManager* shaderMgr)
{
	if (!shaders_[kCopyShader]) {
		shaders_[kCopyShader] = shaderMgr->getEffectsFileShader("Copy", "");
	}

	// Mayaからシェーダを取得してオぺレーションにセット
	ToBackBufferOperation* quadOp = (ToBackBufferOperation*)renderOperations_[kToBackBufferOp];
	if (quadOp) {
		quadOp->setShader(shaders_[kCopyShader]);
		if (targets_[kColor]) {
			MHWRender::MRenderTargetAssignment assignment;
			assignment.target = targets_[kColor];
			shaders_[kCopyShader]->setParameter("gInputTex", assignment);
		}
	}
	return MStatus::kSuccess;
}

MStatus CustomRenderOverride::setup(const MString& destination)
{
	MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer();
	if (!renderer) return MStatus::kFailure;
	const MHWRender::MShaderManager* shaderMgr = renderer->getShaderManager();
	if (!shaderMgr) return MStatus::kFailure;
	const MHWRender::MRenderTargetManager *targetManager = renderer->getRenderTargetManager();
	if (!targetManager) return MStatus::kFailure;

	// ターゲット更新
	MStatus status = updateRenderTargets(renderer, targetManager);
	if (!status) return status;

	// シェーダ更新
	status = updateShaders(shaderMgr);
	if (!status) return status;

	// パネル名を設定
	if (renderOperations_[kUserOp]) {
		CustomRenderOperation* op = static_cast<CustomRenderOperation*>(renderOperations_[kUserOp]);
		op->setPanelName(destination.asChar());
	}
	if (renderOperations_[kUIItemOp]) {
		UIItemsRenderOperation* op = static_cast<UIItemsRenderOperation*>(renderOperations_[kUIItemOp]);
		op->setPanelName(destination.asChar());
	}

	return MStatus::kSuccess;
}

MStatus CustomRenderOverride::cleanup()
{
	currentOperation_ = -1;

	// レンダーターゲットをクリア
	CustomRenderOperation* userOp = static_cast<CustomRenderOperation*>(renderOperations_[kUserOp]);
	userOp->setRenderTargets(nullptr, 0);
	UIItemsRenderOperation* uiItem = (UIItemsRenderOperation*)renderOperations_[kUIItemOp];
	uiItem->setRenderTargets(nullptr, 0);
	HUDRenderOperation* hudOp = (HUDRenderOperation*)renderOperations_[kHUDOp];
	hudOp->setRenderTargets(nullptr, 0);

	return MStatus::kSuccess;
}
