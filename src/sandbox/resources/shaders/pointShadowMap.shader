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
        float4 position : SV_POSITION;
    };

    VOut main(VIn input)
    {
        VOut output;

        output.position = mul(u_model, input.position);

        return output;
    }
}

namespace geometry
{
    #include "light_utils.shinc"

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
                output.position = mul(mul(u_pointLights[lightIndex].shadowProjection, u_pointLights[lightIndex].shadowTransforms[face]), output.frag_pos);
                outputStream.Append(output);
            }
            outputStream.RestartStrip();
        }
    }
}

namespace fragment
{
    #include "light_utils.shinc"

	struct PIn
	{
		float4 position : SV_POSITION;
        float4 frag_pos : TEXCOORD1;
	};

	float main(PIn input) : SV_DEPTH
	{
        float3 lightPos = u_pointLights[lightIndex].position.xyz;
        float3 fragPos = input.frag_pos.xyz;
        float lightDistance = length(fragPos - lightPos);

        lightDistance = lightDistance / u_pointLights[lightIndex].farPlane;

        return lightDistance;
	}
}