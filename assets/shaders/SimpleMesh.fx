//
// Copyright (c) GANBARION Co., Ltd. All rights reserved.
// This code is licensed under the MIT License (MIT).
//

#include "Common.h"

cbuffer ViewParameters : register(b0) {
	ViewParameterData View;
};
cbuffer ObjectParameters : register(b1) {
	ObjectParameterData Object;
};

struct VS_Input
{
	float4 a_position 	: POSITION;
};

struct VS_Output
{
	float4 v_position 	: SV_POSITION;
};

VS_Output VS(VS_Input input)
{
	VS_Output output;

	float4 worldPos = mul(input.a_position, Object.localToWorld);
	output.v_position = mul(worldPos, View.worldToClip);
	return output;
}

float4 PS(VS_Output input) : SV_Target
{
	return 1.0f;
}
