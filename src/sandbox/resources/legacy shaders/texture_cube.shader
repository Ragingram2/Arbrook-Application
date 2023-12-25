#GLSL
#shader vertex
#version 420 core

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec2 v_texcoord;

layout(std140, binding = 0) uniform ConstantBuffer
{
	mat4 u_mvp;
};

out vec2 a_texcoord;

void main()
{
	gl_Position = u_mvp * vec4(v_position, 1.0);
    a_texcoord = v_texcoord;
}

#shader fragment
#version 420 core

out vec4 FragColor;
in vec2 a_texcoord;

uniform sampler2D u_texture;

void main()
{
	FragColor = texture(u_texture, a_texcoord);
}
#END

#HLSL
#shader vertex

cbuffer ConstantBuffer : register(b0)
{
	matrix u_mvp;
};

struct VOut
{
	float4 p_position : SV_POSITION;
};

VOut VShader(float3 position : POSITION)
{
	VOut output;

    output.p_position = mul(float4(position, 1.0f), transpose(u_mvp));

	return output;
}

#shader fragment

float4 PShader(float4 position : SV_POSITION) : SV_TARGET
{
	return float4(1,0,0,1);
}
#END