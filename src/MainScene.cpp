//
// Copyright (c) GANBARION Co., Ltd. All rights reserved.
// This code is licensed under the MIT License (MIT).
//

#include "MainScene.h"
#include "bridge/DAGManager.h"
#include "Utility.h"


MainScene::MainScene()
{
}


MainScene::~MainScene()
{
}


void MainScene::Update(const MHWRender::MDrawContext& drawContext, MDagPath cameraPath)
{
#if 0
	// カメラ情報取得サンプル
	if (cameraPath.isValid()) {
		MStatus status;
		MFnCamera camera(cameraPath, &status);
		if (!status) return;
		bool isOrtho = camera.isOrtho(&status);
		if (!status) return;

		int x, y, w, h;
		drawContext.getViewportDimensions(x, y, w, h);
		float aspect = (float)w / h;
		MPoint eyePoint = camera.eyePoint(MSpace::kWorld);
		MPoint lookAtPt = camera.centerOfInterestPoint(MSpace::kWorld);
		MVector	upDirection = camera.upDirection(MSpace::kWorld);
		MMatrix projection = drawContext.getMatrix(MHWRender::MFrameContext::kProjectionMtx);
		double nearClippingPlane = camera.nearClippingPlane();
		double farClippingPlane = camera.farClippingPlane();

		if (!isOrtho) {
			// persepective
			float fovy = (float)camera.verticalFieldOfView();
		} else {
			// ortho
			float width = (float)camera.orthoWidth() * 0.5f;
			float height = width / aspect;
		}
	} 
#endif

	// drawContextからマトリクス取得
	MMatrix view = drawContext.getMatrix(MHWRender::MFrameContext::kViewMtx);
	MMatrix projection = drawContext.getMatrix(MHWRender::MFrameContext::kProjectionMtx);
	MMatrix viewProjection = drawContext.getMatrix(MHWRender::MFrameContext::kViewProjMtx);

	// ユニフォームにセット
	Matrix44 matrix;
	auto& uniform = viewUniforms_.Contents();
	CopyMatrix(&matrix, view);
	uniform.worldToView = Matrix44::Transpose(matrix);
	CopyMatrix(&matrix, projection);
	uniform.viewToClip = Matrix44::Transpose(matrix);
	CopyMatrix(&matrix, viewProjection);
	uniform.worldToClip = Matrix44::Transpose(matrix);
	viewUniforms_.Updated();

	// DAG更新
	auto* dagMgr = bridge::DAGManager::Get();
	dagMgr->UpdateNode();
}


void MainScene::Draw(se::GraphicsContext& context)
{
	// ビューユニフォーム
	viewUniforms_.Update(context);
	context.SetVSConstantBuffer(0, viewUniforms_.GetResource());
	context.SetPSConstantBuffer(0, viewUniforms_.GetResource());

	// 描画
	bridge::DAGManager::Get()->DrawNode(context, ShadingPath::MainPath);
	bridge::DAGManager::Get()->DrawNode(context, ShadingPath::UIPath);
}