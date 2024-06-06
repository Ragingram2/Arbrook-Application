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
    #define MATERIAL_INPUT
	#include "light_utils.shinc"

	
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

	static float3 s = float3(0,0,0);

	float4 main(PIn input) : SV_TARGET
	{
        
        float3 tangent = normalize(input.tangent - dot(input.tangent , input.normal) * input.normal);

        Material material = ExtractMaterial(input.texCoords, input.normal, tangent);
        
        if(material.texCoords.x < 0 || material.texCoords.x > 1 || material.texCoords.y < 0 || material.texCoords.y > 1)
            discard;

        float3 worldPos = (input.normal * material.height) + input.position;

        float3 result = GetAllLighting(material, worldPos);

		return float4(result, material.albedo.a);
	}
}