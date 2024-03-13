
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

		float3 tanViewPos : TANGENT1;
		float3 tanFragPos : TANGENT2;
		float3 tanLightDir : TANGENT3;
    };

    VOut main(VIn input)
    {
        VOut output;

        output.position = mul(mul(mul(u_projection, u_view), u_model), input.position);
        output.fragPos = mul(u_model, input.position).rgb;
		output.texCoords = input.texCoords;
        output.lightSpaceFragPos = mul(mul(u_dirLights[0].projection, u_dirLights[0].view), float4(output.fragPos, 1.0)); 
		output.tangent = normalize(mul((float3x3)inverse(u_model), input.tangent).rgb);
		output.normal = normalize(mul((float3x3)inverse(u_model), input.normal).rgb);
		float3 bitangent = normalize(cross(output.tangent, output.normal));

		float3x3 TBN = transpose(float3x3(output.tangent, bitangent, output.normal));

		output.tanViewPos = mul(TBN, u_viewPosition.xyz).xyz;
		output.tanFragPos = mul(TBN, output.fragPos.xyz).xyz;
		output.tanLightDir = mul(TBN, u_dirLights[0].direction.xyz).xyz;

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

		float3 tanViewPos : TANGENT1;
		float3 tanFragPos : TANGENT2;
		float3 tanLightDir : TANGENT3;
	};

	Texture2D Diffuse : Texture0;
	SamplerState DiffuseSampler : TexSampler0;

	Texture2D Specular : Texture1;
	SamplerState SpecularSampler : TexSampler1;

	Texture2D Normal : Texture2;
	SamplerState NormalSampler : TexSampler2;

	Texture2D Displacement : Texture3;
	SamplerState DispSampler : TexSampler3;

	Texture2D DepthMap : Texture4;
	SamplerState DepthMapSampler : TexSampler4;
	
    TextureCube DepthCube : Texture5;
    SamplerState DepthCubeSampler : TexSampler5;

	static float heightScale = 0.01;
	float2 ParallaxMapping(float2 texCoords, float3 viewDir)
	{
		const float minLayers = 8;
		const float maxLayers = 32;
		float numLayers = lerp(maxLayers, minLayers, abs(dot(float3(0.0,0.0,1.0),viewDir)));

		float layerDepth = 1.0 / numLayers;

		float currentLayerDepth = 0.00001;

		float2 P = (viewDir.xy/viewDir.z) * heightScale;
		float2 deltaUV = P / numLayers;

		float2 currentUV = texCoords;
		float currentHeight = Displacement.Sample(DispSampler, currentUV).r;

		[unroll(512)]
		while(currentLayerDepth < currentHeight)
		{
			currentUV -= deltaUV;
			currentHeight = Displacement.Sample(DispSampler, currentUV).r;
			currentLayerDepth += layerDepth;
		}

		float2 prevUV = currentUV + deltaUV;

		float afterDepth = currentHeight - currentLayerDepth;
		float beforeDepth = Displacement.Sample(DispSampler, prevUV).r - currentLayerDepth + layerDepth;

		float weight = afterDepth/ (afterDepth - beforeDepth);
		float2 finalUV = (prevUV * weight) + currentUV * (1.0 - weight);

		return finalUV;
	}

	float3 CalcBumpedNormal(float3 normal, float3 tangent, float2 texCoord)
	{
		tangent = normalize(tangent - (dot(tangent,normal) * normal));
		float3 bitangent = cross(tangent, normal);
		float3 bumpMapNormal = Normal.Sample(NormalSampler, texCoord).xyz;
		bumpMapNormal *= 2.0;
		bumpMapNormal -= float3(1.0,1.0,1.0);

		float3x3 TBN = float3x3(tangent, bitangent, normal);
		float3 newNormal = mul(TBN,bumpMapNormal).xyz;
		return normalize(newNormal);
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
		//This is the shadows position from the light
		float3 fragToLight = light.position.xyz - fragPos;

		float3 shadowUV = fragToLight;
		#ifdef DirectX
		shadowUV.y *= -1;
		#endif
		

		//This all below is whether something is in shadow or not
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
		float spec = pow(max(dot(normal, halfwayDir),0.0), u_shininess);

		//combine results
		float3 ambient = light.color.rgb * 0.1;
		float3 diffuse = light.color.rgb * diff;
		float3 specular = light.color.rgb * spec * Specular.Sample(SpecularSampler, texCoords).rgb;

		float shadow = DirLightShadowCalculation(lightFragPos, normal, lightDir);
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
		//float3 viewDir = normalize(u_viewPosition.xyz - input.fragPos);
		float3 viewDir = normalize(input.tanViewPos - input.tanFragPos);
		float2 texCoords = ParallaxMapping(input.texCoords, viewDir);
		if(texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0)
    		discard;

		float3 normal = CalcBumpedNormal(normalize(input.tangent), normalize(input.normal), texCoords);
		float3 lightDir = normalize(input.tanLightDir);//normalize(u_dirLights[0].direction.xyz);
		float3 result = CalcDirLight(lightDir, input.lightSpaceFragPos, normal, texCoords, viewDir);

		int i = 0;
		for(i = 0; i < lightCount; i++)
			result += CalcPointLight(u_pointLights[i], normal, texCoords, input.fragPos, viewDir);

		return float4(result, 1.0);
	}
}