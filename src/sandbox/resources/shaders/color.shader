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
	cbuffer Color : register(b3)
	{
		float4 color;
	}

	struct PIn
	{
		float4 p_position : SV_POSITION;
	};


	float4 main(PIn input) : SV_TARGET
	{
		return color;
	}
}
