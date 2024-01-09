
namespace vertex
{
	#include "utils.shinc"	
    #include "default.vert"
}

namespace fragment
{
	#include "camera_utils.shinc"
	#include "light_utils.shinc"

	struct PIn
	{
		float4 p_position : SV_POSITION;
		float3 p_normal : NORMAL;
		float2 p_texCoords : TEXCOORD;

		float3 p_fragPos : TEXCOORD1;
	};


	float4 main(PIn input) : SV_TARGET
	{
		float3 normal = normalize(input.p_normal);
		float3 viewDir = normalize(u_viewPosition.xyz - input.p_fragPos);
		float3 result = CalcDirLight(u_lights[0], normal, input.p_texCoords, viewDir, DiffuseTex, SpecularTex);

		for(int i = 1; i< NR_POINT_LIGHTS+1; i++)
			result += CalcPointLight(u_lights[i], normal, input.p_texCoords, input.p_fragPos, viewDir, DiffuseTex, SpecularTex);

		return float4(result, 1.0);
	}
}