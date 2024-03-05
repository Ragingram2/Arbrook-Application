namespace vertex
{
    // #include "camera_utils.shinc"
    struct VIn
    {
        float2 position : POSITION;
        float2 texCoords : TEXCOORD;
    };

    struct VOut
    {
        float4 position : SV_POSITION;
        float2 texCoords : TEXCOORD; 
    };

    VOut main(VIn input)
    {
        VOut output;
        #ifdef DirectX
            output.position = float4(input.position.x, input.position.y, 0.0, 1.0);
        #else
            output.position = float4(input.position.x, -input.position.y, 0.0, 1.0);
        #endif
        output.texCoords = input.texCoords;
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
        float4 position : SV_POSITION;
        float2 texCoords : TEXCOORD;
    };

    Texture2D Color : Texture0;
    SamplerState ColorSampler : TexSampler0;
    
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
        //return Depth.Sample(D_Sampler, input.texCoords);
        //Narcotic Effect
        //return ApplyKernel(narcotic, input.texCoords);
        //Color
        return Color.Sample(ColorSampler, input.texCoords);
        //Invert Color
        //return float4(float3(1.0-Color.Sample(ColorSampler, input.texCoords).xyz),1.0);
    }
}