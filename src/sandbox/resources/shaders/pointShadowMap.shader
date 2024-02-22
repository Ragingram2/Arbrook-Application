namespace vertex
{
    #include "camera_utils.shinc"
    #include "light_utils.shinc"

    struct VIn
    {
        float4 position : POSITION;
    };

    struct VOut
    {
        float4 p_position : SV_POSITION;
    };

    VOut main(VIn input)
    {
        VOut output;

        output.p_position = mul(input.position, u_model);

        return output;
    }
}

namespace geometry
{
    #include "light_utils.shinc"

    cbuffer LightInfo : register(b3)
    {
        int lightIndex;
        int lightCount;
    };

    struct GSIn
    {
        float4 position : SV_POSITION;
    };

    struct GSOut
    {
        float4 position : SV_POSITION;
        float4 frag_pos : TEXCOORD1;
        uint face : SV_RenderTargetArrayIndex;
    };


    [maxvertexcount(18)]
    void main(triangle GSIn input[3], inout TriangleStream<GSOut> outputStream)
    {
        GSOut output;

        int face = 0;
        for(face = 0; face < 6; face++)
        {
            output.face = face;
            for(int i = 0; i < 3; i++)
            {
                output.frag_pos = input[i].position;
                output.position = mul(output.frag_pos, mul(u_pointLights[lightIndex].shadowTransforms[face], u_pointLights[lightIndex].shadowProjection));
                outputStream.Append(output);
            }
            outputStream.RestartStrip();
        }
    }
}

namespace fragment
{
    #include "light_utils.shinc"

    cbuffer LightInfo : register(b3)
    {
        int lightIndex;
        int lightCount;
    };

	struct PIn
	{
		float4 position : SV_POSITION;
        float4 frag_pos : TEXCOORD1;
        //uint face : SV_RenderTargetArrayIndex;
	};

	float main(PIn input) : SV_DEPTH
	{
        float3 lightPos = float3(u_pointLights[lightIndex].position.xyz);
        float3 fragPos = float3(input.frag_pos.xyz);
        float3 diff = fragPos - lightPos;
        float lightDistance = length(diff);

        lightDistance = lightDistance / u_pointLights[lightIndex].farPlane;

        return lightDistance;
	}
}