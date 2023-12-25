#GLSL
#shader vertex
#version 420 core


layout(location = 0) in vec4 v_position;
layout(location = 1) in mat4 v_model;


layout(std140, binding = 0) uniform ConstantBuffer
{
	mat4 u_projection;
	mat4 u_view;
	mat4 u_model;
};

void main()
{
	gl_Position = ((u_projection * u_view) * v_model) * v_position;
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
	matrix u_projection;
	matrix u_view;
	matrix u_model;
};

struct VIn
{
    float3 position : POSITION;
	matrix model : MODEL;
};


struct VOut
{
	float4 p_position : SV_POSITION;
};


VOut VShader(VIn input, uint instanceID : SV_InstanceID)
{
	VOut output;
	output.p_position = mul(float4(input.position, 1.0), transpose(mul(mul(u_projection,u_view),input.model)));

	return output;
}

#shader fragment

float4 PShader(float4 position : SV_POSITION) : SV_TARGET
{
	return float4(1,0,0,1);
}
#END