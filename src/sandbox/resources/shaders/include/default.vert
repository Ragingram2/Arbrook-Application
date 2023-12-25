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
    float3 p_normal : NORMAL;
    float2 p_texCoords : TEXCOORD;


    float3 p_fragPos : TEXCOORD1;
};

VOut main(VIn input)
{
    VOut output;

    output.p_position = mul(mul(mul(u_projection, u_view), u_model), input.position);
    output.p_fragPos = mul(u_model, input.position).rgb;
    output.p_normal = normalize(mul(input.normal, (float3x3)inverse(u_model)));
    //output.p_normal = normalize(input.normal);
    output.p_texCoords = input.texCoords;

    return output;
}