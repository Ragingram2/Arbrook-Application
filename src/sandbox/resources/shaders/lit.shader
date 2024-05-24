
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

	Texture2D Albedo : Texture2;
	SamplerState AlbedoSampler : TexSampler2;

	Texture2D Roughness : Texture3;
	SamplerState RoughnessSampler : TexSampler3;

	Texture2D Normal : Texture4;
	SamplerState NormalSampler : TexSampler4;

	Texture2D Height : Texture5;
	SamplerState HeightSampler : TexSampler5;

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
		const float maxLayers = 32;
		float numLayers = lerp(maxLayers, minLayers, abs(dot(float3(0.0,0.0,1.0),viewDir)));

		float layerDepth = 1.0 / numLayers;

		float currentLayerDepth = 0.0;
		float prevLayerDepth = 0.0;
		float2 P = (viewDir.xy/max(viewDir.z, 1.0)) * heightScale;
		float2 deltaUV = P / numLayers;
		deltaUV.x *= -1.0;

		float2 currentUV = texCoords- (deltaUV * numLayers);
		float2 prevUV = currentUV;
		float currentDepthMapValue = max(Height.Sample(HeightSampler, currentUV).r,EPSILON);
		float prevDepthMapValue = currentDepthMapValue;

		[unroll(64)]
		for(int i = 0; i < numLayers; i++)
		{
			if(currentLayerDepth >= currentDepthMapValue) break;

			prevUV = currentUV;
			currentUV -= deltaUV;

			prevDepthMapValue = currentDepthMapValue;
			currentDepthMapValue = max(Height.Sample(HeightSampler, currentUV).r,EPSILON);
			
			prevLayerDepth = currentLayerDepth;

			currentLayerDepth += layerDepth;
		}

		float afterDepth = currentDepthMapValue - currentLayerDepth;
		float beforeDepth = prevDepthMapValue - prevLayerDepth;
		float weight = afterDepth / (afterDepth - beforeDepth);
		float2 finalUV = lerp(currentUV, prevUV, weight);

		return finalUV;
	}

	float3 CalcBumpedNormal(float3x3 TBN, float3 tanNormal)
	{
		float3 normal = tanNormal;
		normal = ( 2.0 * normal - float3(1.0,1.0,1.0));
		normal.y *= -1;
		float3 newNormal = normalize(mul(normal,TBN).xyz);
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

	// float3 CalcDirLight(float3 diffuseColor, float3 specularColor, float3 normal, float metallicValue, float roughnessValue, float3 lightDir, float4 lightFragPos, float3 viewDir)
	// {
	// 	DirLight light = u_dirLights[0];

	// 	float3 radiance = light.color.rgb*5.0;
	// 	float3 diffuse = 1.0 - roughnessValue;

	// 	float3 _specularColor = specularColor;
	// 	_specularColor = lerp(_specularColor, diffuseColor, metallicValue);
	// 	diffuse *= 1.0 - metallicValue;

	// 	float3 halfwayDir = normalize(lightDir + viewDir);
	// 	float spec = pow(max(dot(normal, halfwayDir),0.0), (1.0 - roughnessValue.r) * 32.0);
	// 	float3 specular = _specularColor * spec;

	// 	float NdotL = max(dot(normal, lightDir), 0.0);
	// 	float shadow = DirLightShadowCalculation(lightFragPos, normal, lightDir);
	// 	return ((1.0 - shadow) * diffuse * diffuseColor + specular) * radiance * NdotL;
	// }

	float3 CalcPBRDirLight(float3 diffuseColor, float metallicValue, float roughnessValue, float3 normal, float3 ao, float3 lightDir, float4 lightFragPos, float3 viewDir)
	{
		DirLight light = u_dirLights[0];

		float NdotL = max(dot(normal, lightDir), 0.0);//how much diffuse light hits the eye/camera
		float3 halfwayDir = normalize(lightDir + viewDir);//halfway between the light direction and the view direction
		float NdotV = max(dot(normal, viewDir), 0.0);//
		float NdotH = max(dot(normal, halfwayDir), 0.0);//how much specular light hits the eye/camera
		float VdotH = max(dot(viewDir, halfwayDir), 0.0);
		

		float roughness = pow(roughnessValue, 4.0);//0: ideal smooth, 1: max roughness, usualy raised to the pow of 2 but disney said, nah 4's better
		float NDF = DistributionGGX(NdotH, roughness);
		float G = GeometrySmith(NdotV, NdotL, roughness);//Gv*Gl;

		float3 F0 = lerp(float3(0.04,0.04,0.04), diffuseColor, float3(metallicValue, metallicValue, metallicValue));
		float3 F = fresnelSchlick(VdotH, F0);

		float3 kS = F;
		float3 kD = float3(1.0,1.0,1.0) - kS;
		kD *= float3(1.0,1.0,1.0) - float3(metallicValue, metallicValue, metallicValue);

		float3 DiffuseBRDF = kD * diffuseColor/PI;
		
		float3 SpecularBRDF = (NDF*G*F) / (4.0 * NdotL * NdotV + 0.0001);
		float3 radiance = light.color.rgb * 255.0;

		s = SpecularBRDF;

		float3 outColor = (DiffuseBRDF * SpecularBRDF) * radiance * NdotL;

		float3 ambient = float3(0.03,0.03,0.03) * diffuseColor * ao;
		outColor += ambient;

		outColor = outColor / (outColor + float3(1.0,1.0,1.0));
		float v = 1.0/2.2;
		outColor = pow(abs(outColor), float3(v,v,v));

		return outColor;
	}

	float3 CalcPBRPointLight(float3 diffuseColor, float metallicValue, float roughnessValue, float3 normal, float3 ao, float3 fragPos, float3 viewDir)
	{
		
		float3 F0 = lerp(float3(0.04,0.04,0.04), diffuseColor, float3(metallicValue, metallicValue, metallicValue));
		float3 result = float3(0.0,0.0,0.0);

		for(int i = 0; i < lightCount; i++)
		{
			float3 lightDir = normalize(u_pointLights[i].position.xyz - fragPos);
			float3 halfwayDir = normalize(lightDir + viewDir);//halfway between the light direction and the view direction
			float NdotL = max(dot(normal, lightDir), 0.0);//how much diffuse light hits the eye/camera
			float NdotV = max(dot(normal, viewDir), 0.0);//
			float NdotH = max(dot(normal, halfwayDir), 0.0);//how much specular light hits the eye/camera
			float VdotH = max(dot(viewDir, halfwayDir), 0.0);

			float attenuation = Attenuation(u_pointLights[i].position.xyz, fragPos, u_pointLights[i].range, u_pointLights[i].intensity);
			if(attenuation <= 0)
				continue;
			float3 radiance = u_pointLights[i].color.rgb * attenuation * 255.0;

			float roughness = pow(roughnessValue, 4.0);
			float NDF = DistributionGGX(NdotH, roughness);
			float G = GeometrySmith(NdotV, NdotL, roughness);//Gv*Gl;

			float3 F = fresnelSchlick(VdotH, F0);

			float3 kS = F;
			float3 kD = float3(1.0,1.0,1.0) - kS;
			kD *= float3(1.0,1.0,1.0) - float3(metallicValue, metallicValue, metallicValue);

			float3 DiffuseBRDF = kD * diffuseColor/PI;
			
			float3 SpecularBRDF = (NDF*G*F) / (4.0 * NdotL * NdotV + 0.0001);

			result += (DiffuseBRDF * SpecularBRDF) * radiance * NdotL;
		}

		float3 ambient = float3(0.03,0.03,0.03) * diffuseColor * ao;
		float3 color = ambient + result;

		color = color / (color + float3(1.0,1.0,1.0));
		float v = 1.0/2.2;
		color = pow(abs(color), float3(v,v,v));

		return color;
	}


	// float3 CalcPointLight(PointLight light, float3 diffuseColor, float3 specularColor, float3 normal, float metallicValue, float roughnessValue, float3 fragPos, float3 viewDir)
	// {
	// 	float attenuation = Attenuation(light.position.xyz, fragPos, light.range, light.intensity);
	// 	if(attenuation <= 0)
	// 		return float3(0.0,0.0,0.0);

	// 	float3 lightDir = normalize(light.position.xyz - fragPos);

	// 	float3 radiance = light.color.rgb * attenuation;
	// 	float3 diffuse = 1.0 - roughnessValue;

	// 	float3 _specularColor = specularColor;
	// 	_specularColor = lerp(_specularColor, diffuseColor, metallicValue);
	// 	diffuse *= 1.0 - metallicValue;
		

	// 	float3 halfwayDir = normalize(lightDir + viewDir);
	// 	float spec = pow(max(dot(normal, halfwayDir),0.0), (1.0 - roughnessValue.r) * 32.0);
	// 	float3 specular = _specularColor * spec;

	// 	float NdotL = max(dot(normal, lightDir), 0.0);
	// 	float shadow = PointLightShadowCalculation(light, fragPos);
	// 	return ((1.0 - shadow) * diffuse * diffuseColor + specular) * radiance * NdotL;
	// }

	float4 main(PIn input) : SV_TARGET
	{
		float2 uvs = input.texCoords;
		uvs.y *= -1.0;

		float4 albedo = hasAlbedo ? Albedo.Sample(AlbedoSampler, uvs).rgba : float4(diffuseColor.rgb, 1.0);
		if(albedo.a < .1)
		{
			discard;
			return albedo;
		}

		// float3 metallic = hasMetallic ? Metallic.Sample(MetallicSampler, uvs).rrr : float3(0.0,0.0,0.0);
		// float3 roughness = hasRoughness ? Roughness.Sample(RoughnessSampler, uvs).rrr : float3(0.0,0.0,0.0);
		// float3 ambientOcclusion  = hasAmbientOcclusion ? AmbientOcclusion.Sample(AmbientOcclusionSampler, uvs).rrr : float3(0.0,0.0,0.0);

		float3 ambientOcclusion  = hasMetallic ? Metallic.Sample(MetallicSampler, uvs).rrr : float3(1.0,1.0,1.0);
		float3 roughness = hasMetallic ? Metallic.Sample(MetallicSampler, uvs).ggg : float3(1.0,0.0,0.0);
		float3 metallic = hasMetallic ? Metallic.Sample(MetallicSampler, uvs).ggg : float3(1.0,0.0,0.0);
		s = roughness;

		float3 tangent = normalize(input.tangent - dot(input.tangent, input.normal) * input.normal);
		float3 bitangent = cross(input.normal, tangent);

		float3x3 TBN = float3x3(tangent, bitangent, input.normal);
		float3x3 invTBN = transpose(TBN);
		float3 viewDir = normalize(u_viewPosition.xyz - input.fragPos);
		//well thats a fun one, this crashed stuff because hwen you transpose a matrix like this, you also need to change the order of multiplication
		float3 tanViewDir = mul(viewDir, invTBN);

		float2 tanTexCoords = hasHeight ? ParallaxMapping(input.texCoords, tanViewDir) : input.texCoords;
		float3 tanNormal = hasNormal ? Normal.Sample(NormalSampler, tanTexCoords).xyz : input.normal;

		float3 normal = (hasHeight) ? CalcBumpedNormal(TBN, tanNormal) : normalize(input.normal);
		float3 lightDir = normalize(u_dirLights[0].direction.xyz);
		float3 result = CalcPBRDirLight(albedo.rgb, metallic.r, roughness.r, input.normal, ambientOcclusion, lightDir, input.lightSpaceFragPos, viewDir);
		result += CalcPBRPointLight(albedo.rgb, metallic.r, roughness.r, input.normal, ambientOcclusion, input.fragPos, viewDir);

		return float4(result, 1.0);
	}
}