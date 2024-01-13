namespace vertex
{
    #include "utils.shinc"
	#include "camera_utils.shinc"
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

		output.p_position = mul(mul(mul(u_projection, u_view), u_model), input.position);

		return output;
	}
}

namespace fragment
{
    
	struct PIn
	{
		float4 p_position : SV_POSITION;
	};


	float4 main(PIn input) : SV_TARGET
	{
		return float4(0.0, 1.0, 0.0, 1.0);
	}
}