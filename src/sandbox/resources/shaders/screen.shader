namespace vertex
{
    struct VIn
    {
        float2 position : POSITION;
        float2 texCoords : TEXCOORD;
    };

    struct VOut
    {
        float4 p_position : SV_POSITION;
        float2 p_texCoords : TEXCOORD; 
    };

    VOut main(VIn input)
    {
        VOut output;
        output.p_position = float4(input.position.x, -input.position.y,0.0,1.0);
        output.p_texCoords = input.texCoords;
        return output;
    }
}

namespace fragment
{
    #include "camera_utils.shinc"
    #include "light_utils.shinc"
    #include "texture_defines.shinc"
    struct PIn
    {
        float4 p_position : SV_POSITION;
        float2 p_texCoords : TEXCOORD;
    };

    Texture2D Depth : DepthTexture;
    SamplerState D_Sampler : DepthSampler;

    Texture2D Color : Color0Texture;
    SamplerState ColorSampler : rColor0Sampler;
    
    static const float offset = 1.0/300.0;

    static const float2 offsets[9] = 
    {
        float2(-offset,  offset), // top-left
        float2( 0.0f,    offset), // top-center
        float2( offset,  offset), // top-right
        float2(-offset,  0.0f),   // center-left
        float2( 0.0f,    0.0f),   // center-center
        float2( offset,  0.0f),   // center-right
        float2(-offset, -offset), // bottom-left
        float2( 0.0f,   -offset), // bottom-center
        float2( offset, -offset)  // bottom-right   

    };

    static const float narcotic[9] =
    {
        -1, -1, -1,
        -1,  9, -1,
        -1, -1, -1
    };

    float4 ApplyKernel(float kernel[9], float2 texCoord)
    {
        float3 sampleTex[9];
        int i = 0;
        for(i = 0; i < 9; i++)
        {
            sampleTex[i] = float3(Color.Sample(ColorSampler, texCoord.xy + offsets[i]).xyz);
        }

        float3 col = float3(0.0, 0.0, 0.0);
        for(i = 0; i < 9; i++)
            col += sampleTex[i] * kernel[i];
        
        return float4(col, 1.0);
    }

    float4 main(PIn input) : SV_TARGET
    {    
        //Depth
        //return Depth.Sample(D_Sampler, input.p_texCoords).xxxa;
        //Narcotic Effect
        //return ApplyKernel(narcotic, input.p_texCoords);
        //Normal Color
        return Color.Sample(ColorSampler, input.p_texCoords);
        //Invert Color
        //return float4(float3(1.0-Color.Sample(ColorSampler, input.p_texCoords).xyz),1.0);
    }
}