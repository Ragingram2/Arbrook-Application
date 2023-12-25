namespace vertex
{
    #include "utils.shinc"
    #include "default.vert"
}//end

namespace fragment
{
    
struct PIn
{
	float4 p_position : SV_POSITION;
	float3 p_normal : NORMAL;
	float2 p_texCoords : TEXCOORD;

	float3 p_fragPos : TEXCOORD1;
};


float4 main(PIn input) : SV_TARGET
{
	return float4(normalize(input.p_normal), 1.0);
}
}//end