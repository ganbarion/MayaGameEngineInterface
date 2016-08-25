//
// Copyright (c) GANBARION Co., Ltd. All rights reserved.
// This code is licensed under the MIT License (MIT).
//

#include "Maya.inl.h"

// テクニック定義用マクロ
#define DEFINE_MAYA_TECHNIQUE(name) technique11 name \
{\
	pass p0\
	{\
		SetRasterizerState(FillCullFront);\
		SetBlendState(BSAlphaBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);\
	    SetDepthStencilState(DepthEnabling, 0);\
		SetVertexShader(CompileShader(vs_5_0, name##_VS()));\
		SetPixelShader(CompileShader(ps_5_0, name##_PS()));\
	}\
}

#define UV(a)		float2(a.x, 1.0 - a.y)


struct VS_Input
{
	float4 a_position 	: POSITION;
	float4 a_color 		: COLOR0;
	float2 a_texcoord0 	: TEXCOORD0;
};

struct VS_Output
{
	float4 v_position 		: SV_POSITION;
	float4 v_outcolor0 		: TEXCOORD0;
	float2 v_texcoord0 		: TEXCOORD1;
};


// Shader List
VS_Output OneTexture_VS(VS_Input input)
{
	VS_Output output = (VS_Output)0;
	output.v_position = mul(input.a_position, u_wvp_matrix);
	output.v_texcoord0 = input.a_texcoord0;
	return output;
}
float4 OneTexture_PS(VS_Output input) : SV_Target
{
	return Texture0.Sample(s_sampler0, UV(input.v_texcoord0));
}

VS_Output SimpleMesh_VS(VS_Input input)
{
	VS_Output output = (VS_Output)0;
	output.v_position = mul(input.a_position, u_wvp_matrix);
	return output;
}
float4 SimpleMesh_PS(VS_Output input) : SV_Target
{
	return float4(1, 1, 1, 1);
}



// Texhnique List
DEFINE_MAYA_TECHNIQUE(SimpleMesh)
DEFINE_MAYA_TECHNIQUE(OneTexture)
