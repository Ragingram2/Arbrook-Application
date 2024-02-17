namespace vertex
{
    #include "camera_utils.shinc"
    struct VIn
    {
        float4 position : POSITION;
        float3 normal : NORMAL;
        float2 texCoords : TEXCOORD;
    };

    struct VOut
    {
        float4 p_position : SV_POSITION;
        float2 p_texCoords : TEXCOORD;

        float3 p_fragPos : TEXCOORD1;
    };

    VOut main(VIn input)
    {
        VOut output;
        output.p_fragPos = input.position.xyz;
        output.p_position = (float4((u_viewPosition + input.position).rgb, 1.0) * (u_view * u_projection)).xyww;
        output.p_texCoords = input.texCoords;

        return output;
    }
}

namespace fragment
{	
	#include "light_utils.shinc"
    #include "camera_utils.shinc"


    Texture2D Skybox : register(t0);
    SamplerState SkyboxSampler : register(s0);

	struct PIn
	{
		float4 p_position : SV_POSITION;
		float2 p_texCoords : TEXCOORD;

        float3 p_fragPos : TEXCOORD1;
	};

    float4 main(PIn input) : SV_TARGET
    {

        return Skybox.SampleLevel(SkyboxSampler, SkyboxUV(normalize(input.p_fragPos)), 0);
    }
}