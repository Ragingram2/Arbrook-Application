
namespace vertex
{
    #include "camera_utils.shinc"
	#include "light_utils.shinc"
    struct VIn
    {
        float4 position : POSITION;
        float3 normal : NORMAL;
        float2 texCoords : TEXCOORD;
		float3 tangent : TANGENT;
    };

    struct VOut
    {
        float4 position : SV_POSITION;
        float3 normal : NORMAL;
        float2 texCoords : TEXCOORD;

        float3 fragPos : TEXCOORD1;
        float4 lightSpaceFragPos : TEXCOORD2;
		float3 tangent : TANGENT;
    };

    VOut main(VIn input)
    {
        VOut output;

        output.position = mul(mul(mul(u_projection, u_view), u_model), input.position);
        output.fragPos = mul(u_model, input.position).rgb;
		output.texCoords = input.texCoords;
        output.lightSpaceFragPos = mul(mul(u_dirLights[0].projection, u_dirLights[0].view), float4(output.fragPos, 1.0)); 

		float3x3 normalMatrix = transpose((float3x3)inverse(u_model));
		output.tangent = normalize(mul(normalMatrix, input.tangent).rgb);
		output.normal = normalize(mul(normalMatrix, input.normal).rgb);

        return output;
    }
}

namespace fragment
{
	#pragma warning( disable : 4121)
	#include "camera_utils.shinc"
	#include "light_utils.shinc"
	#include "texture_defines.shinc"
	
	struct PIn
	{
		float4 position : SV_POSITION;
		float3 normal : NORMAL;
		float2 texCoords : TEXCOORD;

		float3 fragPos : TEXCOORD1;
		float4 lightSpaceFragPos : TEXCOORD2;
		float3 tangent : TANGENT;
	};

	Texture2D DepthMap : Texture0;
	SamplerState DepthMapSampler : TexSampler0;
	
    TextureCube DepthCube : Texture1;
    SamplerState DepthCubeSampler : TexSampler1;

	Texture2D Diffuse : Texture2;
	SamplerState DiffuseSampler : TexSampler2;

	Texture2D Specular : Texture3;
	SamplerState SpecularSampler : TexSampler3;

	Texture2D Normal : Texture4;
	SamplerState NormalSampler : TexSampler4;

	Texture2D Displacement : Texture5;
	SamplerState DispSampler : TexSampler5;

	Texture2D Metallic : Texture6;
	SamplerState MetallicSampler : TexSampler6;

	Texture2D AmbientOcclusion: Texture7;
	SamplerState AmbientOcclusionSampler : TexSampler7;

	Texture2D Emissive : Texture8;
	SamplerState EmissiveSampler : TexSampler8;

	static float heightScale = 0.1;
	static float3 s = float3(0,0,0);
	float2 ParallaxMapping(float2 texCoords, float3 viewDir)
	{
		const float minLayers = 4;
		const float maxLayers = 128;
		float numLayers = lerp(maxLayers, minLayers, abs(dot(float3(0.0,0.0,1.0),viewDir)));

		float layerDepth = 1.0 / numLayers;

		float currentLayerDepth = 0.0;
		float prevLayerDepth = 0.0;
		float2 P = (viewDir.xy/max(viewDir.z, 1.0)) * heightScale;
		float2 deltaUV = P / numLayers;
		deltaUV.x *= -1.0;

		float2 currentUV = texCoords - (deltaUV * numLayers);
		float2 prevUV = currentUV;
		float currentDepthMapValue = max(Displacement.Sample(DispSampler, currentUV).r,EPSILON);
		float prevDepthMapValue = currentDepthMapValue;

		[unroll(32)]
		for(int i = 0; i < numLayers; i++)
		{
			if(currentLayerDepth >= currentDepthMapValue) break;

			prevUV = currentUV;
			currentUV -= deltaUV;

			prevDepthMapValue = currentDepthMapValue;
			currentDepthMapValue = max(Displacement.Sample(DispSampler, currentUV).r,EPSILON);
			
			prevLayerDepth = currentLayerDepth;

			currentLayerDepth += layerDepth;
		}

		float afterDepth = currentDepthMapValue - currentLayerDepth;
		float beforeDepth = prevDepthMapValue - prevLayerDepth;
		float weight = afterDepth / (afterDepth - beforeDepth);
		float2 finalUV = lerp(currentUV, prevUV, weight);

		return finalUV;
	}

	float3 CalcBumpedNormal(float3x3 TBN, float2 texCoord)
	{
		float3 normal = Normal.Sample(NormalSampler, texCoord).xyz;
		normal = ((normal * 2.0) - float3(1.0));
		float3 newNormal = normalize(mul(TBN, normal).xyz);
		return newNormal;
	}

	float DirLightShadowCalculation(float4 lightFragPos, float3 normal, float3 lightDir)
    {
		float3 projCoords = lightFragPos.xyz/lightFragPos.w;
		#ifdef DirectX
		projCoords.x = (projCoords.x * 0.5) + 0.5;
		projCoords.y = (projCoords.y * -0.5) + 0.5;
		#else
        projCoords = (projCoords * 0.5) + 0.5;
		#endif

        float closestDepth = DepthMap.Sample(DepthMapSampler, projCoords.xy).r;
        float currentDepth = projCoords.z;

		float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);  
		float shadow = 0.0;
		if((currentDepth - bias) > closestDepth && currentDepth < 1.0)
		{
			shadow = 1.0;
		}

        return shadow;
    }

	float PointLightShadowCalculation(PointLight light, float3 fragPos)
    {
		float3 fragToLight = light.position.xyz - fragPos;

		float3 shadowUV = fragToLight;
		#ifdef DirectX
		shadowUV.y *= -1;
		#endif
		
		float closestDepth = DepthCube.Sample(DepthCubeSampler, shadowUV).r * light.farPlane;
		float currentDepth = length(fragToLight);

		float bias = 0.05f;
		float shadow = 0.0;
		if((currentDepth - bias) > closestDepth)
		{
			shadow = 1.0;
		}

        return shadow;
    }

	float3 CalcDirLight(float3 lightDir, float4 lightFragPos, float3 normal, float2 texCoords, float3 viewDir)
	{
		DirLight light = u_dirLights[0];
		//diffuse
		float diff = max(dot(normal, lightDir),0.0);
		//specular
		float3 halfwayDir = normalize(lightDir + viewDir);
		float spec = pow(max(dot(normal, halfwayDir),0.0), 32.0);

		//combine results
		float3 ambient = light.color.rgb * 0.1;
		float3 diffuse = light.color.rgb * diff;
		float3 specular = light.color.rgb * spec * (hasSpecular.x ? Specular.Sample(SpecularSampler, texCoords).rgb : float3(0.0));

		float shadow = DirLightShadowCalculation(lightFragPos, normal, lightDir);
		return (ambient + (1.0 - shadow) * (diffuse + specular)) * (hasDiffuse.x ? Diffuse.Sample(DiffuseSampler, texCoords).rgb : diffuseColor);
	}

	float3 CalcPointLight(PointLight light, float3 normal, float2 texCoords, float3 fragPos, float3 viewDir)
	{
		float attenuation = Attenuation(light.position.xyz, fragPos, light.range, light.intensity);
		if(attenuation <= 0)
			return float3(0.00);

		float3 metallic = hasMetallic ? Metallic.Sample(MetallicSampler, texCoords).rrr : float3(1.0);
		float3 albedo = hasDiffuse.x ? Diffuse.Sample(DiffuseSampler, texCoords).rgb : diffuseColor;
		float3 roughness = hasSpecular.x ? Specular.Sample(SpecularSampler, texCoords).rrr : float3(0.0);

		float3 lightDir = normalize(light.position.xyz - fragPos);

		float3 radiance = light.color.rgb * attenuation;
		float3 diffuse = (float3(1.0) - roughness);

		float3 specularColor = float3(0.04);
		specularColor = lerp(specularColor, albedo, metallic);
		diffuse *= float3(1.0) - metallic;
		

		float3 halfwayDir = normalize(lightDir + viewDir);
		float spec = pow(max(dot(normal, halfwayDir),0.0), (1.0 - roughness.r) * 32.0);
		float3 specular = specularColor * spec;

		float NdotL = max(dot(normal, lightDir), 0.0);
		float shadow = PointLightShadowCalculation(light, fragPos);
		return ((1.0 - shadow)*diffuse * albedo + specular) * radiance * NdotL;
		//return (ambient + (1.0 - shadow) * (diffuse + specular)) * (hasDiffuse.x ? Diffuse.Sample(DiffuseSampler, texCoords).rgb : diffuseColor);
	}

	float4 main(PIn input) : SV_TARGET
	{
		float4 albedo = hasDiffuse.x ? Diffuse.Sample(DiffuseSampler, input.texCoords).rgba : float4(diffuseColor.rgb,1.0);
		float3 tangent = normalize(input.tangent - dot(input.tangent, input.normal) * input.normal);
		float3 bitangent = cross(input.normal, tangent);

		float3x3 TBN = float3x3(tangent, bitangent, input.normal);

		float3 tanViewDir = mul(TBN, normalize(u_viewPosition.xyz - input.fragPos));
		float3 viewDir = normalize(u_viewPosition.xyz - input.fragPos);
		
		float2 tanTexCoords = hasHeight ? ParallaxMapping(input.texCoords, tanViewDir) : input.texCoords;
		//float2 tanTexCoords = ParallaxMapping(input.texCoords, tanViewDir);
		if(hasHeight && (tanTexCoords.x > 1.0 || tanTexCoords.y > 1.0 || tanTexCoords.x < 0.0 || tanTexCoords.y < 0.0))
    		discard;

		float3 normal = (hasHeight) ? CalcBumpedNormal(TBN, tanTexCoords) : normalize(input.normal);
		float3 lightDir = normalize(u_dirLights[0].direction.xyz);
		float3 result = CalcDirLight(lightDir, input.lightSpaceFragPos, normal, input.texCoords, viewDir);

		for(int i = 0; i < lightCount; i++)
			result += CalcPointLight(u_pointLights[i], normal, input.texCoords, input.fragPos, viewDir);
		if(albedo.a < .8)
			discard;

		return float4(result, 1.0);
	}
}