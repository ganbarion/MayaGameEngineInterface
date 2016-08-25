//
// Copyright (c) GANBARION Co., Ltd. All rights reserved.
// This code is licensed under the MIT License (MIT).
//

#include <maya/MString.h>
#include <maya/MFnPlugin.h>
#include <maya/MViewport2Renderer.h>
#include "CustomRenderOverride.h"
#include "nodes/CustomViewportGlobals.h"
#include "cmd/ShaderReloadCmd.h"

namespace {
	CustomRenderOverride* renderOverrideInstance = nullptr;
}


MStatus initializePlugin(MObject obj)
{
	MStatus status;
	MFnPlugin plugin(obj, "Ganbarion", "1.0.0.0", "Any");

	// 調整用ノード
	status = plugin.registerNode("customViewportGlobals", CustomViewportGlobals::id, CustomViewportGlobals::creator, CustomViewportGlobals::initialize);
	if (!status) {
		status.perror("registerNode / customViewportGlobals.");
		return status;
	}

	// レンダラ
	MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer();
	if (renderer) {
		if (!renderOverrideInstance) {
			try {
				renderOverrideInstance = new CustomRenderOverride("customViewport");
				renderer->registerOverride(renderOverrideInstance);
			} catch (...) {
				return MStatus::kFailure;
			}
		}
	}

	// コマンド
	status = plugin.registerCommand("customViewportShaderReload", ShaderReloadCmd::creator);
	if (!status) {
		status.perror("registerCommand");
		return status;
	}

	return status;
}

MStatus uninitializePlugin(MObject obj)
{
	MStatus status;
	MFnPlugin plugin(obj);

	MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer();
	if (renderer) {
		if (renderOverrideInstance) {
			renderer->deregisterOverride(renderOverrideInstance);
			delete renderOverrideInstance;
		}
		renderOverrideInstance = nullptr;
	}

	status = plugin.deregisterNode(CustomViewportGlobals::id);
	if (!status) {
		status.perror("deregisterNode / customViewportGlobals.");
		return status;
	}

	status = plugin.deregisterCommand("customViewportShaderReload");
	if (!status) {
		status.perror("deregisterCommand");
		return status;
	}

	return status;
}

