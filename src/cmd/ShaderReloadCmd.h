//
// Copyright (c) GANBARION Co., Ltd. All rights reserved.
// This code is licensed under the MIT License (MIT).
//

#pragma once

#include "Common.h"


/**
 * シェーダ再読み込み用コマンド
 */
class ShaderReloadCmd: public MPxCommand
{
public:
	ShaderReloadCmd() {};
	virtual      ~ShaderReloadCmd() {};

	virtual MStatus doIt(const MArgList& args) override;
	virtual bool  isUndoable() const override { return false; };

	static void* creator();
};