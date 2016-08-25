//
// Copyright (c) GANBARION Co., Ltd. All rights reserved.
// This code is licensed under the MIT License (MIT).
//

#define FXAA_PC 1
#define FXAA_HLSL_5 1
#define FXAA_GREEN_AS_LUMA 1
#define FXAA_QUALITY__PRESET 12

#include "FXAA3_11.h"

struct FxaaVS_Output
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};

FxaaVS_Output FxaaVS(uint id : SV_VertexID)
{
	FxaaVS_Output Output;
	Output.Tex = float2((id << 1) & 2, id & 2);
	Output.Pos = float4(Output.Tex * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
	return Output;
}

SamplerState InputSampler : register(s0);
Texture2D InputTexture : register(t0);

float4 FxaaPS(FxaaVS_Output Input) : SV_TARGET
{
	float2 rcpFrame;
	InputTexture.GetDimensions(rcpFrame.x, rcpFrame.y);
	rcpFrame = rcp(rcpFrame);

	FxaaTex InputFXAATex = { InputSampler, InputTexture };
	return FxaaPixelShader(
		Input.Tex.xy,							// FxaaFloat2 pos,
		FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f),		// FxaaFloat4 fxaaConsolePosPos,
		InputFXAATex,							// FxaaTex tex,
		InputFXAATex,							// FxaaTex fxaaConsole360TexExpBiasNegOne,
		InputFXAATex,							// FxaaTex fxaaConsole360TexExpBiasNegTwo,
		rcpFrame.xy,							// FxaaFloat2 fxaaQualityRcpFrame,
		FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f),		// FxaaFloat4 fxaaConsoleRcpFrameOpt,
		FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f),		// FxaaFloat4 fxaaConsoleRcpFrameOpt2,
		FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f),		// FxaaFloat4 fxaaConsole360RcpFrameOpt2,
		0.75f,									// FxaaFloat fxaaQualitySubpix,
		0.166f,									// FxaaFloat fxaaQualityEdgeThreshold,
		0.0f,									// FxaaFloat fxaaQualityEdgeThresholdMin,
		0.0f,									// FxaaFloat fxaaConsoleEdgeSharpness,
		0.0f,									// FxaaFloat fxaaConsoleEdgeThreshold,
		0.0f,									// FxaaFloat fxaaConsoleEdgeThresholdMin,
		FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f)		// FxaaFloat fxaaConsole360ConstDir,
	);
}
