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

		output.p_position = input.position * (u_model * (u_view * u_projection));

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