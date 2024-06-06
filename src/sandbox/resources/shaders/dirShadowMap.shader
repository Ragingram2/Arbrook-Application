namespace vertex
{
    #define VERTEX
    #include "camera_utils.shinc"
    #include "light_utils.shinc"

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

        output.position = mul(mul(mul(u_dirLights[0].projection, u_dirLights[0].view), u_model), input.position);
        return output;
    }
}

namespace fragment
{
    #define FRAGMENT
    struct PIn
	{
		float4 position : SV_POSITION;
	};

    static float near = -10.0;
    static float far = 40.0;

	float main(PIn input) : SV_DEPTH 
    {
		float z = (input.position.z * 2.0) - 1.0;
        float linDepth = (2.0 * near * far) / ((far + near) - z * (far - near));

        return input.position.z;
    }
}