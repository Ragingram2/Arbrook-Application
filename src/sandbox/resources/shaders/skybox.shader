namespace vertex
{
    #include "camera_utils.shinc"
    struct VIn
    {
        float4 position : POSITION;
        float2 texCoords : TEXCOORD;
    };

    struct VOut
    {
        float4 position : SV_POSITION;
        float2 texCoords : TEXCOORD;

        float3 fragPos : TEXCOORD1;
    };

    VOut main(VIn input)
    {
        VOut output;
        output.fragPos = input.position.xyz;
        #ifdef DirectX
        output.fragPos.y *= -1.0;
        #endif
        output.position = mul(mul(u_projection, u_view), float4((u_viewPosition + input.position).rgb, 1.0)).xyww;
        output.texCoords = input.texCoords;
 

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
		float4 position : SV_POSITION;
		float2 texCoords : TEXCOORD;

        float3 fragPos : TEXCOORD1;
	};

    float4 main(PIn input) : SV_TARGET
    {

        return Skybox.SampleLevel(SkyboxSampler, SkyboxUV(normalize(input.fragPos)), 0);
    }
}