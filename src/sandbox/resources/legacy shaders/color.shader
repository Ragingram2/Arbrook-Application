#GLSL
#shader vertex
#version 420 core

layout(location = 0) in vec4 v_position;

layout(std140, binding = 0) uniform CameraBuffer
{
	vec3 u_viewPosition;
	mat4 u_projection;
	mat4 u_view;
	mat4 u_model;
};


void main()
{
	gl_Position = ((u_projection * u_view) * u_model) * v_position;
}

#shader fragment
#version 420 core

out vec4 FragColor;

void main()
{
	FragColor = vec4(1.0, 1.0, 0.75, 1.0);
}
#END

#HLSL
#shader vertex

cbuffer CameraBuffer : register(b0)
{
	float4 u_viewPosition;
	matrix u_projection;
	matrix u_view;
	matrix u_model;
};

struct VOut
{
	float4 p_position : SV_POSITION;
};

VOut VShader(float4 position : POSITION)
{
	VOut output;
	matrix model = u_model;
	matrix view = u_view;
	matrix projection = u_projection;
	output.p_position = mul(mul(mul(projection, view), model), position);

	return output;
}

#shader fragment

float4 PShader(float4 position : SV_POSITION) : SV_TARGET
{
	return float4(1.0,0.0,0.0,1.0);
}
#END
