$input a_position
//, i_data0, i_data1, i_data2, i_data3

#include <bgfx_shader.sh>

void main()
{
	//mat4 model = mtxFromCols(i_data0, i_data1, i_data2, i_data3);

	//vec4 worldPos = mul(model, a_position );
	//gl_Position = mul(u_viewProj, worldPos);
	gl_Position = mul(u_modelViewProj, a_position);
}