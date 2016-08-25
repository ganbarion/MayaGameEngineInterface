//
// Copyright (c) GANBARION Co., Ltd. All rights reserved.
// This code is licensed under the MIT License (MIT).
//

#pragma once

#include "Common.h"


/**
 * メインシーン
 */
class MainScene
{
private:
	se::TUniformParameter<se::ViewParameterData> viewUniforms_;

public:
	MainScene();
	virtual ~MainScene();

	void Update(const MHWRender::MDrawContext& drawContext, MDagPath cameraPath);
	void Draw(se::GraphicsContext& context);
};



