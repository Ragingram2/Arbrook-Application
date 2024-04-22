
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
		output.normal = input.normal;//normalize(mul(normalMatrix, input.normal).rgb);

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

	static float heightScale = 0.1;
	static float3 s = float3(0,0,0);
	float2 ParallaxMapping(float2 texCoords, float3 viewDir)
	{
		const float minLayers = 8;
		const float maxLayers = 128;
		float numLayers = lerp(maxLayers, minLayers, abs(dot(float3(0.0,0.0,1.0),viewDir)));

		float layerDepth = 1.0 / numLayers;

		float currentLayerDepth = 0.0;
		float prevLayerDepth = 0.0;
		float2 P = (viewDir.xy/max(viewDir.z, 1.0))* heightScale;
		float2 deltaUV = P / numLayers;

		float2 currentUV = texCoords;
		float2 prevUV = currentUV;
		float currentDepthMapValue = max(Displacement.Sample(DispSampler, currentUV).r,EPSILON);
		float prevDepthMapValue = currentDepthMapValue;

		[unroll(1024)]
		while(currentLayerDepth < currentDepthMapValue )
		{
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
		normal = ((normal * 2.0) - 1.0);
		normal.y *= -1.0;

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
		float3 tangent = normalize(input.tangent - dot(input.tangent, input.normal) * input.normal);
		float3 bitangent = cross(input.normal, tangent);

		float3x3 TBN = transpose(float3x3(tangent, bitangent, input.normal));

		float4 tanViewPos = normalize(mul(TBN, u_viewPosition.xyz)).xyzz;
		float4 tanFragPos = normalize(mul(TBN, input.fragPos)).xyzz;

		float3 viewDir = mul(TBN, normalize(u_viewPosition.xyz - input.fragPos));
		
		float2 texCoords = input.texCoords;//ParallaxMapping(input.texCoords, viewDir);
		// if(texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0)
    	// 	discard;

		float3 normal = input.normal;//CalcBumpedNormal(TBN, texCoords);
		float3 lightDir = normalize(u_dirLights[0].direction.xyz);
		float3 result = CalcDirLight(lightDir, input.lightSpaceFragPos, normal, texCoords, viewDir);

		int i = 0;
		for(i = 0; i < lightCount; i++)
			result += CalcPointLight(u_pointLights[i], normal, texCoords, input.fragPos, viewDir);

		return float4(result, 1.0);
	}
}