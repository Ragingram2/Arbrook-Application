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
    #include "light_utils.shinc"
    struct PIn
    {
        float4 p_position : SV_POSITION;
        float2 p_texCoords : TEXCOORD;
    };
    
    const float offset = 1.0/300.0;

    const float2 offsets[9] = 
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

    const float narcotic[9] =
    {
        -1, -1, -1,
        -1,  9, -1,
        -1, -1, -1
    };

    float4 ApplyKernel(float kernel[9], float2 texCoord)
    {
        float3 sampleTex[9];
        for(int i = 0; i < 9; i++)
        {
            sampleTex[i] = float3(Texture0.Sample(TexSampler0, texCoord.xy + offsets[i]).xyz);
        }

        float3 col = float3(0.0);
        for(int i = 0; i < 9; i++)
            col += sampleTex[i] * kernel[i];
        
        return float4(col,1.0);
    }

    float4 main(PIn input) : SV_TARGET
    {    
        //return ApplyKernel(narcotic, input.p_texCoords);
        //Normal Color
        return Texture0.Sample(TexSampler0, input.p_texCoords);
        //Invert Color
        //return float4(float3(1.0-Texture0.Sample(TexSampler0, input.p_texCoords).xyz),1.0);
    }
}