#GLSL
#shader vertex
#version 420 core

layout (location = 0) in vec4 v_position;
layout (location = 1) in vec3 v_normal;
layout (location = 2) in vec2 v_texcoords;

layout(std140, binding = 0) uniform CameraBuffer
{
	vec3 u_viewPosition;
	mat4 u_projection;
	mat4 u_view;
	mat4 u_model;
};

out vec2 TexCoords;
out vec3 Normal;
out vec3 FragPos;

void main()
{
	gl_Position = ((u_projection * u_view) * u_model) * v_position;
	FragPos = vec3(u_model * v_position);
	//this should be moved to the cpu at some point
	Normal = mat3(transpose(inverse(u_model))) * v_normal;
	TexCoords = v_texcoords;
}

#shader fragment
#version 420 core

struct Light
{
	vec4 direction;
	vec4 position;
	vec4 color;
	float range;
	float intensity;
};

#define NR_POINT_LIGHTS 8

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D Diffuse;
uniform sampler2D Specular;

layout(std140, binding = 0) uniform CameraBuffer
{
	vec3 u_viewPosition;
	mat4 u_projection;
	mat4 u_view;
	mat4 u_model;
};

layout(std140, binding = 1) uniform LightBuffer
{
	Light u_lights[NR_POINT_LIGHTS + 1];
} lightBuffer;

layout(std140, binding = 2) uniform MaterialBuffer
{
	float u_shininess;
} material;

out vec4 FragColor;

float Attenuation(vec3 lightPosition, vec3 fragPos, float attenuationRadius, float lightIntensity)
{
	float sqrlightDistance = pow(length(lightPosition - fragPos),2);
	float attenuation = pow(max(1.0 - (sqrlightDistance / (attenuationRadius * attenuationRadius)), 0.0), 2);
	return attenuation * lightIntensity;
}

vec3 CalcDirLight(Light light, vec3 normal, vec3 viewDir)
{
	vec3 lightDir = normalize(-light.direction.xyz);

	//diffuse
	float diff = max(dot(normal, lightDir), 0.0);
	//specular
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir),0.0), material.u_shininess);

	//combine results
	vec3 ambient = light.color.rgb * vec3(texture(Diffuse, TexCoords));
	vec3 diffuse = light.color.rgb * diff * vec3(texture(Diffuse, TexCoords));
	vec3 specular = light.color.rgb * spec * vec3(texture(Specular, TexCoords));
	
	return (ambient + diffuse + specular);
}

vec3 CalcPointLight(Light light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	vec3 lightDir = normalize(light.position.xyz - fragPos);

	//diffuse
	float diff = max(dot(normal, lightDir), 0.0);
	//specular
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir),0.0), material.u_shininess);

	//attenuation
	float attenuation = Attenuation(light.position.xyz, fragPos, light.range, light.intensity);
	if(attenuation <= 0)
    	return vec3(0);

	//combine results
	vec3 ambient = light.color.rgb * vec3(texture(Diffuse, TexCoords));
	vec3 diffuse = light.color.rgb * diff * vec3(texture(Diffuse, TexCoords));
	vec3 specular = light.color.rgb * spec * vec3(texture(Specular, TexCoords));

	ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
	
	return (ambient + diffuse + specular);
}

void main()
{
	vec3 norm = normalize(Normal);
	vec3 viewDir = normalize(u_viewPosition - FragPos);

	vec3 result = CalcDirLight(lightBuffer.u_lights[0], norm, viewDir);

	for(int i = 1; i < NR_POINT_LIGHTS+1; i++)
		result += CalcPointLight(lightBuffer.u_lights[i], norm, FragPos, viewDir);	

	FragColor = vec4(result, 1.0);
}
#END

#HLSL
#shader vertex

#define IDENTITY_MATRIX float4x4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1)

float4x4 inverse(float4x4 m) {
    float n11 = m[0][0], n12 = m[1][0], n13 = m[2][0], n14 = m[3][0];
    float n21 = m[0][1], n22 = m[1][1], n23 = m[2][1], n24 = m[3][1];
    float n31 = m[0][2], n32 = m[1][2], n33 = m[2][2], n34 = m[3][2];
    float n41 = m[0][3], n42 = m[1][3], n43 = m[2][3], n44 = m[3][3];

    float t11 = n23 * n34 * n42 - n24 * n33 * n42 + n24 * n32 * n43 - n22 * n34 * n43 - n23 * n32 * n44 + n22 * n33 * n44;
    float t12 = n14 * n33 * n42 - n13 * n34 * n42 - n14 * n32 * n43 + n12 * n34 * n43 + n13 * n32 * n44 - n12 * n33 * n44;
    float t13 = n13 * n24 * n42 - n14 * n23 * n42 + n14 * n22 * n43 - n12 * n24 * n43 - n13 * n22 * n44 + n12 * n23 * n44;
    float t14 = n14 * n23 * n32 - n13 * n24 * n32 - n14 * n22 * n33 + n12 * n24 * n33 + n13 * n22 * n34 - n12 * n23 * n34;

    float det = n11 * t11 + n21 * t12 + n31 * t13 + n41 * t14;
    float idet = 1.0f / det;

    float4x4 ret;

    ret[0][0] = t11 * idet;
    ret[0][1] = (n24 * n33 * n41 - n23 * n34 * n41 - n24 * n31 * n43 + n21 * n34 * n43 + n23 * n31 * n44 - n21 * n33 * n44) * idet;
    ret[0][2] = (n22 * n34 * n41 - n24 * n32 * n41 + n24 * n31 * n42 - n21 * n34 * n42 - n22 * n31 * n44 + n21 * n32 * n44) * idet;
    ret[0][3] = (n23 * n32 * n41 - n22 * n33 * n41 - n23 * n31 * n42 + n21 * n33 * n42 + n22 * n31 * n43 - n21 * n32 * n43) * idet;

    ret[1][0] = t12 * idet;
    ret[1][1] = (n13 * n34 * n41 - n14 * n33 * n41 + n14 * n31 * n43 - n11 * n34 * n43 - n13 * n31 * n44 + n11 * n33 * n44) * idet;
    ret[1][2] = (n14 * n32 * n41 - n12 * n34 * n41 - n14 * n31 * n42 + n11 * n34 * n42 + n12 * n31 * n44 - n11 * n32 * n44) * idet;
    ret[1][3] = (n12 * n33 * n41 - n13 * n32 * n41 + n13 * n31 * n42 - n11 * n33 * n42 - n12 * n31 * n43 + n11 * n32 * n43) * idet;

    ret[2][0] = t13 * idet;
    ret[2][1] = (n14 * n23 * n41 - n13 * n24 * n41 - n14 * n21 * n43 + n11 * n24 * n43 + n13 * n21 * n44 - n11 * n23 * n44) * idet;
    ret[2][2] = (n12 * n24 * n41 - n14 * n22 * n41 + n14 * n21 * n42 - n11 * n24 * n42 - n12 * n21 * n44 + n11 * n22 * n44) * idet;
    ret[2][3] = (n13 * n22 * n41 - n12 * n23 * n41 - n13 * n21 * n42 + n11 * n23 * n42 + n12 * n21 * n43 - n11 * n22 * n43) * idet;

    ret[3][0] = t14 * idet;
    ret[3][1] = (n13 * n24 * n31 - n14 * n23 * n31 + n14 * n21 * n33 - n11 * n24 * n33 - n13 * n21 * n34 + n11 * n23 * n34) * idet;
    ret[3][2] = (n14 * n22 * n31 - n12 * n24 * n31 - n14 * n21 * n32 + n11 * n24 * n32 + n12 * n21 * n34 - n11 * n22 * n34) * idet;
    ret[3][3] = (n12 * n23 * n31 - n13 * n22 * n31 + n13 * n21 * n32 - n11 * n23 * n32 - n12 * n21 * n33 + n11 * n22 * n33) * idet;

    return ret;
}

cbuffer CameraBuffer : register(b0)
{
	float4 u_viewPosition;
	matrix u_projection;
	matrix u_view;
	matrix u_model;
};

struct VIn
{
    float4 position : POSITION;
	float2 texCoords : TEXCOORD0;
    float3 normal : NORMAL0;
};

struct VOut
{
	float4 p_position : SV_POSITION;
	float2 p_texCoords : TEXCOORD0;
	float3 p_normal : NORMAL0;


	float3 p_fragPos : TEXCOORD1;
};

VOut VShader(VIn input)
{
	VOut output;

    output.p_position = mul(mul(mul(u_projection, u_view), u_model), input.position);
	output.p_fragPos = mul(u_model, input.position).rgb;
	output.p_normal = normalize(mul(input.normal, (float3x3)transpose(inverse(u_model))));
	output.p_texCoords = input.texCoords;

	return output;
}

#shader fragment

struct Light
{
	float4 direction;
	float4 position;
	float4 color;
	float range;
	float intensity;
};

#define NR_POINT_LIGHTS 8

Texture2D Diffuse : register(t0);
SamplerState diff_sampler : register(s0);
Texture2D Specular : register(t1);
SamplerState spec_sampler : register(s1); 

struct VIn
{
	float4 p_position : SV_POSITION;
	float2 p_texCoords : TEXCOORD0;
	float3 p_normal : NORMAL0;

	float3 p_fragPos : TEXCOORD1;
};

cbuffer CameraBuffer : register(b0)
{
	float4 u_viewPosition;
	matrix u_projection;
	matrix u_view;
	matrix u_model;
};


cbuffer LightBuffer : register(b1)
{
	Light u_lights[NR_POINT_LIGHTS + 1];
};

cbuffer MaterialBuffer : register(b2)
{
	float u_shininess;
};

float Attenuation(float3 lightPosition, float3 fragPos, float attenuationRadius, float lightIntensity)
{
	float sqrlightDistance = pow(length(lightPosition - fragPos), 2);
	float attenuation = pow(max(1.0 - (sqrlightDistance / (attenuationRadius * attenuationRadius)), 0.0), 2);
	return attenuation * lightIntensity;
}

float3 CalcDirLight(Light light, float3 normal, float3 viewDir, float2 texCoords)
{
	float3 lightDir = normalize(-light.direction.xyz);

	//diffuse
	float diff = max(dot(normal, lightDir),0.0);
	//specular
	float3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir,reflectDir),0.0), u_shininess);

	//combine results
	float3 ambient = light.color.rgb * float3(0.1,0.1,0.1) * Diffuse.Sample(diff_sampler, texCoords);
	float3 diffuse = light.color.rgb * diff * Diffuse.Sample(diff_sampler, texCoords);
	float3 specular = light.color.rgb * spec * Specular.Sample(spec_sampler, texCoords);


	return float3(ambient + diffuse + specular);
}

float3 CalcPointLight(Light light, float3 normal, float3 fragPos, float3 viewDir, float2 texCoords)
{
	float3 lightDir = normalize(light.position.xyz - fragPos);

	//diffuse
	float diff = max(dot(normal, lightDir),0.0);
	//specular
	float3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_shininess);

	float attenuation = Attenuation(light.position.xyz, fragPos, light.range, light.intensity);
	if(attenuation <= 0)
    	return float3(0.0,0.0,0.0);

	//combine results
	float3 ambient = light.color.rgb * float3(0.1,0.1,0.1) * Diffuse.Sample(diff_sampler, texCoords);
	float3 diffuse = light.color.rgb * diff * Diffuse.Sample(diff_sampler, texCoords);
	float3 specular = light.color.rgb * spec * Specular.Sample(spec_sampler, texCoords);


	ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;

	return float3(ambient + diffuse + specular);
}

float4 PShader(VIn input) : SV_TARGET
{
	float3 norm = normalize(input.p_normal);
	float3 viewDir = normalize(u_viewPosition - input.p_fragPos);
	float3 result = CalcDirLight(u_lights[0], norm, viewDir, input.p_texCoords);

	//for(int i = 1; i< NR_POINT_LIGHTS+1; i++)
		//result += CalcPointLight(u_lights[0], norm, input.p_fragPos, viewDir, input.p_texCoords);

	return float4(result, 1.0);
}
#END