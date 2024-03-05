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
    	float4 position : SV_POSITION;
	};

	VOut main(VIn input)
	{
		VOut output;

		output.position = mul(mul(mul(u_projection, u_view), u_model), input.position);

		return output;
	}
}

namespace fragment
{
    
	struct PIn
	{
		float4 position : SV_POSITION;
	};


	float4 main(PIn input) : SV_TARGET
	{
		return float4(1.0, 1.0, 1.0, 1.0);
	}
}