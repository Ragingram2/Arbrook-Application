#GLSL
#shader vertex
#version 420 core


layout(location = 0) in vec4 v_position;
//layout(location = 1) in vec2 v_texcoord;
layout(location = 2) in mat4 v_model;


layout(std140, binding = 0) uniform ConstantBuffer
{
	mat4 u_vp;
};

out vec2 a_texcoord;

void main()
{
	gl_Position = (u_vp * v_model * v_position);
    //a_texcoord = v_texcoord;
}

#shader fragment
#version 420 core

out vec4 FragColor;
//in vec2 a_texcoord;

//uniform sampler2D u_texture;

void main()
{
    FragColor = vec4(1.0,0.0,0.0,1.0);
	//FragColor = texture(u_texture, a_texcoord);
}
#END

#HLSL
#shader vertex

cbuffer ConstantBuffer : register(b0)
{
	matrix u_vp;
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
	output.p_position = mul(float4(input.position, 1.0), transpose(mul(u_vp,input.model)));

	return output;
}

#shader fragment

float4 PShader(float4 position : SV_POSITION) : SV_TARGET
{
	return float4(1,0,0,1);
}
#END