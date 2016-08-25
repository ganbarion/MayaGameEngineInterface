//
// Copyright (c) GANBARION Co., Ltd. All rights reserved.
// This code is licensed under the MIT License (MIT).
//

#ifndef RENDER_STATE_H_
#define RENDER_STATE_H_

/**
 * States
 */

BlendState BSAlphaBlending
{
	AlphaToCoverageEnable = FALSE;
	BlendEnable[0] = TRUE;
	SrcBlend = SRC_ALPHA;
	DestBlend = INV_SRC_ALPHA;
	BlendOp = ADD;
	SrcBlendAlpha = ONE;	// Required for hardware frame render alpha channel
	DestBlendAlpha = INV_SRC_ALPHA;
	BlendOpAlpha = ADD;
	RenderTargetWriteMask[0] = 0x0F;
};

DepthStencilState DepthEnabling
{
	DepthEnable = TRUE;
};

RasterizerState WireframeCullFront
{
	CullMode = FRONT;
	FillMode = WIREFRAME;
};
RasterizerState FillCullBack
{
	FillMode = SOLID;
	CullMode = BACK;
};
RasterizerState FillCullFront
{
	CullMode = FRONT;
	FillMode = SOLID;
};


/**
 * Texture & Sampler
 */

Texture2D Texture0
<
	string UIGroup = "Texture";
	string ResourceName = "";
	string ResourceType = "2D";
	int mipmaplevels = 0;
	int UIOrder = 201;
	int UVEditorOrder = 1;
>;
SamplerState s_sampler0
{
	Filter = MIN_MAG_MIP_LINEAR;
};

Texture2D Texture1
<
	string UIGroup = "Texture";
	string ResourceName = "";
	string ResourceType = "2D";
	int mipmaplevels = 0;
	int UIOrder = 211;
	int UVEditorOrder = 2;
>;
SamplerState s_sampler1
{
	Filter = MIN_MAG_MIP_LINEAR;
};


Texture2D Texture2
<
	string UIGroup = "Texture";
	string ResourceName = "";
	string ResourceType = "2D";
	int mipmaplevels = 0;
	int UIOrder = 221;
	int UVEditorOrder = 3;
>;
SamplerState s_sampler2
{
	Filter = MIN_MAG_MIP_LINEAR;
};


Texture2D Texture3
<
	string UIGroup = "Texture";
	string ResourceName = "";
	string ResourceType = "2D";
	int mipmaplevels = 0;
	int UIOrder = 231;
	int UVEditorOrder = 4;
>;
SamplerState s_sampler3
{
	Filter = MIN_MAG_MIP_LINEAR;
};


Texture2D Texture4
<
	string UIGroup = "Texture";
	string ResourceName = "";
	string ResourceType = "2D";
	int mipmaplevels = 0;
	int UIOrder = 241;
	int UVEditorOrder = 5;
>;
SamplerState s_sampler4
{
	Filter = MIN_MAG_MIP_LINEAR;
};

Texture2D Texture5
<
	string UIGroup = "Texture";
	string ResourceName = "";
	string ResourceType = "2D";
	int mipmaplevels = 0;
	int UIOrder = 241;
	int UVEditorOrder = 6;
>;
SamplerState s_sampler5
{
	Filter = MIN_MAG_MIP_LINEAR;
};

Texture2D Texture6
<
	string UIGroup = "Texture";
	string ResourceName = "";
	string ResourceType = "2D";
	int mipmaplevels = 0;
	int UIOrder = 241;
	int UVEditorOrder = 7;
>;
SamplerState s_sampler6
{
	Filter = MIN_MAG_MIP_LINEAR;
};

Texture2D Texture7
<
	string UIGroup = "Texture";
	string ResourceName = "";
	string ResourceType = "2D";
	int mipmaplevels = 0;
	int UIOrder = 241;
	int UVEditorOrder = 8;
>;
SamplerState s_sampler7
{
	Filter = MIN_MAG_MIP_LINEAR;
};

Texture2D Texture8
<
	string UIGroup = "Texture";
	string ResourceName = "";
	string ResourceType = "2D";
	int mipmaplevels = 0;
	int UIOrder = 241;
	int UVEditorOrder = 8;
>;
SamplerState s_sampler8
{
	Filter = MIN_MAG_MIP_LINEAR;
};

/**
 * ConstantBuffer
 */

cbuffer CB_PerObject : register( b0 )
{
	float4x4 u_model_view_mat	: WorldView 				< string UIWidget = "None"; >;
	float4x4 u_proj_mat 		: Projection 				< string UIWidget = "None"; >;
	float4x4 u_world_mat 		: World 					< string UIWidget = "None"; >;
	float4x4 u_normal_mat		: WorldViewInverseTranspose	< string UIWidget = "None"; >;
	float4x4 u_inv_view_mat		: ViewInverse				< string UIWidget = "None"; >;
	float4x4 u_world_inverse_mat	: WorldInverse 			< string UIWidget = "None"; >;
	float4x4 u_wvp_matrix		: WorldViewProjection		< string UIWidget = "None"; >;
	float4x4 u_normal_matrix	: WorldInverseTranspose		< string UIWidget = "None"; >;
}

#endif
