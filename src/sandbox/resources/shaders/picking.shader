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

	cbuffer EntityData : register(b1)
	{
		int4 id;
	};


	float4 main(PIn input) : SV_TARGET
	{
		return id/float4(255.0,255.0,255.0,255.0);
		//return float4(id.x,id.y,id.z,1.0);
	}
}