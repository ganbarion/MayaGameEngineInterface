//
// Copyright (c) GANBARION Co., Ltd. All rights reserved.
// This code is licensed under the MIT License (MIT).
//

#pragma once

#include "Common.h"

/**
 * レンダラグローバル設定ノード
 */
class CustomViewportGlobals : public MPxNode
{
public:
	static	MTypeId		id;

public:
	static MObject fxaaEnable_;

private:

public:
	CustomViewportGlobals();
	virtual	~CustomViewportGlobals();

	virtual MStatus compute(const MPlug& plug, MDataBlock& data) override;

	static void* creator();
	static MStatus initialize();
};



