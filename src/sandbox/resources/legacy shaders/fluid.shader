#GLSL
#shader vertex
#version 420 core

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec2 v_texCoord;

out vec2 TexCoord;

void main()
{
	gl_Position = vec4(v_position, 1);
	TexCoord = v_texCoord;
}

#shader fragment
#version 420 core

#define SIZE 32

layout(std140, binding = 0) uniform ConstantBuffer
{
		vec4 source[256];
};

in vec2 TexCoord;
out vec4 FragColor;

void main()
{
	int x = int((TexCoord.x) * SIZE);
	int y = int((TexCoord.y) * SIZE);

	int idx = int(x + (y * SIZE));
	float val = source[idx / 4][idx % 4];
	//float val = source[idx];
	FragColor = vec4(vec3(val), 1.0);
}
#END

#HLSL
#shader vertex

struct VOut
{
	float4 p_position : SV_POSITION;
};

VOut VShader(float4 position : POSITION) 
{
	VOut output;
	output.p_position = position;
	return output;
}

#shader fragment
Texture2D m_texture;
SamplerState m_sampler;

float4 PShader(float4 position : SV_POSITION) : SV_TARGET
{
	return float4(1.0,0.0,0.0,1.0);
}
#END