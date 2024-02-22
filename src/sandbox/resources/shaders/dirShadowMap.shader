namespace vertex
{
    #include "camera_utils.shinc"
    #include "light_utils.shinc"
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

        output.p_position = mul(input.position,mul(u_model ,mul(u_dirLights[0].view ,u_dirLights[0].projection)));

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
		return float4(0,0,0,0);
	}
}