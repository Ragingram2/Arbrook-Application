#GLSL
#shader vertex
#version 420 core

layout(location = 0) in vec4 v_position;
//layout(location = 1) in vec2 v_texCoord;
layout(location = 2) in mat4 v_mvp;

out vec2 TexCoord;

void main()
{
	gl_Position = v_mvp * v_position;
	//TexCoord = v_texCoord;
}

#shader fragment
#version 420 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D ourTexture;

void main()
{
	//vec4 tColor = texture(ourTexture, TexCoord);
	//FragColor = tColor;
	FragColor = vec4(1.0,0.0,0.0,1.0);
}
#END

#HLSL
#shader vertex
cbuffer ConstantBuffer : register(b0)
{
	float3 u_position;
	float u_time;
};

struct VOut
{
	float4 p_position : SV_POSITION;
	float4 p_color : COLOR;
	float2 p_texcoord : TEXCOORD;
};

VOut VShader(float3 position : POSITION, float4 color : COLOR, float2 texCoord : TEXCOORD)
{
	VOut output;

	float3 offset = float3(0, sin(u_time), 0);
	output.p_position = float4(position + u_position + offset, 1);
	output.p_color = color;
	output.p_texcoord = texCoord;

	return output;
}

#shader fragment
Texture2D m_texture;
SamplerState m_sampler;

float4 PShader(float4 position : SV_POSITION, float4 color : COLOR, float2 texCoord : TEXCOORD) : SV_TARGET
{
	return m_texture.Sample(m_sampler, texCoord);
}
#END