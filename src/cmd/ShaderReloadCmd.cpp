//
// Copyright (c) GANBARION Co., Ltd. All rights reserved.
// This code is licensed under the MIT License (MIT).
//

#include "ShaderReloadCmd.h"
#include "bridge/DAGManager.h"

void* ShaderReloadCmd::creator()
{
	return new ShaderReloadCmd();
}

MStatus ShaderReloadCmd::doIt(const MArgList& args)
{
	MStatus s;
	try {
		// オプション
		for (uint32_t i = 0; i < args.length(); i++) {
			auto arg = args.asString(i, &s);
			if (!s) continue;

			/* 特になし */
		}

		// シェーダのリロード
		se::ShaderManager::Get().Reload();
	}
	catch (MString err) {
		MString str = "error \"" + err + " exception / customViewportShaderReload.\"";
		MGlobal::executeCommand(str);
		return MStatus::kFailure;
	}

	// リフレッシュ
	M3dView::active3dView().refresh(true);

	return MStatus::kSuccess;
}
