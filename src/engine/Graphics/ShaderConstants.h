//
// Copyright (c) GANBARION Co., Ltd. All rights reserved.
// This code is licensed under the MIT License (MIT).
//

#pragma once 

#include "engine/Math/Math.h"

namespace se
{

	/**
	 * ビューパラメータ
	 */
	struct ViewParameterData
	{
		float4x4 	worldToView;
		float4x4	viewToClip;
		float4x4 	worldToClip;
	};

	/**
	 * オブジェクトパラメータ
	 */
	struct ObjectParameterData
	{
		float4x4 localToWorld;
	};

}