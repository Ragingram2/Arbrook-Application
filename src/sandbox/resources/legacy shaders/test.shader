#GLSL
#shader vertex
#version 420 core

layout(location = 0) in vec3 v_position;

layout(std140, binding = 0) uniform ConstantBuffer
{
	vec3 u_position;
};

void main()
{
	gl_Position = vec4(v_position + u_position, 1);
}

#shader fragment
#version 420 core

out vec4 FragColor;

void main()
{
	FragColor = vec4(1, 0, 0, 1);
}
#END

#HLSL
#shader vertex

cbuffer ConstantBuffer : register(b0)
{
	float3 u_position = float3(0, 0, 0);
};

struct VOut
{
	float4 p_position : SV_POSITION;
};

VOut VShader(float3 position : POSITION)
{
	VOut output;

	output.p_position = float4(position + u_position, 1);

	return output;
}

#shader fragment

float4 PShader(float4 position : SV_POSITION) : SV_TARGET
{
	return float4(1,0,0,1);
}
#END