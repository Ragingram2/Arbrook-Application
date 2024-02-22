
namespace vertex
{
    #include "camera_utils.shinc"
	#include "light_utils.shinc"
    struct VIn
    {
        float4 position : POSITION;
        float3 normal : NORMAL;
        float2 texCoords : TEXCOORD;
    };

    struct VOut
    {
        float4 p_position : SV_POSITION;
        float3 p_normal : NORMAL;
        float2 p_texCoords : TEXCOORD;


        float3 p_fragPos : TEXCOORD1;
        float4 p_lightSpaceFragPos : TEXCOORD2;
    };

    VOut main(VIn input)
    {
        VOut output;

        output.p_position = mul(input.position,mul(u_model , mul(u_view , u_projection)));
        output.p_fragPos = mul(input.position ,u_model).rgb;
        output.p_normal = normalize(mul(input.normal ,(float3x3)inverse(u_model)).rgb);
        output.p_texCoords = input.texCoords;
        output.p_lightSpaceFragPos = mul(float4(output.p_fragPos,1.0) , mul(u_dirLights[0].view ,u_dirLights[0].projection)); 

        return output;
    }
}

namespace fragment
{
	#include "camera_utils.shinc"
	#include "light_utils.shinc"
	#include "texture_defines.shinc"

	cbuffer LightInfo : register(b3)
    {
        int lightIndex;
        int lightCount;
    }

	struct PIn
	{
		float4 p_position : SV_POSITION;
		float3 p_normal : NORMAL;
		float2 p_texCoords : TEXCOORD;

		float3 p_fragPos : TEXCOORD1;
		float4 p_lightSpaceFragPos : TEXCOORD2;
	};

	Texture2D DepthMap : Texture1;
	SamplerState DepthMapSampler : TexSampler1;
	
    TextureCube DepthCube : Texture2;
    SamplerState DepthCubeSampler : TexSampler2;

	Texture2D Diffuse : Texture3;
	SamplerState DiffuseSampler : TexSampler3;

	Texture2D Specular : Texture4;
	SamplerState SpecularSampler : TexSampler4;

	float DirLightShadowCalculation(float4 fragPos, float3 normal, float3 lightDir)
    {
		float3 projCoords = fragPos.xyz/fragPos.w;
        projCoords = projCoords * 0.5 + 0.5;
        float closestDepth = DepthMap.Sample(DepthMapSampler, projCoords.xy).r;
        float currentDepth = projCoords.z;

		float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);  
        float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

		if(projCoords.z > 1.0)
        	shadow = 0.0;
			
        return shadow;
    }

	float PointLightShadowCalculation(PointLight light, float3 fragPos)
    {
		fragPos.y *= -1.0;
		//This is the shadows position from the light
		float3 fragToLight = fragPos - light.position.xyz;

		//This all below is whether something is in shadow or not
		float closestDepth = DepthCube.Sample(DepthCubeSampler, fragToLight).r;
		closestDepth *= light.farPlane;

		float currentDepth = length(fragToLight);
		float bias = 0.05;
		float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

        return shadow;
    }

	float3 CalcDirLight(float4 fragPos, float3 normal, float2 texCoords, float3 viewDir)
	{
		DirLight light = u_dirLights[0];
		float3 lightDir = normalize(light.direction.xyz);
		//diffuse
		float diff = max(dot(normal, lightDir),0.0);
		//specular
		float3 halfwayDir = normalize(lightDir + viewDir);
		float spec = pow(max(dot(normal, halfwayDir),0.0), u_shininess);

		//combine results
		float3 ambient = light.color.rgb * 0.1;
		float3 diffuse = light.color.rgb * diff;
		float3 specular = light.color.rgb * spec * Specular.Sample(SpecularSampler, texCoords).rgb;

		float shadow = DirLightShadowCalculation(fragPos, normal, lightDir);
		return (ambient + (1.0 - shadow) * (diffuse + specular)) * Diffuse.Sample(DiffuseSampler, texCoords).rgb;
	}

	float3 CalcPointLight(PointLight light, float3 normal, float2 texCoords, float3 fragPos, float3 viewDir)
	{
		float3 lightDir = normalize(light.position.xyz - fragPos);

		//diffuse
		float diff = max(dot(normal, lightDir),0.0);
		//specular
		float3 halfwayDir = normalize(lightDir + viewDir);
		float spec = pow(max(dot(normal, halfwayDir),0.0), u_shininess);

		float attenuation = Attenuation(light.position.xyz, fragPos, light.range, light.intensity);
		if(attenuation <= 0)
			return float3(0.0,0.0,0.0);

		//combine results
		float3 ambient = light.color.rgb * 0.1;
		float3 diffuse = light.color.rgb * diff;
		float3 specular = light.color.rgb * spec * Specular.Sample(SpecularSampler, texCoords).rgb;

		ambient  *= attenuation;
		diffuse  *= attenuation;
		specular *= attenuation;

		float shadow = PointLightShadowCalculation(light, fragPos);
		return (ambient + (1.0 - shadow) * (diffuse + specular)) * Diffuse.Sample(DiffuseSampler, texCoords).rgb;
	}

	float4 main(PIn input) : SV_TARGET
	{
		float3 normal = normalize(input.p_normal);
		float3 viewDir = normalize(u_viewPosition.xyz - input.p_fragPos);
		float3 result = CalcDirLight(input.p_lightSpaceFragPos, normal, input.p_texCoords, viewDir);

		int i = 0;
		for(i = 0; i < lightCount; i++)
			result += CalcPointLight(u_pointLights[i], normal, input.p_texCoords, input.p_fragPos, viewDir);
		
		return float4(result, 1.0);
	}
}