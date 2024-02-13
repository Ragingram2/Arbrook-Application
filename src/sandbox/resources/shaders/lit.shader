
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

        output.p_position = input.position * (u_model * (u_view * u_projection));//mul(mul(mul(u_projection, u_view), u_model), input.position);
        output.p_fragPos = (input.position *u_model).rgb;
        output.p_normal = normalize((input.normal * (float3x3)inverse(u_model)).rgb);
        output.p_texCoords = input.texCoords;
        output.p_lightSpaceFragPos = float4(output.p_fragPos,1.0) * (u_lights[0].view * u_lights[0].projection); 

        return output;
    }
}

namespace fragment
{
	#include "camera_utils.shinc"
	#include "light_utils.shinc"
	#include "texture_defines.shinc"

	struct PIn
	{
		float4 p_position : SV_POSITION;
		float3 p_normal : NORMAL;
		float2 p_texCoords : TEXCOORD;

		float3 p_fragPos : TEXCOORD1;
		float4 p_lightSpaceFragPos : TEXCOORD2;
	};

    Texture2D Color : Color0Texture;
    SamplerState ColorSampler : Color0Sampler;
    Texture2D DepthMap : DepthTexture;
    SamplerState DepthTexSampler : DepthSampler;

	Texture2D Diffuse : Texture0;
	SamplerState DiffuseSampler : TextureSampler0;
	Texture2D Specular : Texture1;
	SamplerState SpecularSampler : TextureSampler1;

	float ShadowCalculation(float4 fragPosLightSpace, float3 normal, float3 lightDir)
    {
        float3 projCoords = fragPosLightSpace.xyz/fragPosLightSpace.w;
        projCoords = projCoords * 0.5 + 0.5;
        float closestDepth = DepthMap.Sample(DepthTexSampler, projCoords.xy).r;
        float currentDepth = projCoords.z;

		float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);  
        float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

		if(projCoords.z > 1.0)
        	shadow = 0.0;
			
        return shadow;
    }

	float3 CalcDirLight(Light light, float4 lightSpaceFragPos, float3 normal, float2 texCoords, float3 viewDir)
	{
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

		float shadow = ShadowCalculation(lightSpaceFragPos, normal, lightDir);
		return (ambient + (1.0 - shadow) * (diffuse + specular)) * Diffuse.Sample(DiffuseSampler, texCoords).rgb;
	}

	float3 CalcPointLight(Light light, float3 normal, float2 texCoords, float3 fragPos, float3 viewDir)
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

		return (ambient + diffuse + specular)* Diffuse.Sample(DiffuseSampler, texCoords).rgb;
	}

	float4 main(PIn input) : SV_TARGET
	{
		float3 normal = normalize(input.p_normal);
		float3 viewDir = normalize(u_viewPosition.xyz - input.p_fragPos);
		float3 result = CalcDirLight(u_lights[0], input.p_lightSpaceFragPos, normal, input.p_texCoords, viewDir);

		for(int i = 1; i< NR_POINT_LIGHTS+1; i++)
			result += CalcPointLight(u_lights[i], normal, input.p_texCoords, input.p_fragPos, viewDir);
		
		return float4(result, 1.0);
	}
}