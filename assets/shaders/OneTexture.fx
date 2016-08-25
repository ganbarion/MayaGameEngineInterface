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
Texture2D s_texture0 : register(t0);
SamplerState s_sampler0 : register(s0);

struct VS_Input
{
	float4 a_position 	: POSITION;
	float2 a_texcoord0	: TEXCOORD0;
};

struct VS_Output
{
	float4 v_position 	: SV_POSITION;
	float2 v_texcoord0	: TEXCOORD0;
};

VS_Output VS(VS_Input input)
{
	VS_Output output;

	float4 worldPos = mul(input.a_position, Object.localToWorld);
	output.v_position = mul(worldPos, View.worldToClip);
	output.v_texcoord0 = input.a_texcoord0;
	return output;
}

float4 PS(VS_Output input) : SV_Target
{
	return s_texture0.Sample(s_sampler0, input.v_texcoord0);
}
